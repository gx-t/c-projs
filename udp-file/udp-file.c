#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <time.h>

#define CHUNK_SIZE              1024
#define SLEEP_MIN_US            1
#define SLEEP_MAX_US            1000000
#define SLEEP_DEFAULT_US        200
#define SENDTO_BUFF_PAUSE_US    1000
#define MAX_SEND_ROUND          16
#define OUTBOX_SCAN_PAUSE_US    500000

static int __argc__ = 0;
static char** __argv__ = NULL;

static int running = 1;
static int ss = -1;

enum
{
    ERR_OK
        , ERR_ARGC
        , ERR_SUBCMD
        , ERR_ARGV
        , ERR_FILE
        , ERR_MAP
        , ERR_ALGO
        , ERR_INVALID_HASH
        , ERR_NETWORK
        , ERR_FORK
        , ERR_CHILD
        , ERR_GENERAL
};

enum
{
    CMD_SEND_FILE_CHUNK = 2
        , CMD_ACK_FILE_CHUNK
};

enum
{
    FLAG_NONE = 0
        , FLAG_ACK = 1
};

static int show_usage(int err, const char* descr)
{
    fprintf(stderr, "\n%s\nUsage:\n", descr);
    fprintf(stderr, "\t%s enc <master-key> <file-name> <in-file> <out-file>\n", __argv__[0]);
    fprintf(stderr, "\t%s dec <master-key> <inbox> < <in-file>\n", __argv__[0]);
    fprintf(stderr, "\t%s shuffle <file-name>\n", __argv__[0]);
    fprintf(stderr, "\t%s dump < <file-name>\n", __argv__[0]);
    fprintf(stderr, "\t%s send <master key> <ip> <port> <enc-file> <chunk pause us>\n", __argv__[0]);
    fprintf(stderr, "\t%s recv <master-key> <port> <out-dir>\n", __argv__[0]);
    fprintf(stderr, "\t%s enc-send <master-key> <ip> <port> <outbox>\n", __argv__[0]);
    return err;
}

#pragma pack(push, 1)

struct UDP_CHUNK
{
    uint8_t iv[16];
    uint8_t cmd;
    uint8_t dummy[1071];
    uint8_t hash[32];
};

struct FILE_CHUNK_ID
{
    char file_name[32];
    uint32_t chunk_num;
};

struct UDP_FILE_CHUNK
{
    uint8_t iv[16];
    uint8_t cmd;
    uint8_t reserved[9];
    struct FILE_CHUNK_ID id;
    uint16_t data_len;
    uint8_t data[CHUNK_SIZE];
    uint8_t hash[32];
};

struct UDP_FILE_ACK
{
    uint8_t iv[16];
    uint8_t cmd;
    uint8_t reserved[25];
    uint16_t count;
    struct FILE_CHUNK_ID id[29];
    uint8_t hash[32];
};
#pragma pack(pop)

struct FILE_CHUNK
{
    struct FILE_CHUNK_ID id;
    uint32_t chunk_num;
    uint8_t flag;
    uint8_t sent_count;
    struct UDP_FILE_CHUNK udp_chunk;
};

void xor_iv_and_mk(uint8_t mk[2 * AES_BLOCK_SIZE], uint8_t iv[AES_BLOCK_SIZE])
{
    for(int i = 0; i < AES_BLOCK_SIZE; i ++)
    {
        iv[i] ^= mk[i];
        iv[i] ^= mk[i + AES_BLOCK_SIZE];
    }
}

static int decrypt_chunk(uint8_t mk[2 * AES_BLOCK_SIZE], struct UDP_CHUNK* chunk)
{
    uint8_t iv[AES_BLOCK_SIZE];
    memcpy(iv, chunk->iv, sizeof(iv));
    xor_iv_and_mk(mk, iv);

    AES_KEY key;
    AES_set_decrypt_key(mk, 256, &key);

    AES_cbc_encrypt(&chunk->cmd
            , &chunk->cmd
            , sizeof(*chunk) - sizeof(chunk->iv)
            , &key
            , iv
            , AES_DECRYPT);

    uint8_t hash[2 * AES_BLOCK_SIZE];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, chunk, sizeof(*chunk) - sizeof(chunk->hash));
    SHA256_Final(hash, &sha256);

    if(memcmp(hash, chunk->hash, sizeof(hash)))
        return ERR_INVALID_HASH;

    return ERR_OK;
}

