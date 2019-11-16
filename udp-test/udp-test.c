#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <errno.h>

enum {
    ERR_OK
    , ERR_ARG
    , ERR_SOCKET
    , ERR_BIND
    , ERR_DNS
    , ERR_SETTIMEOUT
    , ERR_SEND
    , ERR_RECV
};

#define DATA_SIZE        (32 - 1)
#define MAX_DEV_COUNT    0x100
#define DEV_NAME_SIZE    16


struct BASE {
    int ss;
    struct sockaddr_in addr;
    struct {
        uint8_t cmd;
        uint8_t data[DATA_SIZE];
    } buff;

    int argc;
    char** argv;
};

struct SRV {
    struct BASE base;
    struct {
        int timeout_counter;
        struct {
            char name[DEV_NAME_SIZE];
            uint32_t ip;
            uint16_t port;
        }info;
    }cl[MAX_DEV_COUNT];
};

struct DEV {
    struct BASE base;
    char NAME[DEV_NAME_SIZE];
    int time_counter;
};

static struct BASE* gg = 0;

#define LISTEN_PORT         23456
#define TIMEOUT_SEC         1

static void close_socket()
{
    if(-1 != gg->ss) {
        close(gg->ss);
        gg->ss = -1;
    }
}

static void handle_srv_timeout()
{
    struct SRV* srv = (struct SRV*)gg;
    for(int i = 0; i < MAX_DEV_COUNT; i ++)
        srv->cl[i].timeout_counter -= !!srv->cl[i].timeout_counter;
}

static void handle_dev_timeout()
{
    struct DEV* dev = (struct DEV*)gg;
}

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    signal(SIGALRM, SIG_IGN);
    signal(SIGINT, ctrl_c);
    close_socket();
}

static void srv_alarm(int sig)
{
    signal(SIGALRM, srv_alarm);
    handle_srv_timeout();
    alarm(1);
}

static void dev_alarm(int sig)
{
    signal(SIGALRM, dev_alarm);
    handle_dev_timeout();
    alarm(1);
}

static int init_base(struct BASE* base, int argc, char* argv[])
{
    gg = base;

    base->argc = argc;
    base->argv = argv;
    base->ss = socket(AF_INET, SOCK_DGRAM, 0);
    if(base->ss < 0) {
        perror("socket");
        return ERR_SOCKET;
    }
    base->addr.sin_family = AF_INET;

    signal(SIGINT, ctrl_c);

    return ERR_OK;
}

static int init_srv(struct SRV* srv, int argc, char* argv[])
{
    int res = ERR_OK;

    if((res = init_base(&srv->base, argc, argv)))
        return res;

    srv->base.addr.sin_family = AF_INET;
    srv->base.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv->base.addr.sin_port = htons(LISTEN_PORT);

    if(bind(srv->base.ss, (struct sockaddr *)&srv->base.addr, sizeof(srv->base.addr)) < 0)
    {
        perror("bind");
        close(srv->base.ss);
        return ERR_BIND;
    }

    for(int i = 0; i < MAX_DEV_COUNT; i ++)
        srv->cl[i].timeout_counter = 0;

    signal(SIGALRM, srv_alarm);
    alarm(1);

    return ERR_OK;
}

static int init_dev(struct DEV* dev, int argc, char* argv[])
{
    int res = ERR_OK;

    if((res = init_base(&dev->base, argc, argv)))
        return res;

    dev->base.addr.sin_family = AF_INET;

    struct hostent* he = gethostbyname(argv[3]);
    if(!he) {
        perror("gethostbyname");
        close(dev->base.ss);
        return ERR_DNS;
    }

    dev->base.addr.sin_addr.s_addr = *(uint32_t*)he->h_addr_list[0];
    dev->base.addr.sin_port = htons(LISTEN_PORT);

    signal(SIGALRM, dev_alarm);
    alarm(1);

    return ERR_OK;
}

static int set_socket_timeout(time_t timeout)
{
    struct timeval tv = {timeout, 0};
    if(0 > setsockopt(gg->ss, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval))) {
        perror("set_socket_timeout");
        return ERR_SETTIMEOUT;
    }
    return ERR_OK;
}

static void dump_addr()
{
    fprintf(stderr, "%s:%d\n", inet_ntoa(gg->addr.sin_addr), ntohs(gg->addr.sin_port));
}

enum {
    CMD_PING,
    CMD_DEV_LIST,
    CMD_LAST = CMD_DEV_LIST
};

static int find_cl()
{
    struct SRV* srv = (struct SRV*)gg;
    for(int i = 0; i < MAX_DEV_COUNT; i ++) {
        if(gg->addr.sin_addr.s_addr == srv->cl[i].info.ip && gg->addr.sin_port == srv->cl[i].info.port)
            return i;
    }
    return -1;
}

static int find_free_cl_slot()
{
    for(int i = 0; i < MAX_DEV_COUNT; i ++) {
        if(!((struct SRV*)gg)->cl[i].timeout_counter)
            return i;
    }
    return -1;
}

