#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

static int r_weeknum()
{
  char buff[8];
  time_t t = time(0);
  strftime(buff, sizeof(buff), "%V", localtime(&t));
  return atoi(buff);
}

static void r_cutline(char* buff)
{
  while(*buff && *buff != '\r' && *buff != '\n') buff ++;
  *buff = 0;
}

static const char* r_grpname()
{
  static char gname[256];
  FILE* ff = fopen(".title", "r");
  if(!ff) return 0;
  if(fgets(gname, sizeof(gname), ff));
  fclose(ff);
  r_cutline(gname);
  return gname;
}

static void r_errgrp(const char* alias)
{
  printf("Content-type: text/plain\r\n\r\nERROR: unknown group alias: %s\r\n", alias);
}

static void r_form_name()
{
  char buff[4096];
  FILE* ff = fopen(".users", "r");
  if(!ff)
  {
    printf("Name: <input name=name type=text size=32></input>\r\n");
    return;
  }
  printf("<select name=name>\r\n");
  while(fgets(buff, sizeof(buff), ff))
  {
    r_cutline(buff);
    printf("<option value=\"%s\">%s</option>", buff, buff);
  }
  printf("</select>\r\n");
  fclose(ff);
}

static void r_form(const char* arg)
{
  if(!*arg) return;
  if(chdir("../reports"));
  if(chdir(arg)) return r_errgrp(arg);
  const char* group = r_grpname();
  const char* cc1 =
  "Content-type: text/html\r\n\r\n"
  "<!DOCTYPE html>\r\n<html>\r\n"
  "<head><title>Weekly Report</title></head>\r\n"
  "<body>\r\n<h2>Weekly Report (Week %d)</h2>\r\n<h3>%s</h3>"
  "<form action=/cgi-bin/report?submit&%s method=post enctype=text/plain>\r\n";
  printf(cc1, r_weeknum(), group, arg);
  r_form_name();
  printf("<ol>\r\n");
  int i = 16;
  while(i--) puts("<li><input name=task type=text size=64></input></li>");
  printf("</ol>\r\n<h2>Next Week (Week %d):</h2>\r\n<input name=ww type=hidden value=---------></input>\r\n<ol>\r\n", r_weeknum() + 1);
  i = 16;
  while(i--) puts("<li><input name=task type=text size=64></input></li>");
  puts("</ol>\r\n<input type=submit value=Submit></input>\r\n</form>\r\n</body>\r\n</html>");
}

static char* r_weekstr()
{
  static char buff[8];
  time_t t = time(0);
  strftime(buff, sizeof(buff), "%V%y", localtime(&t));
  return buff;
}

static char* r_line()
{
  static char buff[4096];
  if(!fgets(buff, sizeof(buff), stdin)) return 0;
  char* pp = buff;
  while(*pp && *pp != '=') pp++;
  if(!*pp) return 0;
  char* value = ++pp;
  r_cutline(value);
  return value;
}

//TODO: Special symbols in input: <, >, ---------

static void r_report(const char* gr, const char* gname)
{
  char buff[4096];
  sprintf(buff, "../%s.html", r_weekstr());
  FILE* rf = fopen(buff, "w");
  if(!rf) return;
  fprintf(rf, "<html><head><meta http-equiv=Expires content=0 /></head><body>\r\n<h2><u>%s</u></h2>\r\n", gname);
  struct dirent *de = 0;
  DIR* pd = opendir(".");
  while((de = readdir(pd)))
  {
    if(de->d_type != DT_REG) continue;
    FILE* ff = fopen(de->d_name, "r");
    fprintf(rf, "<h2>%s</h2>\r\n", de->d_name);//name
    fprintf(rf, "<ol>\r\n");
    while(fgets(buff, sizeof(buff), ff) && strcmp(buff, "---------\r\n")) fprintf(rf, "<li>%s</li>\r\n", buff);
    fprintf(rf, "</ol>\r\nWW%d tasks:<ol>\r\n", r_weeknum() + 1);
    while(fgets(buff, sizeof(buff), ff)) fprintf(rf, "<li>%s</li>\r\n", buff);
    fprintf(rf, "</ol>\r\n");
    fclose(ff);
  }
  closedir(pd);
  fprintf(rf, "</body></html>\r\n");
  fclose(rf);
  sprintf(buff, "abiword ../%s.html -t docx", r_weekstr());
  printf("<a href=\"/reports/%s/%s.html\">View online</a><br>\r\n", gr, r_weekstr());
  if(system(buff));
  printf("<script>parent.frames['list'].location.reload();</script>\r\n");
  printf("<a href=\"/reports/%s/%s.docx\">Download docx file</a>\r\n</body></html>", gr, r_weekstr());
}

static void r_erruser()
{
  printf("Content-type: text/plain\r\n\r\nERROR: Name field is empty\r\n");
}

static void r_submit(const char* gr)
{
  if(!*gr) return;
  if(chdir("../reports")) return;
  if(chdir(gr)) r_errgrp(gr);
  const char* group = r_grpname();
  char* week = r_weekstr();
  mkdir(week, 0755);
  if(chdir(week)) return;
  char* line = r_line();
  if(!*line) return r_erruser();
  FILE* ff = fopen(line, "w");
  if(!ff)
  {
    return;
  }
  const char* cc1 = 
  "Content-type: text/html\r\n\r\n<html><body>Submited.<hr>\r\nUser: %s<br>\r\nGroup: %s<br>\r\n";
  printf(cc1, line, group);
  while((line = r_line())) {
    if(!*line) continue;
    fprintf(ff, "%s\r\n", line);
  }
  fclose(ff);
  printf("<hr>\r\n");
  r_report(gr, group);
}

static void r_list(const char* gr)
{
  if(!*gr) return;
  if(chdir("../reports")) return;
  if(chdir(gr)) r_errgrp(gr);
  printf("Content-type: text/html\r\n\r\n<html><head><meta http-equiv=Expires content=0 /></head><body><h2>Report archive</h2><table style=\"width: 200px\">\r\n");
  struct dirent *de = 0;
  DIR* pd = opendir(".");
  while((de = readdir(pd)))
  {
    if(de->d_type != DT_REG) continue;
    char* pp = de->d_name;
    while(*pp && *pp != '.') pp++;
    if(*pp) *pp++ = 0;
    if(strcmp(pp, "html")) continue;
    printf("<tr><td><b>%s</b></td><td><a href=/reports/%s/%s.html><i>html</i></a></td><td><a href=/reports/%s/%s.docx><i>docx</i></a></td></tr>\r\n", de->d_name, gr, de->d_name, gr, de->d_name);
  }
  closedir(pd);
  printf("</table></body></html>\n");
}

static void r_time(const char* fmt)
{
  char buff[4096];
  time_t t = time(0);
  strftime(buff, sizeof(buff), fmt, localtime(&t));
  printf("Content-type: text/plain\r\n\r\n%s\r\n", buff);
}

int main(int argc, char* argv[])
{
  char* pp = getenv("QUERY_STRING");
  if(!pp) return 1;
  char* func = pp;
  while(*pp && *pp != '&') pp++;
  if(*pp) *pp++ = 0;
  if(!strcmp(func, "form")) return r_form(pp), 0;
  if(!strcmp(func, "submit")) return r_submit(pp), 0;
  if(!strcmp(func, "list")) return r_list(pp), 0;
  if(!strcmp(func, "time")) return r_time(pp), 0;
  return 1;
}