static void encrypt_chunk(uint8_t mk[2 * AES_BLOCK_SIZE], struct UDP_CHUNK* chunk)
{
    uint8_t iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);
    memcpy(chunk->iv, iv, sizeof(iv));
    xor_iv_and_mk(mk, chunk->iv);

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, chunk, sizeof(*chunk) - sizeof(chunk->hash));
    SHA256_Final(chunk->hash, &sha256);

    AES_KEY key;
    AES_set_encrypt_key(mk, 256, &key);

    AES_cbc_encrypt(&chunk->cmd
            , &chunk->cmd
            , sizeof(*chunk) - sizeof(chunk->iv)
            , &key
            , iv
            , AES_ENCRYPT);
}

static uint8_t* mk_from_hex(const char* hex_str)
{
    long mk_len = 0;
    uint8_t* mk = OPENSSL_hexstr2buf(__argv__[2], &mk_len);
    if(mk)
    {
        if(2 * AES_BLOCK_SIZE == mk_len)
            return mk;
        OPENSSL_free(mk);
    }
    fprintf(stderr, "Invalid master key. Must be a 64-character hex string (32 bytes).\n");
    return NULL;
}

static off_t get_file_size(int fd)
{
    struct stat st;
    if(-1 == fstat(fd, &st))
    {
        perror("fstat");
        return 0;
    }
    if(S_ISDIR(st.st_mode))
        return -1;

    return st.st_size;
}

