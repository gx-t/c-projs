#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>



////////////////////////////////////////////////////////////////////////////////
//COM PORT
////////////////////////////////////////////////////////////////////////////////
#define FLG_START 0xF0
#define FLG_STOP 0xF1
#define FLG_CTRLESC 0xF2
#define XOR_BYTE 0xAA

#define S_WAIT_START  0
#define S_READ        1
#define S_ESCAPE      2

union CRC
{
  struct
  {
	  unsigned char low;
	  unsigned char high;
  }byte;
  unsigned short word;
};

unsigned short crc16(const unsigned char* buff, unsigned len)
{
	union CRC crc;
	unsigned j;
	unsigned char i;
	unsigned char b;

  crc.word = 0xFFFF;
	for (j = 0; j < len; j++)
	{
		b = buff[j];
		for (i = 0; i < 8; i++)
		{
			crc.word = ((b ^ crc.byte.high) & 0x80) ? ((crc.word << 1) ^ 0x8005) : (crc.word << 1);
			b <<= 1;
		}
	}
	return crc.word;
}

void com_init_port(int fd)
{
  struct termios options;
  // Get the current options for the port...
  tcgetattr(fd, &options);
  cfsetospeed(&options, B115200);
  // Set the baud rates to 19200...
  cfsetispeed(&options, B115200);
  // Enable the receiver and set local mode...
  options.c_cflag |= (CLOCAL|CREAD);

  options.c_cflag &= ~CSIZE;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~PARENB;
  options.c_cflag |= CS8;     //No parity, 8bits, 1 stop bit (8N1)
  options.c_cflag &= ~CRTSCTS;//CNEW_RTSCTS; //Turn off flow control ???
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //Make sure that Canonical input is off (raw input data)
  options.c_oflag &= ~OPOST; //Raw output data
  options.c_iflag &= ~(IXON | IXOFF | IXANY); //Turn off software control flow

  // Set the new options for the port...
  tcsetattr(fd, TCSANOW, &options);
  fcntl(fd, F_SETFL, FNDELAY);
}

static int com_write_binary(int fd, int from, int to, unsigned char* data, int len)
{
  unsigned char bt, crc[2], *p;
  bt = FLG_START;
  if(1 != write(fd, &bt, 1))
    return -1;
  bt = from;
  if(1 != write(fd, &bt, 1))
    return -1;
  bt = to;
  if(1 != write(fd, &bt, 1))
    return -1;
  p = data;
  while(fd != -1 && len --)
  {
    bt = *p;
    if(bt == FLG_START || bt == FLG_STOP || bt == FLG_CTRLESC)
    {
      bt = FLG_CTRLESC;
      if(1 != write(fd, &bt, 1))
        return -1;
      bt = *p;
      bt ^= XOR_BYTE;
    }
    if(1 != write(fd, &bt, 1))
      return -1;
    p ++;
  }
  *(unsigned short*)crc = crc16(data, len);
  bt = FLG_STOP;
  if(1 != write(fd, &crc[1], 1) || 1 != write(fd, &crc[0], 1) || 1 != write(fd, &bt, 1))
    return -1;
  return 0;
}

int com_read_binary(int fd, unsigned char* buff, int* len)
{
  int count = 0, state = S_WAIT_START;
  unsigned char *p;
  while(fd != -1)
  {
    unsigned char bt;
    if(count == *len)
    {
      fprintf(stderr, "No stop bit until buffer overflow, setting S_WAIT_START\n");
      state = S_WAIT_START;
    }
    if(1 != read(fd, &bt, 1))
      return -1;
    if(bt == FLG_START)
    {
      state = S_READ;
      p = buff;
      count = 0;
      continue;
    }
    switch(state)
    {
    case S_READ:
      if(bt == FLG_STOP)
      {
        *len = count;
        return 0;
      }
      if(bt == FLG_CTRLESC)
      {
        state = S_ESCAPE;
        break;
      }
      *p++ = bt;
      count ++;
      break;
    case S_ESCAPE:
      bt ^= XOR_BYTE;
      *p++ = bt;
      count ++;
      state = S_READ;
      break;
    }
  }
  return 0;
}

void write_data(int ff, char* data)
{
}

const char* read_data(int ff)
{
  unsigned char buff[0x100];
  int len = read(ff, buff, sizeof(buff));
  fprintf(stderr, ">>>%d\n", len);
  return 0;
}

int get_cmd_code(const char* c)
{
  if(!strcasecmp(c, "get")) return 0x01;
  if(!strcasecmp(c, "set")) return 0x02;
  if(!strcasecmp(c, "channel_on")) return 0x03;
  if(!strcasecmp(c, "channel_off")) return 0x04;
  if(!strcasecmp(c, "group_on")) return 0x05;
  if(!strcasecmp(c, "group_off")) return 0x06;
  if(!strcasecmp(c, "group_idle")) return 0x07;
  if(!strcasecmp(c, "test")) return 0x0f;
  return 0x00;
}

