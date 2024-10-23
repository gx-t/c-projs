#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

static int running = 1;

enum
{
    ERR_OK
        , ERR_ARGC
        , ERR_SUBCMD
};

static void init_camera()
{
    fprintf(stderr, "Init camera...\n");
    sleep(3);
    fprintf(stdout, "1\n1\n1\n");
    fflush(stdout);
    sleep(6);
    fprintf(stderr, "Ready\n");
}

static void set_timer(int cmd)
{
    struct itimerval timer =
    {
        .it_interval.tv_sec = cmd,
        .it_interval.tv_usec = 0,
        .it_value.tv_sec = cmd,
        .it_value.tv_usec = 0
    };

    if(-1 == setitimer(ITIMER_REAL, &timer, NULL))
    {
        perror("setitimer");
        return;
    }
    fprintf(stderr, "==>> Timer: %d\n", cmd);
}

static void shutdown_camera()
{
    set_timer(0);
    fprintf(stderr, "Shutdown camera...\n");
    sleep(6);
    fprintf(stdout, "0\n0\nx\n");
    fflush(stdout);
    sleep(6);
    fprintf(stderr, "Done\n");
}

static void shoot_camera()
{
    fprintf(stdout, "1\n");
    fflush(stdout);
}

static void (*timer_proc)() = shoot_camera;

static void alrm_handler(int sig)
{
    timer_proc();
}

static int ctrl_main(int arc, char* argv[])
{
    signal(SIGALRM, alrm_handler);
    init_camera();
    while(running)
    {
        int period = -1;
        fprintf(stderr, ">>");
        char buff[64] = {0};

        if(!fgets(buff, sizeof(buff), stdin))
            break;

        if(1 == sscanf(buff, "%d", &period))
        {
            set_timer(period);
            continue;
        }
        fprintf(stdout, "%s", buff);
        fflush(stdout);
    }
    shutdown_camera();
    return ERR_OK;
}

static int filter_main(int argc, char* argv[])
{
    enum
    {
        ACK_OFF = 1,
        ACK_ON  = 2
    } mode = ACK_ON;
    const char file_ready[] = "Complete download. File:";
    while(running)
    {
        char buff[256];
        if(!fgets(buff, sizeof(buff), stdin))
            break;
        char* ss = strstr(buff, file_ready);
        if(ss)
        {
            ss += sizeof(file_ready);
            if(ACK_OFF == mode)
                *--ss = '-';
            fprintf(stdout, "%s", ss);
            fflush(stdout);
            continue;
        }
        if('-' == *buff)
        {
            mode = ACK_OFF;
            fprintf(stderr, "ack off\n");
            continue;
        }
        if('+' == *buff)
        {
            mode = ACK_ON;
            fprintf(stderr, "ack on\n");
            continue;
        }
    }
    return ERR_OK;
}

static void ctrl_c(int sig)
{
    signal(SIGINT, ctrl_c);
    running = 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\t%s control\n", *argv);
        fprintf(stderr, "\t%s filter\n", *argv);
        return ERR_ARGC;
    }

    signal(SIGINT, ctrl_c);

    argc --;
    argv ++;
    if(!strcmp(*argv, "control"))
        return ctrl_main(argc, argv);
    if(!strcmp(*argv, "filter"))
        return filter_main(argc, argv);

    fprintf(stderr, "Unknown sub-command: %s\n", *argv);
    return ERR_SUBCMD;
}

