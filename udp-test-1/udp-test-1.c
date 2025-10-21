#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>

#define MIN_PORT    1024
#define MAX_PORT    65535

enum
{
    ERR_OK
    , ERR_ARGC
    , ERR_DNS
};

static int str_to_sockaddr_in(struct sockaddr_in* srv_addr, const char* str_ip, const char* str_port)
{
    struct hostent* he = gethostbyname(str_ip);
    if(!he)
    {
        perror("gethostbyname");
        return ERR_DNS;
    }
    long l_port = atol(str_port);
    if(l_port < MIN_PORT)
    {
        fprintf(stderr, "Invalid port number is normalized to minimum value: %d\n", MIN_PORT);
        l_port = MIN_PORT;
    }
    if(l_port > MAX_PORT)
    {
        fprintf(stderr, "Invalid port number is normalized to maximum value: %d\n", MIN_PORT);
        l_port = MAX_PORT;
    }
    srv_addr->sin_family        = AF_INET;
    srv_addr->sin_addr.s_addr   = *(uint32_t*)he->h_addr_list[0];
    srv_addr->sin_port          = htons((short)l_port);
    return ERR_OK;
}

static int srv_main(int argc, char* argv[])
{
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd < 0){ perror("socket"); return 1; }

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(5001) };
    if(bind(udp_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){ perror("bind"); close(udp_fd); return 1; }

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO; pfds[0].events = POLLIN;
    pfds[1].fd = udp_fd;      pfds[1].events = POLLIN;

    while(1){
        int timeout_ms = 5000; // 5s
        int ret = poll(pfds, 2, timeout_ms);
        if(ret < 0){
            if(errno == EINTR) continue;
            perror("poll"); break;
        }
        if(ret == 0){
            printf("timeout\n");
            continue;
        }

        if(pfds[0].revents & POLLIN){
            char buf[256];
            if(fgets(buf, sizeof(buf), stdin) != NULL){
                printf("stdin: %s", buf);
            } else {
                if(feof(stdin)){ printf("stdin EOF\n"); break; }
                if(ferror(stdin)){ perror("fgets"); clearerr(stdin); }
            }
        }

        if(pfds[1].revents & POLLIN){
            char buf[1500];
            struct sockaddr_in src;
            socklen_t srclen = sizeof(src);
            ssize_t n = recvfrom(udp_fd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&src, &srclen);
            if(n < 0){ perror("recvfrom"); }
            else {
                buf[n]=0;
                char srcip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &src.sin_addr, srcip, sizeof(srcip));
                printf("udp from %s:%d: %s\n", srcip, ntohs(src.sin_port), buf);
            }
        }
    }

    close(udp_fd);
    return ERR_OK;
}

static int clnt_main(int argc, char* argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Not enought arguments for %s\n", argv[0]);
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "%s <server ip> <server port> \n", argv[0]);
        return ERR_ARGC;
    }

    const char* str_srv_ip     = argv[1];
    const char* str_srv_port   = argv[2];

    int res = ERR_OK;
    struct sockaddr_in srv_addr = {};

    if(ERR_OK != (res = str_to_sockaddr_in(&srv_addr, str_srv_ip, str_srv_port)))
        return res;

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd < 0){ perror("socket"); return 1; }

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(5001) };
    if(bind(udp_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){ perror("bind"); close(udp_fd); return 1; }

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO; pfds[0].events = POLLIN;
    pfds[1].fd = udp_fd;      pfds[1].events = POLLIN;

    while(1){
        int timeout_ms = 5000; // 5s
        int ret = poll(pfds, 2, timeout_ms);
        if(ret < 0){
            if(errno == EINTR) continue;
            perror("poll"); break;
        }
        if(ret == 0){
            printf("timeout\n");
            continue;
        }

        if(pfds[0].revents & POLLIN){
            char buf[256];
            if(fgets(buf, sizeof(buf), stdin) != NULL){
                printf("stdin: %s", buf);
            } else {
                if(feof(stdin)){ printf("stdin EOF\n"); break; }
                if(ferror(stdin)){ perror("fgets"); clearerr(stdin); }
            }
        }

        if(pfds[1].revents & POLLIN){
            char buf[1500];
            struct sockaddr_in src;
            socklen_t srclen = sizeof(src);
            ssize_t n = recvfrom(udp_fd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&src, &srclen);
            if(n < 0){ perror("recvfrom"); }
            else {
                buf[n]=0;
                char srcip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &src.sin_addr, srcip, sizeof(srcip));
                printf("udp from %s:%d: %s\n", srcip, ntohs(src.sin_port), buf);
            }
        }
    }

    close(udp_fd);
    return ERR_OK;
}

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    signal(SIGINT, ctrl_c);
    close(STDIN_FILENO);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, ctrl_c);
    if(argc < 2)
    {
        fprintf(stderr, "Not enought arguments for %s\n", argv[0]);
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "%s srv|clnt <params>\n", argv[0]);
        return ERR_ARGC;
    }
    if(!strcmp(argv[1], "srv"))
        return srv_main(-- argc, ++ argv);
    if(!strcmp(argv[1], "clnt"))
        return clnt_main(-- argc, ++ argv);
}

