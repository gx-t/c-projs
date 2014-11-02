#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char* argv[])
{
  if(argc < 3) return 1;
  int port = atoi(argv[1]);
  if(port < 1 || port > 65535) return 2;
  int ss = socket(AF_INET, SOCK_STREAM, 0);
  if(ss < 0)
  {
    perror("socket");
    return 3;
  }
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  int on = 1;
  setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  if(bind(ss, (struct sockaddr *) &sin, sizeof(sin)) < 0 || listen(ss, 1) < 0)
  {
    close(ss);
    perror("bind/listen");
    return 4;
  }
  int sc = accept(ss, 0, 0);
  close(ss);
  dup2(sc, STDOUT_FILENO);
  close(sc);
//  dup2(STDIN_FILENO, STDOUT_FILENO);
  const char hdr[] =
  "HTTP/1.0 200 OK\r\n"
  "Connection: close\r\n\r\n";
//  "Content-Type: video/mpeg\r\n\r\n";
  if(write(STDOUT_FILENO, hdr, sizeof(hdr) - 1) != sizeof(hdr) - 1)
  {
    perror("write");
    return 5;
  }
  argv += 2;
  execv(*argv, argv);
  perror("execv");
  return 6;
}


