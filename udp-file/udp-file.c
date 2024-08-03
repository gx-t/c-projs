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
        long chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
        fprintf(stderr, "Chunk count: %lu\n", chunk_count);

        long mk_len = 0;
        mk = OPENSSL_hexstr2buf(__argv__[2], &mk_len);
        if(!mk || 2 * AES_BLOCK_SIZE != mk_len)
        {
            ret = show_usage(ERR_ARGV, "Invalid master key value. Must be 32 bytes length. Hex string.");
            break;
        }
        uint8_t chunk_buff[CHUNK_SIZE];
        uint16_t chunk_num = 0;
        while(running && !feof(stdin))
        {
            size_t chunk_len = fread(chunk_buff, 1, sizeof(chunk_buff), stdin); 
            if(chunk_len != CHUNK_SIZE)
                fprintf(stderr, "===>>> Last chunk length is %lu\n", chunk_len);
            chunk_num ++;
        }
        if(chunk_num != chunk_count)
        {
            fprintf(stderr
                    , "Algo error! Calculated number of chunks = %lu, prceeded = %u\n"
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