static int enc_main()
{
    if(6 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"enc\" subcommand");

    uint8_t* mk = mk_from_hex(__argv__[2]);
    if(!mk)
        return ERR_ARGV;

    const char* file_name = __argv__[3];

    if(31 < strlen(file_name))
        return show_usage(ERR_ARGV, "File name is too long. Must be 1-31.");

    int fd_in = open(__argv__[4], O_RDONLY);
    if(-1 == fd_in)
    {
        perror(__argv__[4]);
        return ERR_FILE;
    }

    off_t in_file_size = get_file_size(fd_in);

    fprintf(stderr, "==>>%lld\n", (long long)in_file_size);

    if(1 > in_file_size || (off_t)0xFFFFFFFF * CHUNK_SIZE < in_file_size)
    {
        close(fd_in);
        fprintf(stderr, "The input must be file, 1 to 4294967295 bytes length.\n");
        return ERR_FILE;
    }

    uint8_t* in_data = (uint8_t*)mmap(NULL
            , in_file_size
            , PROT_READ
            , MAP_SHARED
            , fd_in
            , 0);

    close(fd_in);

    if(in_data == MAP_FAILED)
    {
        perror("mmap");
        return ERR_FILE;
    }

    int ret = ERR_OK;

    uint32_t chunk_count = (in_file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    fprintf(stderr, "Chunk count: %u\n", chunk_count);

    int fd_out = open(__argv__[5], O_RDWR | O_CREAT, 0644);
    if(-1 == fd_out)
    {
        munmap(in_data, in_file_size);
        perror(__argv__[5]);
        return ERR_FILE;
    }

    if(ftruncate(fd_out, (off_t)chunk_count * sizeof(struct FILE_CHUNK)))
    {
        munmap(in_data, in_file_size);
        close(fd_out);
        perror("ftruncate");
        return ERR_FILE;
    }

    struct FILE_CHUNK* out_data = (struct FILE_CHUNK*)mmap(NULL
            , (off_t)chunk_count * sizeof(struct FILE_CHUNK)
            , PROT_WRITE
            , MAP_SHARED
            , fd_out
            , 0);

    close(fd_out);

    if(out_data == MAP_FAILED)
    {
        munmap(in_data, in_file_size);
        perror("mmap");
        return ERR_FILE;
    }

    uint32_t chunk_num = 0;
    uint8_t* pp_in = in_data;
    struct FILE_CHUNK* pp_out = out_data;
    off_t copy_len = in_file_size; 

    while(running && copy_len)
    {
        pp_out->chunk_num = chunk_num;
        pp_out->flag = FLAG_NONE;
        pp_out->sent_count = 0;
        strcpy(pp_out->id.file_name, file_name);
        pp_out->id.chunk_num = chunk_num;

        memset(&pp_out->udp_chunk, 0x00, sizeof(pp_out->udp_chunk));
        pp_out->udp_chunk.cmd = CMD_SEND_FILE_CHUNK;
        strcpy(pp_out->udp_chunk.id.file_name, file_name);
        pp_out->udp_chunk.id.chunk_num = htonl(chunk_num);

        if(copy_len < CHUNK_SIZE)
        {
            pp_out->udp_chunk.data_len = htons((uint16_t)copy_len);
            memcpy(pp_out->udp_chunk.data, pp_in, copy_len);
            pp_in += copy_len;
            fprintf(stderr, "===>>> Last chunk length is %u\n", (uint32_t)copy_len);
            copy_len = 0;
        }
        else
        {
            pp_out->udp_chunk.data_len = htons((uint16_t)CHUNK_SIZE);
            memcpy(pp_out->udp_chunk.data, pp_in, CHUNK_SIZE);
            pp_in += CHUNK_SIZE;
            copy_len -= CHUNK_SIZE;
        }

        encrypt_chunk(mk, (struct UDP_CHUNK*)&pp_out->udp_chunk);

        chunk_num ++;
        pp_out ++;
    }
    if(chunk_num != chunk_count)
    {
        fprintf(stderr
                , "Algo error! Calculated number of chunks = %u, proceeded = %u\n"
                , chunk_count, chunk_num);
        ret = ERR_ALGO;
    }
    fprintf(stderr, "==>> Processed chunks: %u\n", chunk_num);

    munmap(in_data, in_file_size);
    munmap(out_data, chunk_count * sizeof(struct FILE_CHUNK));
    OPENSSL_free(mk);
    return ret;
}

static int write_chunk_to_file(struct UDP_FILE_CHUNK* chunk)
{
    int fd = open(chunk->id.file_name, O_WRONLY | O_CREAT, 0644);
    if(-1 == fd)
    {
        perror(chunk->id.file_name);
        return ERR_FILE;
    }
    int res = ERR_OK;
    do
    {
        lseek(fd, 0, SEEK_SET);
        if(-1 == lseek(fd, CHUNK_SIZE * (off_t)chunk->id.chunk_num, SEEK_SET))
        {
            perror("lseek");
            res = ERR_FILE;
            break;
        }
        if(chunk->data_len != write(fd, chunk->data, chunk->data_len))
        {
            perror("write");
            res = ERR_FILE;
            break;
        }
    } while(0);
    close(fd);
    return res;
}

static int dec_main()
{
    if(4 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"dec\" subcommand");

    uint8_t* mk = mk_from_hex(__argv__[2]);
    if(!mk)
        return ERR_ARGV;

    if(chdir(__argv__[3]))
    {
        perror(__argv__[3]);
        return show_usage(ERR_ARGV, "Cannot enter given \"Inbox\" directory");
    }

    while(running)
    {
        struct FILE_CHUNK chunk_buff;
        ssize_t bytes_read = read(STDIN_FILENO, &chunk_buff, sizeof(chunk_buff));
        if(0 == bytes_read)
            break; //EOF
        if(bytes_read != sizeof(chunk_buff))
        {
            fprintf(stderr
                    , "File data chunk of invalid size (%ld vs %lu) read, ignoring.\n"
                    , bytes_read
                    , sizeof(chunk_buff));
            continue;
        }

        if(ERR_OK != decrypt_chunk(mk, (struct UDP_CHUNK*)&chunk_buff.udp_chunk))
        {
            fprintf(stderr, "Invalid hash! ignoring.\n");
            continue;
        }
        if(CMD_SEND_FILE_CHUNK != chunk_buff.udp_chunk.cmd)
        {
            fprintf(stderr, "Not CMD_SEND_FILE_CHUNK command! ignoring.\n");
            continue;
        }
        chunk_buff.udp_chunk.id.chunk_num = ntohl(chunk_buff.udp_chunk.id.chunk_num);
        chunk_buff.udp_chunk.data_len = ntohs(chunk_buff.udp_chunk.data_len);

        int res = write_chunk_to_file(&chunk_buff.udp_chunk);
        if(res)
            return res;
    }

    return ERR_OK;
}

static struct FILE_CHUNK* open_chunk_file_mapping(const char* file_name, size_t* chunk_count)
{
    int fd = open(file_name, O_RDWR);
    if(-1 == fd)
    {
        perror(file_name);
        return NULL;
    }

    off_t file_size = get_file_size(fd);
    fprintf(stderr, "===>>> %lld\n", (long long)file_size);

    if(!file_size || file_size % sizeof(struct FILE_CHUNK))
    {
        close(fd);
        fprintf(stderr, "File size must be multiple of FILE_CHUNK\n");
        return NULL;
    }
    *chunk_count = file_size / sizeof(struct FILE_CHUNK);
    struct FILE_CHUNK* data = (struct FILE_CHUNK*)mmap(NULL
            , file_size
            , PROT_READ | PROT_WRITE
            , MAP_SHARED
            , fd
            , 0);
    close(fd);

    if(data == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }
    return data;
}

static void close_chunk_file_mapping(struct FILE_CHUNK* data, size_t chunk_count)
{
    if(!data || !chunk_count)
        return;
    munmap(data, chunk_count * sizeof(struct FILE_CHUNK));
}

static int shuffle_main()
{
    if(3 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"shuffle\" subcommand");

    size_t num_blocks = 0;
    struct FILE_CHUNK* data = open_chunk_file_mapping(__argv__[2], &num_blocks);
    if(!data)
        return ERR_FILE;

    size_t cnt = 2 * num_blocks;
    while(running && cnt --)
    {
        size_t i = rand() * rand() % num_blocks;
        size_t j = rand() * rand() % num_blocks;
        struct FILE_CHUNK tmp = data[j];
        data[j] = data[i];
        data[i] = tmp;
    }

    close_chunk_file_mapping(data, num_blocks);
    return ERR_OK;
}

static void dump_file_chunk_statistics(const struct FILE_CHUNK* chunks, uint32_t num_blocks)
{
    fprintf(stderr, "File chunk send state ...\n");
    uint32_t hist[256];
    memset(hist, 0, sizeof(hist));
    while(running && num_blocks --)
    {
//        fprintf(stdout, "%s--%10u--%02d--%01d\n"
//                , chunks->id.file_name
//                , chunks->chunk_num
//                , chunks->sent_count
//                , chunks->flag);
        hist[chunks->sent_count] ++;
        chunks ++;
    }
    fprintf(stderr, "Histogram ...\n");
    for(int i = 0; i < sizeof(hist) / sizeof(hist[0]); i ++)
        if(hist[i])
            fprintf(stderr, "%d--%d\n", i, hist[i]);
}

static int dump_main()
{
    if(2 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"dump\" subcommand");

    off_t file_size = get_file_size(STDIN_FILENO);
    if(!file_size || file_size % sizeof(struct FILE_CHUNK))
        return show_usage(ERR_FILE, "Wrong file size.");

    size_t num_blocks = file_size / sizeof(struct FILE_CHUNK);
    struct FILE_CHUNK* data = (struct FILE_CHUNK*)mmap(NULL
            , file_size
            , PROT_READ
            , MAP_SHARED
            , STDIN_FILENO
            , 0);

    if(data == MAP_FAILED)
    {
        perror("mmap");
        return ERR_FILE;
    }

    dump_file_chunk_statistics(data, num_blocks);

    munmap(data, file_size);
    return ERR_OK;
}

static pid_t fork_ack_recv_loop(int ss
        , uint8_t mk[2 * AES_BLOCK_SIZE]
        , struct FILE_CHUNK* data
        , size_t chunk_count)
{
    pid_t pid = fork();
    if(-1 == pid)
    {
        perror("fork");
        return pid;
    }
    if(pid)
        return pid;

    fprintf(stderr, "===>> Acknowledge receive loop\n");
    while(running)
    {
        struct UDP_FILE_ACK ack = {0};
        ssize_t bytes_read = recvfrom(ss
                , &ack
                , sizeof(ack)
                , 0
                , NULL
                , NULL);

        if(bytes_read != sizeof(ack))
            continue;

        if(ERR_OK != decrypt_chunk(mk, (struct UDP_CHUNK*)&ack))
        {
            fprintf(stderr, "Invalid hash! ignoring.\n");
            continue;
        }

        if(CMD_ACK_FILE_CHUNK != ack.cmd)
        {
            fprintf(stderr, "Not CMD_ACK_FILE_CHUNK command! ignoring.\n");
            continue;
        }

        uint16_t ack_count = htons(ack.count);
        for(uint16_t i = 0; running && i < ack_count; i ++)
        {
            for(size_t j = 0; running && j < chunk_count; j ++)
            {
                if(!strcmp(data[j].id.file_name, ack.id[i].file_name)
                        && data[j].id.chunk_num == ntohl(ack.id[i].chunk_num))
                {
                    data[j].flag |= FLAG_ACK;
                    break;
                }
            }
        }

    }
    return pid;
}

static int file_send_loop(int ss
        , struct sockaddr_in *addr
        , struct FILE_CHUNK* data
        , size_t chunk_count
        , useconds_t sleep_us)
{
    int res = ERR_OK;
    int sent_count = -1;
    while(running && sent_count)
    {
        sent_count = 0;
        for(size_t cnt = 0; running && cnt < chunk_count; cnt ++)
        {
            if(data[cnt].flag & FLAG_ACK)
                continue;

            if(sizeof(data[cnt].udp_chunk) != sendto(ss
                        , &data[cnt].udp_chunk
                        , sizeof(data[cnt].udp_chunk)
                        , 0
                        , (struct sockaddr *)addr
                        , sizeof(*addr)))
            {
                if(errno == ENOBUFS)
                {
                    perror("sendto");
                    usleep(sleep_us);
                    continue;
                }
                perror("sendto");
                res = ERR_NETWORK;
                break;
            }
            data[cnt].sent_count ++;
            sent_count ++;
            usleep(sleep_us);
            fprintf(stderr, "\r==>> sent %d chunks", sent_count);
        }
        fprintf(stderr, "\n");
    }
    return res;
}

static int send_main()
{
    if(7 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"send\" subcommand");

    uint8_t* mk = mk_from_hex(__argv__[2]);
    if(!mk)
        return ERR_ARGV;

    size_t chunk_count = 0;
    struct FILE_CHUNK* data = NULL;
    pid_t ack_pid = -1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = inet_addr(__argv__[3]);
    if(INADDR_NONE == addr.sin_addr.s_addr)
    {
        OPENSSL_free(mk);
        fprintf(stderr, "Invalid IP address: %s\n", __argv__[3]);
        return ERR_NETWORK;
    }
    int port = atoi(__argv__[4]);
    if(1024 > port || 0xFFFF < port)
    {
        OPENSSL_free(mk);
        fprintf(stderr, "Invalid port number: %s. Port number is integer from 1024 to 65565\n", __argv__[4]);
        return ERR_NETWORK;
    }
    addr.sin_port = htons(port);
    ss = socket(PF_INET, SOCK_DGRAM, 0);
    if(0 > ss)
    {
        OPENSSL_free(mk);
        perror("socket");
        return ERR_NETWORK;
    }

    data = open_chunk_file_mapping(__argv__[5], &chunk_count);
    if(!data)
    {
        close(ss);
        OPENSSL_free(mk);
        return ERR_FILE;
    }

    useconds_t sleep_us = atoi(__argv__[6]);
    fprintf(stderr, "Chunk send pause time is: %d microseconds\n", sleep_us);
    if(sleep_us < SLEEP_MIN_US || sleep_us > SLEEP_MAX_US)
    {
        fprintf(stderr
                , "Chunk send pause time is out of range (%d to %d), using default: %d\n"
                , SLEEP_MIN_US
                , SLEEP_MAX_US
                , SLEEP_DEFAULT_US);
        sleep_us = SLEEP_DEFAULT_US;
    }

    ack_pid = fork_ack_recv_loop(ss, mk, data, chunk_count);
    if(-1 == ack_pid)
    {
        close_chunk_file_mapping(data, chunk_count);
        close(ss);
        OPENSSL_free(mk);
        return ERR_FORK;
    }
    if(!ack_pid)
    {
        close_chunk_file_mapping(data, chunk_count);
        close(ss);
        OPENSSL_free(mk);
        return ERR_OK;
    }

    int res = file_send_loop(ss, &addr, data, chunk_count, sleep_us);

    kill(ack_pid, SIGINT);
    while(-1 != wait(0));
    close_chunk_file_mapping(data, chunk_count);
    close(ss);
    OPENSSL_free(mk);
    return res;
}

static int push_send_ack(int ss
        , const struct sockaddr_in *client_addr
        , uint8_t mk[32]
        , struct UDP_FILE_ACK* ack
        , const char* file_name
        , uint32_t chunk_num)
{
    if(file_name)
    {
        ack->id[ack->count].chunk_num = htonl(chunk_num);
        strcpy(ack->id[ack->count].file_name, file_name);
        ack->count ++;
    }
    if((!file_name && ack->count)
            || (sizeof(ack->id) / sizeof(ack->id[0]) == ack->count))
    {
        fprintf(stderr, "===>>> ACK -- %d\n", ack->count);
        ack->cmd = CMD_ACK_FILE_CHUNK;
        ack->count = htons(ack->count);
        encrypt_chunk(mk, (struct UDP_CHUNK*)ack);
        if(sizeof(*ack) != sendto(ss
                    , ack
                    , sizeof(*ack)
                    , 0
                    , (struct sockaddr *)client_addr
                    , sizeof(*client_addr)))
        {
            perror("sendto");
            return ERR_NETWORK;
        }
        ack->count = 0;
    }
    return ERR_OK;
}

static int recv_main()
{
    if(5 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"recv\" subcommand");

    uint8_t* mk = mk_from_hex(__argv__[2]);
    if(!mk)
        return ERR_ARGV;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = atoi(__argv__[3]);
    if(1024 > port || 0xFFFF < port)
    {
        OPENSSL_free(mk);
        fprintf(stderr, "Invalid port number: %s. Port number is integer from 1024 to 65565\n", __argv__[3]);
        return ERR_ARGV;
    }
    if(chdir(__argv__[4]))
    {
        OPENSSL_free(mk);
        perror(__argv__[4]);
        return ERR_ARGV;
    }
    addr.sin_port = htons(port);
    ss = socket(PF_INET, SOCK_DGRAM, 0);
    if(0 > ss)
    {
        OPENSSL_free(mk);
        perror("socket");
        return ERR_NETWORK;
    }

    int res = ERR_OK;

    if(bind(ss, (struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        OPENSSL_free(mk);
        close(ss);
        perror("bind");
        return ERR_NETWORK;
    }

    struct timeval tv = {.tv_sec = 0, .tv_usec = 100000};
    setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct UDP_FILE_ACK ack =
    {
        .cmd = CMD_ACK_FILE_CHUNK, 
        .count = 0,
        .id = {}
    };

    struct sockaddr_in client_addr = {0};
    client_addr.sin_family = AF_INET;
    socklen_t client_addr_len = sizeof(client_addr);

    while(running)
    {
        struct UDP_FILE_CHUNK chunk = {0};
        ssize_t bytes_read = recvfrom(ss
                , &chunk
                , sizeof(chunk)
                , 0
                , (struct sockaddr *)&client_addr
                , &client_addr_len);

        if(0 > bytes_read && EWOULDBLOCK == errno)
        {
            if((res = push_send_ack(ss, &client_addr, mk, &ack, NULL, 0)))
                break;

            continue;
        }

        if(bytes_read != sizeof(chunk))
        {
            fprintf(stderr, "%ld bytes UDP datagram received\n", bytes_read);
            continue;
        }

        if(ERR_OK != decrypt_chunk(mk, (struct UDP_CHUNK*)&chunk))
        {
            fprintf(stderr, "Invalid hash! ignoring.\n");
            continue;
        }

        if(CMD_SEND_FILE_CHUNK != chunk.cmd)
        {
            fprintf(stderr, "Not CMD_SEND_FILE_CHUNK command! ignoring.\n");
            continue;
        }

        chunk.id.chunk_num = ntohl(chunk.id.chunk_num);
        chunk.data_len = ntohs(chunk.data_len);

        res = write_chunk_to_file(&chunk);
        if(res)
            return res;

        if((res = push_send_ack(ss, &client_addr, mk, &ack, chunk.id.file_name, chunk.id.chunk_num)))
            break;

        continue;
    }
    OPENSSL_free(mk);
    close(ss);
    return res;
}

static int send_chunk(int ss
        , const struct sockaddr_in* addr
        , struct FILE_CHUNK* chunk
        , uint32_t* sent_chunks
        , uint8_t* mk
        , uint32_t chunk_count
        , struct FILE_CHUNK* chunks
        , int (*process_ack_proc)(uint8_t*, int, uint32_t, struct FILE_CHUNK*))
{
    if(chunk->flag & FLAG_ACK)
        return ERR_OK;

    while(running)
    {
        if(sizeof(chunk->udp_chunk) != sendto(ss
                    , &chunk->udp_chunk
                    , sizeof(chunk->udp_chunk)
                    , 0
                    , (struct sockaddr *)addr
                    , sizeof(*addr)))
        {
            perror("sendto");
            if(errno == ENOBUFS)
            {
                process_ack_proc(mk, ss, chunk_count, chunks);
                continue;
            }
            return ERR_NETWORK;
        }
        break;
    }
    chunk->sent_count ++;
    (*sent_chunks) ++;
    process_ack_proc(mk, ss, chunk_count, chunks);
    return ERR_OK;
}

static int process_ack(uint8_t* mk, int ss, uint32_t chunk_count, struct FILE_CHUNK* chunks)
{
    int ack_count_to_send = 29;
    while(running && ack_count_to_send)
    {
        struct UDP_FILE_ACK ack = {0};
        ssize_t bytes_read = recvfrom(ss
                , &ack
                , sizeof(ack)
                , 0
                , NULL
                , NULL);

        if(0 > bytes_read)
            return ERR_OK; // no more datagrams in queue

        if(bytes_read != sizeof(ack)) //wrong UDP datagram
        {
            fprintf(stderr, "%ld bytes UDP datagram received\n", bytes_read);
            continue;
        }

        if(ERR_OK != decrypt_chunk(mk, (struct UDP_CHUNK*)&ack))
        {
            fprintf(stderr, "Invalid hash! ignoring.\n");
            continue;
        }

        if(CMD_ACK_FILE_CHUNK != ack.cmd)
        {
            fprintf(stderr, "Not CMD_ACK_FILE_CHUNK command! ignoring.\n");
            continue;
        }

        uint16_t ack_count = htons(ack.count);
        for(uint16_t i = 0; running && i < ack_count; i ++)
        {
            uint32_t chunk_num = ntohl(ack.id[i].chunk_num);
            if(chunk_num >= chunk_count
                    || strncmp(ack.id[i].file_name
                        , chunks[chunk_num].id.file_name
                        , sizeof(ack.id[i].file_name))
                    || chunk_num != chunks[chunk_num].id.chunk_num)
            {
                fprintf(stderr, "!!! Chunk not found, must never happen! %s %u %s %u, %u\n"
                        , ack.id[i].file_name
                        , chunk_num
                        , chunks[chunk_num].id.file_name
                        , chunks[chunk_num].id.chunk_num
                        , chunk_count);
                return ERR_GENERAL;
            }
            chunks[chunk_num].flag |= FLAG_ACK;
        }
        ack_count_to_send --;
    }
    return ERR_OK;
}

static int enc_send_file(uint8_t* mk, const struct sockaddr_in* addr)
{
    int res = ERR_OK;
    fprintf(stderr, "==>>> Child started: %d\n", getpid());
    ss = socket(PF_INET, SOCK_DGRAM, 0);
    if(0 > ss)
    {
        OPENSSL_free(mk);
        perror("socket");
        return ERR_NETWORK;
    }

    struct timeval tv = {.tv_sec = 0, .tv_usec = 500}; //Adjust this
    setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char file_name[32];
    while(running)
    {
        int fd_in = -1;
        DIR *dir = opendir(".");
        if(!dir)
        {
            perror("outbox");
            res = ERR_FILE;
            break;
        }
        while(running)
        {
            struct dirent* entry = readdir(dir);
            if(!entry)
                break;
            if('.' == entry->d_name[0] || entry->d_type != DT_REG)
                continue;

            if(31 < strlen(entry->d_name))
            {
                fprintf(stderr, "====>>>> file name: %s is too long. Deleting.\n", entry->d_name);
                unlink(entry->d_name);
                continue;
            }

            char tmp_name[32] = {};
            sprintf(tmp_name, ".data-%d", getpid());
            if(!rename(entry->d_name, tmp_name))
            {
                fprintf(stderr, "====>>>> Found file: %s\n", entry->d_name);
                fd_in = open(tmp_name, O_RDONLY);
                unlink(tmp_name);
                strcpy(file_name, entry->d_name);
                break;
            }
        }
        closedir(dir);

        if(-1 == fd_in)
        {
            sleep(1);
            continue;
        }

        off_t in_file_size = get_file_size(fd_in);

        fprintf(stderr, "===>>> pid=%d, %s, %lld bytes\n"
                , getpid()
                , file_name
                , (long long)in_file_size);

        if(1 > in_file_size || (off_t)0xFFFFFFFF * CHUNK_SIZE < in_file_size)
        {
            close(fd_in);
            fprintf(stderr, "The input must be file with 1 to 4294967295 bytes of data.\n");
            continue;
        }

        uint32_t chunk_count = (in_file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

        fprintf(stderr, "===>>> %s chunk count: %u\n", file_name, chunk_count);

        struct FILE_CHUNK* enc_buff = (struct FILE_CHUNK*)mmap(NULL
                , (off_t)chunk_count * sizeof(struct FILE_CHUNK)
                , PROT_READ | PROT_WRITE
                , MAP_SHARED | MAP_ANONYMOUS
                , -1
                , 0);

        if(enc_buff == MAP_FAILED)
        {
            res = ERR_MAP;
            perror("mmap");
            continue;
        }

        struct FILE_CHUNK* pp = enc_buff;
        uint32_t sent_chunks = 0;
        clock_t t1, t2;
        t1 = clock();

        for(uint32_t chunk_num = 0; running && chunk_num < chunk_count; chunk_num ++, pp ++)
        {
            pp->chunk_num = chunk_num;
            pp->flag = FLAG_NONE;
            pp->sent_count = 0;
            strcpy(pp->id.file_name, file_name);
            pp->id.chunk_num = chunk_num;

            memset(&pp->udp_chunk, 0x00, sizeof(pp->udp_chunk));
            pp->udp_chunk.cmd = CMD_SEND_FILE_CHUNK;
            strcpy(pp->udp_chunk.id.file_name, file_name);
            pp->udp_chunk.id.chunk_num = htonl(chunk_num);

            ssize_t data_len = read(fd_in, pp->udp_chunk.data, CHUNK_SIZE);
            if(0 == data_len)
            {
                fprintf(stderr, "===>>> Error, bytes read must never be zero!\n");
                break;
            }

            if(chunk_num == (chunk_count - 1))
                fprintf(stderr, "\n===>>> Last chunk length is %lu\n", data_len);

            pp->udp_chunk.data_len = htons((uint16_t)data_len);

            encrypt_chunk(mk, (struct UDP_CHUNK*)&pp->udp_chunk);
            if((res = send_chunk(ss, addr, pp, &sent_chunks, mk, chunk_count, enc_buff, process_ack)))
                break;
            fprintf(stderr, "\r====>>>> %s encrypted and sent: %u", file_name, chunk_num);
        }
        close(fd_in);

        fprintf(stderr, "\n====>>>> %s encryption finished, %u chunks sent\n", file_name, sent_chunks);
        fprintf(stderr, "====>>>> Starting send rounds for %s\n", file_name);

        int send_round = 0;

        while(running && sent_chunks && (MAX_SEND_ROUND > ++ send_round))
        {
            sent_chunks = 0;
            pp = enc_buff;
            for(uint32_t chunk_num = 0; running && chunk_num < chunk_count; chunk_num ++, pp ++)
            {
                if((res = send_chunk(ss, addr, pp, &sent_chunks, mk, chunk_count, enc_buff, process_ack)))
                    break;
            }
        }
        t2 = clock();
        fprintf(stderr, "====>>>> File send finished: %s, %u/%u, %u/%u. %f sec\n"
                , file_name
                , sent_chunks
                , chunk_count
                , send_round
                , MAX_SEND_ROUND
                , (double)(t2 - t1) / CLOCKS_PER_SEC);
        dump_file_chunk_statistics(enc_buff, chunk_count);
        munmap(enc_buff, chunk_count * sizeof(struct FILE_CHUNK));
    }
    close(ss);
    OPENSSL_free(mk);
    return res;
}

static int enc_send_main()
{
    if(6 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"enc-send\" subcommand");

    if(chdir(__argv__[5]))
    {
        perror(__argv__[5]);
        return ERR_ARGV;
    }

    uint8_t* mk = mk_from_hex(__argv__[2]);
    if(!mk)
        return ERR_ARGV;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = inet_addr(__argv__[3]);
    if(INADDR_NONE == addr.sin_addr.s_addr)
    {
        OPENSSL_free(mk);
        fprintf(stderr, "Invalid IP address: %s\n", __argv__[3]);
        return ERR_NETWORK;
    }
    int port = atoi(__argv__[4]);
    if(1024 > port || 0xFFFF < port)
    {
        OPENSSL_free(mk);
        fprintf(stderr, "Invalid port number: %s. Port number is integer from 1024 to 65565\n", __argv__[4]);
        return ERR_NETWORK;
    }
    addr.sin_port = htons(port);

    pid_t pid = -1;
    int res = ERR_OK;
    const long cpu_core_count = sysconf(_SC_NPROCESSORS_ONLN);
    for(int i = 0; i < cpu_core_count; i ++)
    {
        if(!(pid = fork()))
            return enc_send_file(mk, &addr);

        if(-1 == pid)
        {
            kill(-getpgrp(), SIGINT);
            res = ERR_FORK;
            break;
        }
    }

    int status = 0;
    while(-1 != (pid = wait(&status)))
    {
        fprintf(stderr, "===>>> Child stopped, pid = %d, status: %d\n", pid, status);
        if(status)
            res = ERR_CHILD;
    }
    OPENSSL_free(mk);
    return res;
}

static void ctrl_c(int sig)
{
    fprintf(stderr, "SIGINT (%d)\n", getpid());
    signal(SIGINT, ctrl_c);
    close(ss);
    fclose(stdin);
    running = 0;
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "==>> %lu, %lu, %lu, %lu\n"
            , sizeof(struct FILE_CHUNK)
            , sizeof(struct UDP_CHUNK)
            , sizeof(struct UDP_FILE_CHUNK)
            , sizeof(struct UDP_FILE_ACK));

    setpgid(0, 0);
    __argc__ = argc;
    __argv__ = argv;

    if(argc < 2)
        return show_usage(ERR_ARGC, "Subcommand is missing");

    signal(SIGINT, ctrl_c);
    srand(time(NULL));

    if(!strcmp("enc", argv[1]))
        return enc_main();
    if(!strcmp("dec", argv[1]))
        return dec_main();
    if(!strcmp("shuffle", argv[1]))
        return shuffle_main();
    if(!strcmp("dump", argv[1]))
        return dump_main();
    if(!strcmp("send", argv[1]))
        return send_main();
    if(!strcmp("recv", argv[1]))
        return recv_main();
    if(!strcmp("enc-send", argv[1]))
        return enc_send_main();

    return show_usage(ERR_SUBCMD, "Unknown subcommand");
}
