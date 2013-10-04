#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


//3g-zte-tools cdon
//3g-zte-tools cdoff
//3g-zte-tools stay online
//3g-zte-tools prefer 3g
//3g-zte-tools prefer 2g
//3g-zte-tools only 3g
//3g-zte-tools only 2g
//3g-zte-tools pin xxx
//3g-zte-tools apn internet.orange))
//3g-zte-tools /dev/ttyUSB2 connect
//3g-zte-tools /dev/ttyUSB2 signal
//3g-zte-tools /dev/ttyUSB1 sms 095643208 "test message to send"
//3g-zte-tools /dev/ttyUSB2 ATZ OK ATDT*99# CONNECT -exec /usr/bin/pppd /dev/ttyUSB2 defaultroute nodetach noauth
//3g-zte-tools stay online only 3g connect

// SMS send/receive:
// http://simoncpu.wikia.com/wiki/Experiments_on_GSM/GPRS/HSDPA_modem
// Convert serial to SLIP:
// http://www.ibiblio.org/pub/Linux/docs/Raven/EyeView/SSR03/SSR03-06.htm
// DONE :( : Try: slattach /dev/ttyUSB2; ls -l /dev/ppp; ifup ppp0
// N_PPP, N_GSM, ... :
// http://lxr.free-electrons.com/source/include/uapi/linux/tty.h#L33
// !!!
// https://www.kernel.org/doc/Documentation/serial/n_gsm.txt

enum
{
  ERR_OK = 0
  , ERR_ARGC
  , ERR_ARGV
  , ERR_OPEN
  , ERR_READ
  , ERR_CHAT
  , ERR_TIMEOUT
  , ERR_HOME
  , ERR_PORT
  , ERR_FLAG
  , ERR_SOCKET
  , ERR_BIND
  , ERR_FORK
  , ERR_UNSUPPORTED_REQUEST
  , ERR_EMPTY_REQUEST
  , ERR_INVALID_REQUEST
  , ERR_NOT_FOUND
  , ERR_SEND
};

#define RESP_BUFF_SIZE 256
#define Z_DEF_TRYCOUNT 3
#define Z_DEF_TIMEOUT 1
#define Z_LISTOPER_TIMEOUT 30

static const char* g_cmd_port = "/dev/ttyUSB1";
static char* const g_pppd[] = {"/usr/sbin/pppd", "/dev/ttyUSB2", "defaultroute", "nodetach", "usepeerdns", 0};

static int z_show_usage(int code)
{
  const char* msg = "Usage: 3g-zte-tools <subcommand>\n"
  "Subcommand can be one of:\n"
  "  help               - display this text\n"
  "  connect            - call *99# and run pppd\n"
  "  signal             - get signal levels: rssi, ecio, rscp\n"
  "  active-operator    - show active operator for installed SIM\n"
  "  list-operators     - list all available operators (timeout 30s)\n"
  "  autorun-status     - show internal flash drive autorun status\n"
  "  autorun-on         - switch internal flash autorun on\n"
  "  autorun-off        - switch internal flash autorun off\n"
  "  stay-online        - stay online\n"
  "  operational-mode   - show operational mode (UMTS, HSPA etc.)\n"
  "  network-status     - same as operational-mode and signal\n"
  "  modem-info         - modem technical information\n"
  "  http-server        - run as http server\n";
  fputs(msg, stderr);
  return code;
}

static char* z_cut_line(char* ss)
{
  char* p = ss;
  while(*p && *p != '\r') p ++;
  *p = 0;
  return ss;
}

static void z_report_error(const char* err) {printf("@error {%s}\n", err);}

static FILE* z_ff = 0;

static void z_timeout(int sig)
{
  if(z_ff)
  {
    z_report_error("timeout");
    fclose(z_ff);
    z_ff = 0;
  }
}

