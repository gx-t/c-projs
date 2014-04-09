#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void r_head(const char* title)
{
  printf("Content-type: text/html\nConnection: close\n\n"
          "<!DOCTYPE html>\n<html>\n<head>\n"
          "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>"
          "<h2>%s</h2>\n</head>\n<body>\n", title);
}

static void r_tail()
{
  puts("</body>\n</html>");
}

static int r_weeknum()
{
  char buff[8];
  time_t t = time(0);
  strftime(buff, sizeof(buff), "+%V", localtime(&t));
  return atoi(buff);
}

static void r_tasks(int count)
{
  while(count--) puts("<li><input name=task type=text size=64></input></li>");
}

static int r_form()
{
  r_head("Weekly Report");
  puts("<form action=/cgi-bin/report-submit.cgi method=post enctype=text/plain>\n"
        "Name: <input name=name type=text size=32></input><br/>\n<ol>");
  r_tasks(10);
  int ww = r_weeknum();
  printf("</ol>\n<hr>\n<h4>WW%d</h4>\n"
          "<input name=ww type=hidden value=%d></input>\n<ol>\n", ww + 1, ww + 1);
  r_tasks(10);
  puts("</ol>\n<input type=submit value=Submit></input>\n</form>");
  r_tail();
  return 0;
}

static int r_submit()
{
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2) return 1;
  if(!strcmp(argv[1], "form")) return r_form();
  if(!strcmp(argv[1], "submit")) return r_submit();
  return 1;
}

