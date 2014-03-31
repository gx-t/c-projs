#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

static int g_s = -1;

static void ctrl_c()
{
  if(g_s != -1) close(g_s);
  g_s = -1;
  fprintf(stderr ,"⇘⇘⇘ STOPPING...\n");
}

static int udp_client_show_usage(char* descr, int ret_code)
{
  fprintf(stderr, "%s\nUsage:\n\tudp-test client address port data\n", descr);
  return ret_code;
}

static int udp_client_send_data(char* addr_s, int port, char* data)
{
  if(!*addr_s) return udp_client_show_usage("☹ Empty destination address.", 14);
  if(port < 1 || port > ((1 << 16) - 1)) return udp_client_show_usage("Port number is out of range.", 15);
  if(!*data) return udp_client_show_usage("☹ Empty data to be sent.", 16);
  struct sockaddr_in addr = {0};
  addr.sin_addr.s_addr = inet_addr(addr_s);
  if(addr.sin_addr.s_addr == INADDR_NONE) return udp_client_show_usage("☹ Cannot find destination address.", 17);
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

static int udp_client(int argc, char* argv[])
{
  if(argc != 4) return udp_client_show_usage("☹ Invalid number of arguments for client mode.", 13);
  fprintf(stderr, "✔ Starting in client mode...\n");
  return udp_client_send_data(argv[1], atoi(argv[2]), argv[3]);
}

static int udp_server_show_usage(char* descr, int ret_code)
{
  fprintf(stderr, "%s\nUsage:\n\tudp-test server port\n", descr);
  return ret_code;
}

static int udp_init_server_socket(int port)
{
  if(port < 1 || port > ((1 << 16) - 1)) return udp_server_show_usage("Port number is out of range.", 4);
  if((g_s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    perror("☹☹☹ Cannot create server socket.");
    return 5;
  }
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if(bind(g_s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("☹☹ Failed to bind server socket");
    close(g_s);
    return 6;
  }
  return 0;
}

static int udp_server_loop()
{
  fprintf(stderr, "↺ Entering server loop...\n");
  while(1)
  {
    char buff[4096];
    struct sockaddr_in addr = {0};
    int addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    int bytes_recvd = 0;
    if((bytes_recvd = recvfrom(g_s, buff, sizeof(buff) - 1, 0, (struct sockaddr*)&addr, &addr_len)) < 0)
    {
      if(g_s != -1)
      {
        perror("☹☹☹ Error during receive");
        close(g_s);
        return 7;
      }
      return 0;
    }
    buff[bytes_recvd] = 0;
    fprintf(stderr, ">>FROM:%s:%d\n>>DATA: %s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buff);
  }
}

static int udp_server(int argc, char* argv[])
{
  int res = 0;
  if(argc != 2) return udp_server_show_usage("☹ Invalid number of arguments for server mode.", 3);
  fprintf(stderr, "✔ Starting in server mode...\n");
  res = udp_init_server_socket(atoi(argv[1]));
  if(res) return res;
  return udp_server_loop();
}

static int udp_show_usage(int ret_code)
{
  fprintf(stderr, "Usage:\n\tudp-test client\n\tudp-test server\n");
  return ret_code;
}

int main(int argc, char* argv[])
{
  if(argc < 2) return udp_show_usage(1);
  argc --;
  argv ++;
  signal(SIGINT, ctrl_c);
  if(!strcmp(argv[0], "client")) return udp_client(argc, argv);
  if(!strcmp(argv[0], "server")) return udp_server(argc, argv);
  fprintf(stderr, "☹ Unknown mode: %s\n", argv[1]);
  return udp_show_usage(2);
}