static int z_use_dev(const char* dev_path, int (*fn)(char**), char** argv)
{
  int res = ERR_OK;
  int tries = Z_DEF_TRYCOUNT;
  alarm(Z_DEF_TIMEOUT);
  do
  {
    z_ff = fopen(dev_path, "r+");
    if(!z_ff)
    {
      printf("@error {Cannot open modem device: %s}\n", dev_path);
      return ERR_OPEN;
    }
    res = fn(argv);
    if(z_ff)
    {
      fclose(z_ff);
      z_ff = 0;
    }
  }while(--tries && res != ERR_OK && res != ERR_READ && res != ERR_TIMEOUT);
  return res;
}

static int z_chat(const char* cmd, char* buff, char** result_ptr, const char* resp, ...)
{
  fputs(cmd, z_ff);
  while(fgets(buff, RESP_BUFF_SIZE, z_ff))
  {
    if(*buff == '\r') continue;
    z_cut_line(buff);
//    printf("@modem {%s}\n", z_cut_line(buff));
    *result_ptr = buff;
    while(**result_ptr == *resp && *resp) (*result_ptr)++, resp++;
    return (*resp) ? z_report_error(buff), ERR_CHAT : ERR_OK;
  }
  if(z_ff)
  {
    z_report_error("I/O error, modem removed?");
    return ERR_READ;
  }
  return ERR_TIMEOUT;
}

static int z_simple_chat(const char* func, const char* cmd, const char* resp)
{
  char buff[RESP_BUFF_SIZE];
  char* result_ptr = 0;
  int cc = z_chat(cmd, buff, &result_ptr, resp, NULL);
  if(cc) return cc;
  printf("@%s {%s}\n", func, result_ptr);
  return ERR_OK;
}

static int z_connect()
{
  char buff[RESP_BUFF_SIZE];
  char* result_ptr = 0;
  int cc = z_chat("ATDT*99#\r",buff, &result_ptr, "CONNECT ", NULL);
  if(cc) return cc;
  printf("@connect {speed %s}\n", result_ptr);
  dup2(fileno(z_ff), 0);
  fclose(z_ff);
  z_ff = 0;
  alarm(0);
  execv(*g_pppd, g_pppd);
  printf("@error {Cannot execute %s}\n", *g_pppd);
  return 5;
}

static int z_signal()
{
  char buff[RESP_BUFF_SIZE];
  char* result_ptr = 0;
  int rssi = 0, ecio = 1000, rscp = 1000;
  int cc = z_chat("AT+ZRSSI\r", buff, &result_ptr, "+ZRSSI: ", NULL);
  if(cc) return cc;
  sscanf(result_ptr, "%d,%d,%d", &rssi, &ecio, &rscp);
  printf("@signal {rssi {-%d dbm} ecio {-%d.%d db} rscp {-%d.%d dbm}}\n", rssi, ecio >> 1, (ecio & 1) * 5, rscp >> 1, (rscp & 1) * 5);
  return ERR_OK;
}

static int z_active_operator()
{
  return z_simple_chat("active-operator", "AT+COPS?\r", "+COPS:");
}

static int z_list_operators()
{
  alarm(0);
  alarm(Z_LISTOPER_TIMEOUT);
  return z_simple_chat("list-operators", "AT+COPS=?\r", "+COPS:");
}

static int z_autorun_status()
{
  return z_simple_chat("autorun-status", "AT+ZCDRUN=4\r", "Inquiry autorun open state result(0:NO 1:YES):");
}

static int z_autorun_on()
{
  return z_simple_chat("autorun-on", "AT+ZCDRUN=9\r", "Open autorun state result(0:FAIL 1:SUCCESS):");
}

static int z_autorun_off()
{
  return z_simple_chat("autorun-off", "AT+ZCDRUN=8\r", "Close autorun state result(0:FAIL 1:SUCCESS):");
}

static int z_stay_online()
{
  return z_simple_chat("stay-online", "AT+ZOPRT=5\r", "OK");
}

static int z_operational_mode()
{
  return z_simple_chat("operational-mode", "AT+ZPAS?\r", "+ZPAS: ");
}