const char* comrw(char* dev, char* addr, char* cmd, char* oper, char* data)
{
  unsigned char bt[5];
  const char* resp = "<html>\n<body>\nOK<hr>\n%s<br>\ndev=%s<br>\naddr=%s<br>\n"
                   "cmd=%s<br>\noper=%s<br>\ndata=%s<br>\n</body>\n</html>\n";
  const char* err = "<html>\n<body>\nERROR<hr>\n%s<br>\ndev=%s<br>\n"
                    "Error writing to port<br>\n</body>\n</html>\n";
  int ff = open(dev, O_RDWR);
  if(ff < 0)
    return "<html>\n<body>\nERR<hr>\n%s<br>\nCannot open %s<br>\n</body>\n</html>\n";
  com_init_port(ff);
  bt[0] = FLG_START;
  bt[1] = (unsigned char)atoi(addr);
  bt[2] = 0x01;
  bt[3] = (unsigned char)get_cmd_code(cmd);
  bt[4] = (unsigned char)atoi(oper);
  if(5 != write(ff, bt, 5))
  {
    close(ff);
    return err;
  }
  write_data(ff, data);
  bt[1] = bt[0] = 0;
  bt[2] = FLG_STOP;
  if(3 != write(ff, bt, 3))
  {
    close(ff);
    return err;
  }
  usleep(100000);
  read_data(ff);
  close(ff);
  return resp;
}

////////////////////////////////////////////////////////////////////////////////
//WEB SERVER
////////////////////////////////////////////////////////////////////////////////

#define SERVER "HTTPCOM"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PORT 8111

char* adate()
{
  long nt;
  char *ss, *p;
  struct tm *t;
  time(&nt);
  p = ss = asctime(localtime(&nt));
  while(*p != '\n') p ++;
  *p = 0;
  return ss;  
}

char *get_mime_type(const char *name)
{
  const char *e = name;
  while(*e && *e != '.') e ++;
  if(*e != '.')
    return 0;
  if(strcasecmp(e, ".html") == 0 || strcasecmp(e, ".htm") == 0) return "text/html";
  if(strcasecmp(e, ".jpg") == 0 || strcasecmp(e, ".jpeg") == 0) return "image/jpeg";
  if(strcasecmp(e, ".gif") == 0) return "image/gif";
  if(strcasecmp(e, ".png") == 0) return "image/png";
  if(strcasecmp(e, ".css") == 0) return "text/css";
  if(strcasecmp(e, ".au") == 0) return "audio/basic";
  if(strcasecmp(e, ".wav") == 0) return "audio/wav";
  if(strcasecmp(e, ".avi") == 0) return "video/x-msvideo";
  if(strcasecmp(e, ".mpeg") == 0 || strcasecmp(e, ".mpg") == 0) return "video/mpeg";
  if(strcasecmp(e, ".mp3") == 0) return "audio/mpeg";
  return 0;
}

void send_headers(FILE *f, int status, char *title, char *extra, char *mime, 
                  int length, time_t date)
{
  time_t now;
  char timebuf[128];
  fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
  fprintf(f, "Server: %s\r\n", SERVER);
  now = time(0);
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
  fprintf(f, "Date: %s\r\n", timebuf);
  if(extra) fprintf(f, "%s\r\n", extra);
  if(mime) fprintf(f, "Content-Type: %s\r\n", mime);
  if(length >= 0) fprintf(f, "Content-Length: %d\r\n", length);
  if(date != -1)
  {
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&date));
    fprintf(f, "Last-Modified: %s\r\n", timebuf);
  }
  fprintf(f, "Last-Modified: %s\r\n", timebuf);
  fprintf(f, "Connection: close\r\n");
  fprintf(f, "\r\n");
}

