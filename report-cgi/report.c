#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

static int r_weeknum()
{
  char buff[8];
  time_t t = time(0);
  strftime(buff, sizeof(buff), "%V", localtime(&t));
  return atoi(buff);
}

static const char* r_grpname(const char* key)
{
  const char* ss =
  "ya\0Manufacturing Test Group\0"
  "mdg\0MASIS Design Group\0"
  "rtl\0RTL Team\0"
  "qa\0QA Team\0";
  const char* p1 = ss;
  while(*p1)
  {
    const char* p2 = key;
    while(*p1 && *p2 == *p1) p1++, p2++;
    if(*p1 == *p2)
    {
      const char* value = ++p1;
      return value;
    }
    while(*p1) p1++; p1++;
    while(*p1) p1++; p1++;
  }
  return 0;
}

static void r_form(char* arg)
{
  if(!*arg) return;
  const char* group = r_grpname(arg);
  if(!group)
  {
    printf("Content-type: text/plain\r\n\r\nERROR: unknown group alias: %s\n", arg);
    return;
  }
  const char* cc1 =
  "Content-type: text/html\r\n\r\n"
  "<!DOCTYPE html>\r\n<html>\r\n"
  "<head><title>Weekly Report</title></head>\r\n"
  "<body>\r\n<h2>Weekly Report (Week %d)</h2>\r\n<h3>%s</h3>"
  "<form action=/cgi-bin/report?submit&%s method=post enctype=text/plain>\r\n"
  "Name: <input name=name type=text size=32></input>\r\n<ol>\r\n";
  printf(cc1, r_weeknum(), group, arg);
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
  while(*pp && *pp != '\r') pp ++;
  *pp = 0;
  return value;
}

static void r_report(const char* gr, const char* gname)
{
  char buff[4096];
  sprintf(buff, "../%s.html", r_weekstr());
  FILE* rf = fopen(buff, "w");
  if(!rf) return;
  fprintf(rf, "<html><head><meta http-equiv=\"pragma\" content=\"no-cache\" /></head><body>\r\n<h2><u>%s</u></h2>\r\n", gname);
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
  sprintf(buff, "abiword ../%s.html -t doc", r_weekstr());
  printf("<a href=\"/reports/%s/%s.html\">View online</a><br>\r\n", gr, r_weekstr());
  if(system(buff));
  printf("<a href=\"/reports/%s/%s.doc\">Download doc file</a>\r\n</body></html>", gr, r_weekstr());
}

static void r_submit(char* gr)
{
  if(!*gr) return;
  const char* group = r_grpname(gr);
  if(!group)
  {
    printf("Content-type: text/plain\r\n\r\nERROR: unknown group alias: %s\n", gr);
    return;
  }
  if(chdir("../reports")) return;
  mkdir(gr, 0755);
  if(chdir(gr)) return;
  char* week = r_weekstr();
  mkdir(week, 0755);
  if(chdir(week)) return;
  char* line = r_line();
  if(!line) return;
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

int main(int argc, char* argv[])
{
  char* pp = getenv("QUERY_STRING");
  if(!pp) return 1;
  char* func = pp;
  while(*pp && *pp != '&') pp++;
  if(*pp) *pp++ = 0;
  if(!strcmp(func, "form")) return r_form(pp), 0;
  if(!strcmp(func, "submit")) return r_submit(pp), 0;
  return 1;
}

