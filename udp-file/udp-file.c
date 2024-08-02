#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <openssl/crypto.h>

#define CHUNK_SIZE          1024

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
};

enum
{
    CMD_SEND_FILE_CHUNK = 2
    , CMD_DEL_FILE_CHUNK
};

static int show_usage(int err, const char* descr)
{
    fprintf(stderr, "\n%s\nUsage:\n", descr);
    fprintf(stderr, "\t%s enc <master-key> <in-file> <out-file>\n", __argv__[0]);
    fprintf(stderr, "\t%s dec <inbox> <file> <master-key>\n", __argv__[0]);
    fprintf(stderr, "\t%s send <outbox> <ip> <port>\n", __argv__[0]);
    fprintf(stderr, "\t%s recv <inbox> <port>\n", __argv__[0]);
    return err;
}


// CHUNK-ENCRYPTED FILE FORMAT
// flag - 1 byte, acknoledge received
// number of attempts to send - 2 bytes, host byte order
// chunk length - 2 bytes, host byte order
// random - 16 bytes iv for AES
// command protocol command meaning "send file chunk" - 1 bytes
// file name zero terminated string - up to 32 bytes
// chunk number - 2 bytes, network byte order
// total chunks - 2 bytes, network byte order
// padding length - 1 byte
// data - up to 1024 bytes
// padding - padding length bytes, up to 15
// hash - 32 bytes

static int enc_main()
{
    if(5 != __argc__)
        return show_usage(ERR_ARGC, "Not enough arguments for \"enc\" subcommand");

    uint8_t mk[2 * AES_BLOCK_SIZE];
    size_t mk_len = 0;
    if(!OPENSSL_hexstr2buf_ex(mk, sizeof(mk), &mk_len, __argv__[2], 0)
        || sizeof(mk) != mk_len)
        return show_usage(ERR_ARGV, "Invalid master key value. Must be 32 bytes length. Hex string.");

    char* file_name = strrchr(__argv__[3], '/');

    file_name = file_name ? file_name + 1 : __argv__[3];

    if(31 < strlen(file_name))
        return show_usage(ERR_ARGV, "File name is too long. Must be 1-31.");

    FILE* in_file = fopen(__argv__[3], "r");
    if(!in_file)
    {
        perror(__argv__[3]);
        return ERR_FILE;
    }
    fseek(in_file, 0, SEEK_END);
    long file_size = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);
    fprintf(stderr, "==>>%ld\n", file_size);

    int ret = ERR_OK;
    do {
        if((1 > file_size) || (0xFFFF * CHUNK_SIZE < file_size))
        {
            fprintf(stderr, "Only files from 1B to 65535KB are supported.\n");
            ret = ERR_FILE;
            break;
        }
        uint16_t last_chunk_size = file_size % CHUNK_SIZE;
        uint16_t chunk_count = file_size / CHUNK_SIZE + !!last_chunk_size;
        fprintf(stderr, "Chunk count: %u, last chunk length: %u\n", chunk_count, last_chunk_size);
    } while(0);
    fclose(in_file);
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
