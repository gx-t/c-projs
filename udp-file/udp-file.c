#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/sha.h>

#define CHUNK_SIZE              1024

static int __argc__;
static char** __argv__;

static int running = 1;

enum
{
    ERR_OK
        , ERR_ARGC
        , ERR_SUBCMD
        , ERR_ARGV
        , ERR_FILE
        , ERR_ALGO
        , ERR_INVALID_HASH
};

enum
{
    CMD_SEND_FILE_CHUNK = 2
        , CMD_DEL_FILE_CHUNK
};

static int show_usage(int err, const char* descr)
{
    fprintf(stderr, "\n%s\nUsage:\n", descr);
    fprintf(stderr, "\t%s enc <master-key> <file-name> < <in-file> > <out-file>\n", __argv__[0]);
    fprintf(stderr, "\t%s dec <master-key> <inbox> < <in-file>\n", __argv__[0]);
    fprintf(stderr, "\t%s send <outbox> <ip> <port>\n", __argv__[0]);
    fprintf(stderr, "\t%s recv <inbox> <port>\n", __argv__[0]);
    return err;
}

#pragma pack(push, 1)
struct UDP_FILE_CHUNK
{
    uint8_t iv[16];
    uint8_t cmd;
    uint8_t padding[9];
    char file_name[32];
    uint32_t offset;
    uint16_t data_len;
    uint8_t data[CHUNK_SIZE];
    uint8_t hash[32];
};
#pragma pack(pop)

struct FILE_CHUNK
{
    uint16_t chunk_num;
    uint8_t flag;
    uint8_t send_count;
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

static int decrypt_chunk(uint8_t mk[2 * AES_BLOCK_SIZE], struct UDP_FILE_CHUNK* chunk)
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

static void encrypt_chunk(uint8_t mk[2 * AES_BLOCK_SIZE], struct UDP_FILE_CHUNK* chunk)
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

static int enc_main()
{
    if(4 != __argc__)
        return show_usage(ERR_ARGC, "Invalid number of arguments for \"enc\" subcommand");

    const char* file_name = __argv__[3];

    if(31 < strlen(file_name))
        return show_usage(ERR_ARGV, "File name is too long. Must be 1-31.");

    fseek(stdin, 0, SEEK_END);
    long file_size = ftell(stdin);
    fseek(stdin, 0, SEEK_SET);
    fprintf(stderr, "==>>%ld\n", file_size);

    if((1 > file_size) || (0xFFFF * CHUNK_SIZE < file_size))
    {
        fprintf(stderr, "Only redirection of files from 1B to 65535KB is supported.\n");
        return ERR_FILE;
    }

    int ret = ERR_OK;

    long mk_len = 0;
    uint8_t* mk = OPENSSL_hexstr2buf(__argv__[2], &mk_len);
    if(!mk || 2 * AES_BLOCK_SIZE != mk_len)
    {
        OPENSSL_free(mk);
        return show_usage(ERR_ARGV, "Invalid master key value. Must be 32 bytes length. Hex string.");
    }

    uint16_t chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    fprintf(stderr, "Chunk count: %u\n", chunk_count);

    uint16_t chunk_num = 0;
    uint32_t offset = 0;
    while(running)
    {
        struct FILE_CHUNK chunk_buff = {.chunk_num = chunk_num, .flag = 0, .send_count = 0};
        memset(&chunk_buff.udp_chunk, 0x55, sizeof(chunk_buff.udp_chunk));
        chunk_buff.udp_chunk.cmd = CMD_SEND_FILE_CHUNK;
        strcpy(chunk_buff.udp_chunk.file_name, file_name);
        chunk_buff.udp_chunk.offset = htonl(offset);

        ssize_t data_len = read(STDIN_FILENO, chunk_buff.udp_chunk.data, CHUNK_SIZE);
        if(0 == data_len)
            break; //EOF

        offset += data_len;
        if(data_len != CHUNK_SIZE)
            fprintf(stderr, "===>>> Last chunk length is %lu\n", data_len);

        chunk_buff.udp_chunk.data_len = htons((uint16_t)data_len);

        encrypt_chunk(mk, &chunk_buff.udp_chunk);
        write(STDOUT_FILENO, &chunk_buff, sizeof(chunk_buff));

        chunk_num ++;
    }
    if(chunk_num != chunk_count)
    {
        fprintf(stderr
                , "Algo error! Calculated number of chunks = %u, prceeded = %u\n"
                , chunk_count, chunk_num);
        ret = ERR_ALGO;
    }
    fprintf(stderr, "==>> Processed chunks: %u\n", chunk_num);

    OPENSSL_free(mk);
    return ret;
}

static int write_chunk_to_file(struct UDP_FILE_CHUNK* chunk)
{
    int fd = open(chunk->file_name, O_WRONLY | O_CREAT, 0644);
    if(-1 == fd)
    {
        perror(chunk->file_name);
        return ERR_FILE;
    }
    int res = ERR_OK;
    do
    {
        lseek(fd, 0, SEEK_SET);
        if(-1 == lseek(fd, chunk->offset, SEEK_SET))
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

    if(chdir(__argv__[3]))
    {
        perror(__argv__[3]);
        return show_usage(ERR_ARGC, "Cannot enter given \"Inbox\" directory");
    }

    long mk_len = 0;
    uint8_t* mk = OPENSSL_hexstr2buf(__argv__[2], &mk_len);
    if(!mk || 2 * AES_BLOCK_SIZE != mk_len)
    {
        OPENSSL_free(mk);
        return show_usage(ERR_ARGV, "Invalid master key value. Must be 32 bytes length. Hex string.");
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
                    , "File data chunk of invalid size (%ld vs %lu) received, ignoring.\n"
                    , bytes_read
                    , sizeof(chunk_buff));
            continue;
        }

        if(ERR_OK != decrypt_chunk(mk, &chunk_buff.udp_chunk))
        {
            fprintf(stderr, "Invalid hash! ignoring.\n");
            continue;
        }
        if(CMD_SEND_FILE_CHUNK != chunk_buff.udp_chunk.cmd)
        {
            fprintf(stderr, "Not CMD_SEND_FILE_CHUNK command! ignoring.\n");
            continue;
        }
        chunk_buff.udp_chunk.offset = ntohl(chunk_buff.udp_chunk.offset);
        chunk_buff.udp_chunk.data_len = ntohs(chunk_buff.udp_chunk.data_len);

        int res = write_chunk_to_file(&chunk_buff.udp_chunk);
        if(res)
            return res;
    }

    return ERR_OK;
}

static int send_main()
{
    return ERR_OK;
}

static int recv_main()
{
    return ERR_OK;
}

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    signal(SIGINT, ctrl_c);
    running = 0;
}

int main(int argc, char* argv[])
{
    __argc__ = argc;
    __argv__ = argv;

    if(argc < 2)
        return show_usage(ERR_ARGC, "Subcommand is missing");

    signal(SIGINT, ctrl_c);

    if(!strcmp("enc", argv[1]))
        return enc_main();
    if(!strcmp("dec", argv[1]))
        return dec_main();
    if(!strcmp("send", argv[1]))
        return send_main();
    if(!strcmp("recv", argv[1]))
        return recv_main();

    return show_usage(ERR_SUBCMD, "Unknown subcommand");
}