static void save_cl_ping_info(int idx)
{
    struct SRV* srv = (struct SRV*)gg;
    srv->cl[idx].info.ip = gg->addr.sin_addr.s_addr;
    srv->cl[idx].info.port = gg->addr.sin_port;
    memcpy(srv->cl[idx].info.name, gg->buff.data, DEV_NAME_SIZE);
}

static int send_empty()
{
    if(0 == sendto(gg->ss, &gg->buff, 0, 0, (struct sockaddr *)&gg->addr, sizeof(gg->addr)) || gg->ss == -1)
        return ERR_OK;

    perror("sendto");
    return ERR_SEND;
}

static int send_data()
{
    if(sizeof(gg->buff) == sendto(gg->ss, &gg->buff, sizeof(gg->buff), 0, (struct sockaddr *)&gg->addr, sizeof(gg->addr)) || gg->ss == -1)
        return ERR_OK;

    perror("sendto");
    return ERR_SEND;
}

static int cmd_unsupported()
{
    return ERR_OK;
}

static int cmd_ping()
{
    int idx = find_cl();
    if(-1 == idx)
        idx = find_free_cl_slot();

    if(!idx) {
        fprintf(stderr, "No free client slots\n");
        return ERR_OK;
    }
    save_cl_ping_info(idx);
    return send_empty();
}

static void load_dev_data(int idx)
{
    struct SRV* srv = (struct SRV*)gg;
    uint8_t* pp = gg->buff.data;
    *pp ++ = (uint8_t)idx;
    memcpy(pp, &srv->cl[idx].info, sizeof(srv->cl[idx].info));
}

static int is_dev_alive(int idx)
{
    return !!((struct SRV*)gg)->cl[idx].timeout_counter;
}

static int cmd_dev_list_srv()
{
    int res = ERR_OK;

    for(int i = 0; i < MAX_DEV_COUNT && ERR_OK == res; i ++) {
        if(is_dev_alive(i))
            continue;
        load_dev_data(i);
        res = send_data();
    }
    return res;
}

static int recv_loop(int (*cmd_proc[])())
{
    int res = ERR_OK;
    int len = 0;
    socklen_t addr_len;

    while(addr_len = sizeof(gg->addr), 0 <= (len = recvfrom(gg->ss, &gg->buff, sizeof(gg->buff), 0, (struct sockaddr*)&gg->addr, &addr_len))) {
        dump_addr();

        if(sizeof(gg->buff) != len) {
            fprintf(stderr, "Invalid data length: %d\n", len);
            continue;
        }

        if(gg->buff.cmd > CMD_LAST) {
            fprintf(stderr, "unsupported command: %d\n", gg->buff.cmd);
            continue;
        }

        if((res = cmd_proc[gg->buff.cmd]()))
            return res;
    }
    if(gg->ss == -1)
        return ERR_OK;

    perror("recvfrom");
    return ERR_RECV;
}

static int srv_main(int argc, char* argv[])
{
    static int (*cmd_proc[])() = {
        [CMD_PING]      = cmd_ping,
        [CMD_DEV_LIST]  = cmd_dev_list_srv,
    };

    int res = ERR_OK;
    struct SRV srv;

    if((res = init_srv(&srv, argc, argv)))
        return res;

    res = recv_loop(cmd_proc);

    close_socket();
    return res;
}

static int clnt_main(int argc, char* argv[])
{
    static int (*cmd_proc[])() = {
        [CMD_PING]      = cmd_unsupported,
        [CMD_DEV_LIST]  = cmd_unsupported,
    };

    int res = ERR_OK;
    struct SRV srv;

    if((res = init_srv(&srv, argc, argv)))
        return res;

    res = recv_loop(cmd_proc);

    close_socket();
    return res;
}

static int dev_main(int argc, char* argv[])
{
    static int (*cmd_proc[])() = {
        [CMD_PING]      = cmd_unsupported,
        [CMD_DEV_LIST]  = cmd_unsupported,
    };

    int res = ERR_OK;
    struct DEV dev;

    if(argc != 4) {
        fprintf(stderr, "Invalid number of parameters.\nUsage: %s %s <name> <server ip>\n", argv[0], argv[1]);
        return ERR_ARG;
    }

    if((res = init_dev(&dev, argc, argv)))
        return res;

    res = recv_loop(cmd_proc);

    close_socket();
    return res;
}

static char* extract_cmd(char* argv_0)
{
    char* cmd = argv_0;
    while(*argv_0) {
        if(*argv_0++ == '/')
            cmd = argv_0;
    }
    return cmd;
}

int main(int argc, char* argv[])
{

    char* cmd = extract_cmd(argv[0]);

    if(!strcmp("udp-srv", cmd))
        return srv_main(argc, argv);
    if(!strcmp("udp-clnt", cmd))
        return clnt_main(argc, argv);
    if(!strcmp("udp-dev", cmd))
        return dev_main(argc, argv);

    fprintf(stderr, "Unknown command: %s\n", cmd);
    return ERR_ARG;
}

