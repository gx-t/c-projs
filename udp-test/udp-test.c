#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

enum
{
  ERR_OK
  , ERR_ARGC
  , ERR_ARGV
  , ERR_PORT
  , ERR_SOCKET
  , ERR_RECVFROM
};

enum
{
  CMD_FIRST = '1'
  , CMD_PING = CMD_FIRST
  , CMD_LAST = CMD_PING
  
};

#define MIN_PORT 1
#define MAX_PORT 65535
#define BUFF_SIZE 4096
#define NAME_SIZE 8
#define USER_COUNT 256

struct U_USER
{
  struct sockaddr_in addr;
  char name[NAME_SIZE];
};

static int g_s = -1;

static void ctrl_c()
{
  if(g_s != -1) close(g_s);
  g_s = -1;
  fprintf(stderr ,"⇘⇘⇘ STOPPING...\n");
}

static int u_client_show_usage(char* descr, int ret_code)
{
  fprintf(stderr, "%s\nUsage:\n\tudp-test client address port data\n", descr);
  return ret_code;
}

static int u_client_send_data(char* addr_s, int port, char* data)
{
  if(!*addr_s) return u_client_show_usage("☹ Empty destination address.", 14);
  if(port < 1 || port > ((1 << 16) - 1)) return u_client_show_usage("Port number is out of range.", 15);
  if(!*data) return u_client_show_usage("☹ Empty data to be sent.", 16);
  struct sockaddr_in addr = {0};
  addr.sin_addr.s_addr = inet_addr(addr_s);
  if(addr.sin_addr.s_addr == INADDR_NONE) return u_client_show_usage("☹ Cannot find destination address.", 17);
  if((g_s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    perror("☹☹☹ Cannot create client socket.");
    return 18;
  }
  int data_len = strlen(data);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if(sendto(g_s, data, data_len, 0, (struct sockaddr *)&addr, sizeof(addr)) != data_len)
  {
    if(g_s != -1)
    {
      perror("☹☹☹ Error during send");
      close(g_s);
      return 19;
    }
  }
  close(g_s);
  return 0;
}

static int u_client(int argc, char* argv[])
{
  if(argc != 4) return u_client_show_usage("☹ Invalid number of arguments for client mode.", 13);
  fprintf(stderr, "✔ Starting in client mode...\n");
  return u_client_send_data(argv[1], atoi(argv[2]), argv[3]);
}

static int u_server_usage(const char* txt, int code)
{
  if(txt) fputs(txt, stderr);
  const char* msg = "Usage: udp-test server help,\n"
  "       udp-test server <port>\n"
  "  help               - display this text\n"
  "  port               - listen port 1-65535\n";
  fputs(msg, stderr);
  return code;
}

static int u_init_listen(int port)
{
  int ss = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(ss < 0) perror("Cannot create server socket");
  return ss;
}

static int u_wait_data(int ss, char* buff, struct sockaddr_in* addr)
{
  socklen_t addr_len = sizeof(*addr);
  int bytes_rcvd = recvfrom(ss, buff, BUFF_SIZE - 1, 0, (struct sockaddr*)&addr, &addr_len);
  if(bytes_rcvd < 0)
  {
    perror("recvfrom");
    return ERR_RECVFROM;
  }
  return ERR_OK;
}

static void u_cmd_ping(struct sockaddr_in* addr, char* data, struct U_USER* usr_arr)
{
  char* name = data;
}

static void (*u_cmd_arr[])(struct sockaddr_in*, char*, struct U_USER*) =
{
  u_cmd_ping
};

static void u_process_data(struct sockaddr_in* addr, char* data, struct U_USER* usr_arr)
{
  fprintf(stderr, "Connection: %s:%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
  int cmd = *data++;
  if(cmd < CMD_FIRST || cmd > CMD_LAST)
  {
    fprintf(stderr, "Unknown command code: %d\n", *data);
    return;
  }
  cmd -= CMD_FIRST;
  return u_cmd_arr[cmd](addr, data, usr_arr);
}

static int u_server_loop(int ss)
{
  fputs("Entering server loop...\n", stderr);
  char buff[BUFF_SIZE];
  struct sockaddr_in addr = {0};
  struct U_USER usr_arr[USER_COUNT];
  while(u_wait_data(ss, buff, &addr))
  {
    u_process_data(&addr, buff, usr_arr);
    fprintf(stderr, ">>FROM:%s:%d\n>>DATA: %s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buff);
  }
  close(ss);
}

static int u_server(int argc, char** argv)
{
  if(argc != 2) return u_server_usage("Not enough arguments for server mode.\n", ERR_ARGC);
  argc--;
  argv++;
  if(!strcmp(*argv, "help")) return u_server_usage(0, ERR_OK);
  int port = atoi(*argv);
  if(port < MIN_PORT || port > MAX_PORT) return u_server_usage("Invalid port number.\n", ERR_PORT);
  int ss = u_init_listen(port);
  if(ss < 0) return ERR_SOCKET;
  return u_server_loop(ss);
}

static int u_usage(const char* txt, int code)
{
  if(txt) fputs(txt, stderr);
  const char* msg = "Usage: udp-test <mode> ...\n"
  "Mode can be one of:\n"
  "  help               - display this text\n"
  "  client             - run as client\n"
  "  server             - run as server\n";
  fputs(msg, stderr);
  return code;
}

int main(int argc, char* argv[])
{
  if(argc < 2) return u_usage("Not enough arguments.\n", ERR_ARGC);
  argc --;
  argv ++;
  if(!strcmp(*argv, "client")) return u_client(argc, argv);
  if(!strcmp(*argv, "server")) return u_server(argc, argv);
  return u_usage("Unknown mode.\n", ERR_ARGV);
}