static int z_network_status(char** argv)
{
  int res = z_use_dev(g_cmd_port, z_operational_mode, argv);
  if(res) return res;
  return z_use_dev(g_cmd_port, z_signal, argv);
}

static int z_modem_info()
{
  char buff[RESP_BUFF_SIZE];
  fputs("ATI\r", z_ff);
  const char* resp = "Manufacturer: \0Model: \0Revision: \0IMEI: \0+GCAP: \0";
  while(fgets(buff, RESP_BUFF_SIZE, z_ff))
  {
    if(*buff == '\r') continue;
    z_cut_line(buff);
//    printf("@modem {%s}\n", z_cut_line(buff));
    char* pp = buff;
    while(*pp == *resp && *resp) pp++, resp++;
    if(*resp)
    {
      z_report_error(buff);
      return ERR_CHAT;
    }
    printf("@modem-info {%s}\n", buff);
    if(!*++resp) return ERR_OK;
  }
  if(z_ff)
  {
    z_report_error("I/O error, modem removed?");
    return ERR_READ;
  }
  return ERR_TIMEOUT;
  return z_simple_chat("modem-info", "ATI\r", "Model: ");
}

////////////////////////////////////////////////////////////////////////////////
//http-server
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int z_http_server_usage(int code)
{
  const char* msg = "Usage: 3g-zte-tools http-server <home> <port> [-local]\n";
  fputs(msg, stderr);
  return code;
}

static int z_http_init(char** argv)
{
  int local = 0;
  int port = 0;
  int on = 1;
  struct sockaddr_in sin;
  if(!*argv)
  {
    fputs("Missing port number\n", stderr);
    return z_http_server_usage(ERR_ARGC);
  }
  port = atoi(*argv);
  if(port < 1 || port > (1 << 16))
  {
    fputs("Invalid port value. Valid range is [1 65536]\n", stderr);
    return z_http_server_usage(ERR_PORT);
  }
  if(*++argv)
  {
    if(strcmp(*argv, "-local"))
    {
      fprintf(stderr, "Unknown flag: %s\n", *argv);
      return z_http_server_usage(ERR_FLAG);
    }
    local = 1;
  }
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s < 0)
  {
    perror("server socket");
    return ERR_SOCKET;
  }
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = local ? inet_addr("127.0.0.1") : INADDR_ANY;
  sin.sin_port = htons(port);
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  if(bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0 || listen(s, 5) < 0)
  {
    close(s);
    perror("bind/listen");
    return ERR_BIND;
  }
  dup2(s, 0);
  close(s);
  return ERR_OK;
}


//GET / HTTP/1.1
//Host: localhost:8888
//Connection: keep-alive
//Cache-Control: max-age=0
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.71 Safari/537.36
//Accept-Encoding: gzip,deflate,sdch
//Accept-Language: en-US,en;q=0.8
//Cookie: GUID=HUjC8QwZMhzGhnPZtYuX
//
//


//==========MOTION JPEG==================
//http://149.43.156.105/mjpg/video.mjpg


//HTTP/1.0 200 OK
//Cache-Control: no-cache
//Pragma: no-cache
//Expires: Thu, 01 Dec 1994 16:00:00 GMT
//Connection: close
//Content-Type: multipart/x-mixed-replace; boundary=--myboundary


static int z_http_ok()
{
  const char ok[] = "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n";
  if(write(1, ok, sizeof(ok) - 1) != sizeof(ok) - 1)
  {
    perror("write");
    return ERR_SEND;
  }
  return ERR_OK;
}

static int z_http_not_found()
{
  const char err[] = "HTTP/1.0 404 Not found\r\nConnection: close\r\n\r\n<html><body>404 - The requested object was not found</body></html>\n";
  if(write(1, err, sizeof(err) - 1) != sizeof(err) - 1)
  {
    perror("write");
    return ERR_SEND;
  }
  return ERR_NOT_FOUND;
}