int send_error(FILE *f, int status, char *title, char *extra, char *text)
{
  send_headers(f, status, title, extra, "text/html", -1, -1);
  fprintf(f, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\r\n", status, title);
  fprintf(f, "<BODY><H4>%d %s</H4>\r\n", status, title);
  fprintf(f, "%s\r\n", text);
  fprintf(f, "</BODY></HTML>\r\n");
  return 0;
}

int send_file(FILE *f, const char *path, struct stat *statbuf)
{
  char data[4096];
  int n;
  FILE *file = fopen(path, "r");
  if(!file)
    return send_error(f, 403, "Forbidden", NULL, "Access denied.");
  int length = S_ISREG(statbuf->st_mode) ? statbuf->st_size : -1;
  send_headers(f, 200, "OK", 0, "text/html", length, statbuf->st_mtime);
  while ((n = fread(data, 1, sizeof(data), file)) > 0)
    fwrite(data, 1, n, f);
  fclose(file);
  return 0;
}

void parse_request(FILE* f, char* buf, char** method, char** path, char** protocol)
{
  *method = *path = *protocol = 0;
  if(!fgets(buf, 4096, f)) return;
  *method = buf;
  while(*buf && *buf != ' ') buf ++;
  if(*buf != ' ') return;
  *buf ++ = 0;
  *path = buf;
  while(*buf && *buf != ' ') buf ++;
  if(*buf != ' ') return;
  *buf ++ = 0;
  *protocol = buf;
  while(*buf && *buf != '\r') buf ++;
  if(*buf != '\r') *protocol = 0;
  *buf = 0;
  fprintf(stderr, "%s client request:\n--%s\n--%s\n--%s\n", adate(), *method, *path, *protocol);
}

char* crack_path(char** p)
{
  char* r = *p;
  while(**p && **p != '&') (*p) ++;
  if(**p != '&')//format error
    return 0;
  **p = 0;
  (*p) ++;
  return r;
}

int send_comrw(FILE* f, char* path)
{
  const char* resp = "<html>\n<body>\nERROR<hr>%s<br>\n</body>\n</html>\n";
  char *dev, *addr, *cmd, *oper, *data, *p = path;
  dev = crack_path(&p);
  if(!dev)
    goto send;
  addr = crack_path(&p);
  if(!addr)
    goto send;
  cmd = crack_path(&p);
  if(!cmd)
    goto send;
  oper = crack_path(&p);
  if(!oper)
    goto send;
  data = p;
  if(!*data)
    goto send;
  fprintf(stderr, "%s d:%s, a:%s, c:%s, o:%s, d:%s\n", adate(), dev, addr, cmd, oper, data);
  resp = comrw(dev, addr, cmd, oper, data);
send:
  send_headers(f, 200, "OK", 0, get_mime_type(path), -1, -1);
  fprintf(f, resp, adate(), dev, addr,cmd, oper, data);
  return 0;
}

int send_cgi(FILE* f, char* path)
{
  char* p = path;
  char* p1 = "comrw?";
  fprintf(stderr, "%s CGI request: %s\n", adate(), path);
  while(*p && *p == *p1) p ++, p1 ++;
  if(!*p1)
    return send_comrw(f, p);
  else
    return send_error(f, 404, "Not Found", 0, "CGI not found.");
}

int process_get(FILE* f, char* path)
{
  struct stat statbuf;
  if(*path == '/') path ++;
  if(*path == '?')
    return send_cgi(f, path + 1);
  if(!*path)
    path = "index.html";
  if(stat(path, &statbuf) < 0 || S_ISDIR(statbuf.st_mode))
    return send_error(f, 404, "Not Found", 0, "File not found.");
  return send_file(f, path, &statbuf);
}

int process(FILE *f)
{
  char buf[4096];
  char *method;
  char *path;
  char *protocol;
  char pathbuf[4096];
  int len;
  parse_request(f, buf, &method, &path, &protocol);
  if(!method || !path || !protocol)
  {
    fprintf(stderr, "%s invalid request\n", adate());
    return -1;
  }
  fseek(f, 0, SEEK_CUR); // Force change of stream direction
  if(strcasecmp(method, "GET"))
    send_error(f, 501, "Not supported", 0, "Method is not supported.");
  else
    process_get(f, path);
  fflush(f);
  return 0;
}

int init_server(const char* prog_name, const char* home_dir, int port, int listen_count)
{
  int s = -1, on = 1;
  struct sockaddr_in sin;
  fprintf(stderr, "%s initializing server\n", adate());
  if(chdir(home_dir))
  {
    fprintf(stderr, "%s ", adate());
    perror("cannot enter server home directory");
    return -1;
  }
  s = socket(AF_INET, SOCK_STREAM, 0);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  if(bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0 || listen(s, listen_count) < 0)
  {
    close(s);
    perror(adate());
    return -1;
  }
  fprintf(stderr, "%s %s is listening on port %d\n", adate(), prog_name, port);
  return s;
}

FILE* wait_client(int listen_socket)
{
  struct sockaddr_in a;
  int sz = sizeof(struct sockaddr_in);
  fprintf(stderr, "%s waiting for client\n", adate());
  int s = accept(listen_socket, (struct sockaddr *)&a, &sz);
  if(s < 0)
  {
    perror(adate());
    return 0;
  }
  fprintf(stderr, "%s Accepted client, socket %d\n", adate(), s);
  return fdopen(s, "a+");
}

int main(int argc, char *argv[])
{
  int s;
  FILE* f = 0;
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s http_home_dir port\n", argv[0]);
    return 1;
  }
  if((s = init_server(argv[0], argv[1], atoi(argv[2]), 5)) < 0)
    return 2;
  while((f = wait_client(s)))
  {
    process(f);
    fprintf(stderr, "%s closing client connection\n", adate());
    fclose(f);
  }
  close(s);
  return 0;
}

