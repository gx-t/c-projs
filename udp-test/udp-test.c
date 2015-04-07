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
  CMD_FIRST = 'a'
  , CMD_PING = CMD_FIRST
  , CMD_MESSAGE
  , CMD_LAST = CMD_MESSAGE
  
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
  buff[bytes_rcvd] = 0;
  fprintf(stderr, "Client: %s:%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
  return ERR_OK;
}

static struct U_USER* u_find_name(char* name, struct U_USER* usr_arr)
{
  int i = 0;
  struct U_USER* pp = usr_arr;
  for(; i < USER_COUNT; i++, pp++) if(!strcmp(pp->name, name)) return pp;
  return (struct U_USER*)0;
}

static struct U_USER* u_find_addr(struct sockaddr_in* addr, struct U_USER* usr_arr)
{
  int i = 0;
  struct U_USER* pp = usr_arr;
  for(; i < USER_COUNT; i++, pp++) if(!memcmp(&pp->addr, addr, sizeof(*addr))) return pp;
  return (struct U_USER*)0;
}

static struct U_USER* u_find_empty(struct U_USER* usr_arr)
{
  int i = 0;
  struct U_USER* pp = usr_arr;
  for(; i < USER_COUNT; i++, pp++) if(!*pp->name) return pp;
  return (struct U_USER*)0;
}

static void u_ping_reject(struct sockaddr_in* addr, char* name)
{
  fprintf(stderr, "Ping rejected: no space for new client (%s - %s:%d)\n", name, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

static void u_register(struct sockaddr_in* addr, char* name, struct U_USER* usr_arr)
{
  struct U_USER* pp = u_find_name(name, usr_arr);
  if(!pp) pp = u_find_empty(usr_arr);
  if(!pp) return u_ping_reject(addr, name);
  memcpy(&pp->addr, addr, sizeof(*addr));
  strcpy(pp->name, name);
}

static int u_check_name(char* name)
{
  int len = strlen(name);
  if(len < 1 || len > NAME_SIZE - 1)
  {
    fprintf(stderr, "Invalid name length: %d (must be 1 to 7)\n", len);
    return 0;
  }
  return 1;
}

static void u_cmd_ping(int ss, struct sockaddr_in* addr, char* data, struct U_USER* usr_arr)
{
  char* name = data;
  if(u_check_name(name)) return;
  u_register(addr, name, usr_arr);
}

static void u_cmd_message(int ss, struct sockaddr_in* addr, char* data, struct U_USER* usr_arr)
{
  ssize_t len = 0;
  char* pp = data;
  pp++;
  len ++;
  char* dest_name = pp;
  if(u_check_name(dest_name)) return;
  struct U_USER* dest = u_find_name(dest_name, usr_arr);
  if(!dest)
  {
    fprintf(stderr, "The name: %s is not registered.\n", dest_name);
    return;
  }
  if(len != sendto(ss, data, len, 0, (struct sockaddr*)&dest->addr, sizeof(dest->addr))) perror("sendto");
}

static void (*u_cmd_arr[])(int, struct sockaddr_in*, char*, struct U_USER*) =
{
  u_cmd_ping, u_cmd_message
};

static void u_process_data(int ss, struct sockaddr_in* addr, char* data, struct U_USER* usr_arr)
{
  fprintf(stderr, "Connection: %s:%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
  int cmd = *data++;
  if(cmd < CMD_FIRST || cmd > CMD_LAST)
  {
    fprintf(stderr, "Unknown command code: %d\n", *data);
    return;
  }
  cmd -= CMD_FIRST;
  return u_cmd_arr[cmd](ss, addr, data, usr_arr);
}

static int u_server_loop(int ss)
{
  fputs("Entering server loop...\n", stderr);
  char buff[BUFF_SIZE];
  struct sockaddr_in addr = {0};
  struct U_USER usr_arr[USER_COUNT];
  while(u_wait_data(ss, buff, &addr))
  {
    u_process_data(ss, &addr, buff, usr_arr);
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