static char* z_str_cgi_token(char* arg)
{
  char cc;
  while(*arg && *arg != '&' && !isspace(*arg)) arg ++;
  cc = *arg;
  *arg = 0;
  if(cc == '&') arg ++;
  return arg;
}

int main(int, char**);

static int z_cgi_modem(char* arg)
{
  z_str_cgi_token(arg);
  if(!strcmp("network-status", arg)
    || !strcmp("active-operator", arg)
    || !strcmp("modem-info", arg))
  {
    char* argv[] = {0, 0, 0};
    argv[1] = arg;
    return main(2, argv);
  }
  return z_http_not_found();
}

static int z_cgi_system_uname()
{
  char* const cmd[] = {"/bin/uname", "-a", 0};
  z_http_ok();
  execv(*cmd, cmd);
  perror("execv");
  return z_http_not_found();
}

static int z_cgi_system(char* arg)
{
  z_str_cgi_token(arg);
  if(!strcmp("uname", arg)) return z_cgi_system_uname();
  return z_http_not_found();
}

static int z_cgi_media_camera()
{
  unsigned char buff[1024*1024];
  int fd, cc;
  fd = open("/dev/video0", O_RDONLY);
  if(fd < 0) return z_http_not_found();
  z_http_ok();
  cc = read(fd, buff, sizeof(buff));
  close(fd);
  if(cc > 0) cc = write(1, buff, cc);
  return ERR_OK;
}

static int z_http_multipart()
{
  const char hdr[] =
  "HTTP/1.0 200 OK\r\n"
  "Connection: close\r\n"
  "Content-Type: multipart/x-mixed-replace; boundary=---------\r\n\r\n";
  if(write(1, hdr, sizeof(hdr) - 1) != sizeof(hdr) - 1)
  {
    perror("write");
    return ERR_SEND;
  }
  return ERR_OK;
}

static int z_cgi_media_video()
{
  unsigned char buff[1024*1024];
  int fd, cc;
  fd = open("/dev/video0", O_RDONLY);
  if(fd < 0) return z_http_not_found();
  z_http_multipart();
  while((cc = read(fd, buff, sizeof(buff))) > 0)
  {
    fprintf(stdout, "---------\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", cc);
    fflush(stdout);
    cc = write(1, buff, cc);
    if(cc <= 0) break;
  }
  close(fd);
  return ERR_OK;
}

static int z_cgi_media(char* arg)
{
  z_str_cgi_token(arg);
  if(!strcmp("camera", arg)) return z_cgi_media_camera();
  if(!strcmp("video", arg)) return z_cgi_media_video();
  return z_http_not_found();
}

static int z_http_cgi(char* arg)
{
  char* p = z_str_cgi_token(arg);
  if(!strcmp("system", arg)) return z_cgi_system(p);
  if(!strcmp("modem", arg)) return z_cgi_modem(p);
  if(!strcmp("media", arg)) return z_cgi_media(p);
  return z_http_not_found();
}

static int z_http_send_file(char* arg)
{
  char buff[4096];
  int fd, cc = 0;
  char* p = arg;
  while(*p && !isspace(*p)) p++;
  *p = 0;
  fd = open(arg, O_RDONLY);
  if(fd == -1) return z_http_not_found();
  z_http_ok();
  while((cc = read(fd, buff, sizeof(buff))) > 0) cc = write(1, buff, cc);
  if(cc < 0) perror("read/write");
  close(fd);
  return cc < 0 ? ERR_SEND : ERR_OK;
}

static void z_http_pass_header()
{
  char buff[4096];
  while(fgets(buff, sizeof(buff), stdin) && *buff != '\r' && *buff != '\n');
}

static int z_http_get(char* arg)
{
  z_http_pass_header();
  if(*arg == '/') arg++;
  if(*arg == '?') return z_http_cgi(++arg);
  return z_http_send_file(arg);
}

