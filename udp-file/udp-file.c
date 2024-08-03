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
#define CHUNK_HEADER_SIZE       48

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
    fprintf(stderr, "\t%s dec <inbox> <file> <master-key>\n", __argv__[0]);
    fprintf(stderr, "\t%s send <outbox> <ip> <port>\n", __argv__[0]);
    fprintf(stderr, "\t%s recv <inbox> <port>\n", __argv__[0]);
    return err;
}

// --- CHUNK-ENCRYPTED FILE FORMAT
// flag - 1 byte, acknoledge received
// number of attempts to send - 1 byte

// --- UDP FILE CHUNK FORMAT (16 byte padding)
// random - 16 bytes iv for AES
// command protocol command meaning "send file chunk" - 1 bytes
// padding - 9 bytes
// file name zero terminated string - 32 bytes
// chunk number - 2 bytes, network byte order
// total chunks - 2 bytes, network byte order
// data length - 2 byte
// data - 1024 bytes (with padding to 1024)
// hash - 32 bytes

static void encrypt_chunk(uint8_t mk[2 * AES_BLOCK_SIZE]
        , const char* file_name
        , uint16_t chunk_num
        , uint16_t chunk_count
        , uint16_t chunk_len
        , uint8_t chunk_buff[CHUNK_HEADER_SIZE + CHUNK_SIZE])
{
    uint8_t iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);

// file chunk header
    putchar(0); //flag
    putchar(0); //number of attempts to send

// UDP data header
    fwrite(iv, sizeof(iv), 1, stdout);

    uint8_t* pp = chunk_buff;

    *pp ++ = CMD_SEND_FILE_CHUNK;
    pp += 9; //padding
    memcpy(pp, file_name, 32);
    pp += 32;
    *((uint16_t*)pp) = htons(chunk_num);
    pp += sizeof(uint16_t);
    *((uint16_t*)pp) = htons(chunk_count);
    pp += sizeof(uint16_t);
    *((uint16_t*)pp) = htons(chunk_len);
    pp += sizeof(uint16_t);

    AES_KEY key;
    AES_set_encrypt_key(mk, 256, &key);
    AES_cbc_encrypt(chunk_buff, chunk_buff, CHUNK_HEADER_SIZE + CHUNK_SIZE, &key, iv, AES_ENCRYPT);
    fwrite(chunk_buff, CHUNK_HEADER_SIZE + CHUNK_SIZE, 1, stdout);

    uint8_t hash[32];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, chunk_buff, CHUNK_HEADER_SIZE + CHUNK_SIZE);
    SHA256_Final(hash, &sha256);
    fwrite(hash, 32, 1, stdout);
}

static int enc_main()
{
    if(4 != __argc__)
        return show_usage(ERR_ARGC, "Not enough arguments for \"enc\" subcommand");

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
    uint8_t* mk = NULL;
    do {
        uint16_t chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
        fprintf(stderr, "Chunk count: %u\n", chunk_count);

        long mk_len = 0;
        mk = OPENSSL_hexstr2buf(__argv__[2], &mk_len);
        if(!mk || 2 * AES_BLOCK_SIZE != mk_len)
        {
            ret = show_usage(ERR_ARGV, "Invalid master key value. Must be 32 bytes length. Hex string.");
            break;
        }
        uint16_t chunk_num = 0;
        while(running && !feof(stdin))
        {
            uint8_t chunk_buff[CHUNK_HEADER_SIZE + CHUNK_SIZE];
            memset(chunk_buff, 0x55, sizeof(chunk_buff));
            size_t chunk_len = fread(chunk_buff + CHUNK_HEADER_SIZE, 1, CHUNK_SIZE, stdin); 
            if(chunk_len != CHUNK_SIZE)
                fprintf(stderr, "===>>> Last chunk length is %lu\n", chunk_len);

            encrypt_chunk(mk, file_name, chunk_num, chunk_count, chunk_len, chunk_buff);
            chunk_num ++;
        }
        if(chunk_num != chunk_count)
        {
            fprintf(stderr
                    , "Algo error! Calculated number of chunks = %u, prceeded = %u\n"
                    , chunk_count, chunk_num);
            ret = ERR_ALGO;
            break;
        }
        fprintf(stderr, "==>> Processed chunks: %u\n", chunk_num);

    } while(0);
    OPENSSL_free(mk);
    return ret;
}

static int dec_main()
{
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