static int z_http_head(char* arg)
{
  while(*arg && !isspace(*arg)) arg++;
  *arg = 0;
  z_http_pass_header();
  printf("HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n<html><body>TEST</body></html>\n");
  return ERR_OK;
}

static int z_http_client_loop()
{
  struct sockaddr_in addr = {0};
  socklen_t len = sizeof(addr);
  char buff[4096];
  char* p = buff;
  *p = 0;
  if(!fgets(buff, sizeof(buff), stdin))
  {
    fprintf(stderr, "Empty request from client\n");
    return ERR_EMPTY_REQUEST;
  }
  getpeername(0, (struct sockaddr*)&addr, &len);
  fprintf(stderr, "%s:%d %s", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buff);
  while(*p && !isspace(*p)) p++;
  if(!*p)
  {
    fprintf(stderr, "Invalid request from client\n");
    return ERR_INVALID_REQUEST;
  }
  *p++ = 0;
  if(!strcmp(buff, "GET")) return z_http_get(p);
  if(!strcmp(buff, "HEAD")) return z_http_head(p);
  fprintf(stderr, "Unsupported request from client: %s\n", buff);
  return ERR_UNSUPPORTED_REQUEST;
}

static int z_http_accept_loop()
{
  int s;
  while((s = accept(0, 0, 0)) != -1)
  {
    pid_t pid = fork();
    if(!pid)
    {
      dup2(s, 0);
      dup2(s, 1);
      close(s);
      exit(z_http_client_loop());
    }
    close(s);
    if(pid < 0) perror("fork");
  }
  fputs("Stopping server...\n", stderr);
  return ERR_OK;
}

static void z_http_ctrlc(int sig)
{
  fputs("\nCtrl+C\n", stderr);
  close(0);
  close(1);
}

static int z_http_server(char** argv)
{
  if(!*++argv)
  {
    fputs("Missing home directory\n", stderr);
    return z_http_server_usage(ERR_ARGC);
  }
  if(chdir(*argv))
  {
    perror(*argv);
    return z_http_server_usage(ERR_HOME);
  }
  argv++;
  int res = z_http_init(argv);
  if(res) return res;
  signal(SIGCHLD, SIG_IGN);
  signal(SIGINT, z_http_ctrlc);
  return z_http_accept_loop();
}

int main(int argc, char* argv[])
{
  if(argc < 2) return z_show_usage(ERR_ARGC);
  signal(SIGALRM, z_timeout);
  argv++;
  argc--;
  if(!strcmp(*argv, "help")) return z_show_usage(ERR_OK);
  if(!strcmp(*argv, "connect")) return z_use_dev(g_pppd[1], z_connect, argv);
  if(!strcmp(*argv, "signal")) return z_use_dev(g_cmd_port, z_signal, argv);
  if(!strcmp(*argv, "active-operator")) return z_use_dev(g_cmd_port, z_active_operator, argv);
  if(!strcmp(*argv, "list-operators")) return z_use_dev(g_cmd_port, z_list_operators, argv);
  if(!strcmp(*argv, "autorun-status")) return z_use_dev(g_cmd_port, z_autorun_status, argv);
  if(!strcmp(*argv, "autorun-on")) return z_use_dev(g_cmd_port, z_autorun_on, argv);
  if(!strcmp(*argv, "autorun-off")) return z_use_dev(g_cmd_port, z_autorun_off, argv);
  if(!strcmp(*argv, "stay-online")) return z_use_dev(g_cmd_port, z_stay_online, argv);
  if(!strcmp(*argv, "operational-mode")) return z_use_dev(g_cmd_port, z_operational_mode, argv);
  if(!strcmp(*argv, "network-status")) return z_network_status(argv);
  if(!strcmp(*argv, "modem-info")) return z_use_dev(g_cmd_port, z_modem_info, argv);
  if(!strcmp(*argv, "http-server")) return z_http_server(argv);
  fprintf(stderr, "Unknown subcommand: %s\n", *argv);
  return z_show_usage(ERR_ARGV);
}


