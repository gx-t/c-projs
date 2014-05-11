#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

struct DATA
{
  char* name;
  int time;
  int cost;
  struct DATA* next;
};

int main(int argc, char* argv[])
{
  char buff[4096];
  struct DATA* pdata = 0;
  //get row by row, ignore rows that do not satisfy criteria
  while(fgets(buff, sizeof(buff), stdin))
  {
    int cnt = 0;
    char* line = buff;
    while(*line && *line != '\n' && *line != '\r')
    {
      cnt += (*line == '\t');
      line ++;
    }
    *line = 0;
    if(cnt != 5 && cnt != 6) continue;//6 tabs, last row usually has 5 tabs
    cnt = 0;
    line = buff;
    while(cnt < 3)//nothing in first 3 columns
    {
      cnt += (*line == '\t');
      line ++;
    }
    char* stime = line;
    if(!isdigit(*stime)) continue;//time always begins with digit
    for(; *line != '\t'; line++);
    *line++ = 0;
    char* soper = line;
    for(; *line != '\t'; line++);
    *line++ = 0;
    char* scost = line;
    if(!isdigit(*scost)) continue;//cost always begins with digit
    while(*line && *line != '\t') line ++;//if num of cols is 6 last column ends with tab
    *line = 0;//terminate on tab
    if(!*soper) soper = "Orange";
    int itime = 0;
    while(*stime)//time to seconds conversion
    {
      int iitime = 0;
      while(*stime && *stime != ':')
      {
        iitime *= 10;
        iitime += (*stime - '0');
        stime ++;
      }
      itime += iitime;
      if(*stime == ':')
      {
        itime *= 60;
        stime ++;
      }
    }
    int icost = 0;
    while(*scost)//atoi
    {
      icost *= 10;
      icost += (*scost - '0');
      scost ++;
    }
    struct DATA** pp = &pdata;
    while(*pp && strcmp((*pp)->name, soper))//operator name lookup
      pp = &(*pp)->next;
    if(!*pp)//not, create new
    {
      *pp = alloca(sizeof(struct DATA));
      (*pp)->name = alloca(strlen(soper) + 1);
      strcpy((*pp)->name, soper);
      (*pp)->time = 0;
      (*pp)->cost = 0;
      (*pp)->next = 0;
    }
    (*pp)->time += itime;
    (*pp)->cost += icost;
  }
  struct DATA* pp = pdata;
  int id = 0;
  puts("Content-type: text/html; charset=UTF-8\n\n"
  "<!DOCTYPE html>\n<html>\n"
  "<head>\n<title>Արդյունքները</title>\n<script>\nfunction recalc(){\nvar tc=0\n");
  for(; pp; pp=pp->next, id++)
  {
    printf("var cost=price_%d.value*time_%d.innerHTML;cost_%d.innerHTML=cost;\ntc+=cost;\n", id, id, id);
  }
  puts("cost_total.innerHTML=tc;\nprice_total.innerHTML=tc/time_total.innerHTML;\n}\n</script>\n</head>\n""<body>\n<h2>Արդյունքները</h2>\n"
  "<table border=1>\n<tr><td>Օպերատորի կամ վայրի անվանումը</td><td>Տևողությունը (րոպե)</td><td>Գումարը (դրամ)</td><td>Րոպեի միջին արժեքը (դրամ)</td></tr>");
  int total_time = 0;
  int total_cost = 0;
  for(pp = pdata, id=0; pp; pp = pp->next, id++)//print results
  {
    puts("<tr>");
    printf("<td>%s</td><td id=time_%d>%g</td><td id=cost_%d>%d</td><td><input id=price_%d type=number value=%g></input></td>\n", pp->name, id, (double)pp->time / 60, id, pp->cost, id, (double)pp->cost * 60 / pp->time);
    puts("</tr>");
    total_time += pp->time;
    total_cost += pp->cost;
  }
  printf("<tr>\n<td>Բոլորը միասին</td><td id=time_total>%g</td><td id=cost_total>%d</td><td id=price_total>%g</td>\n</tr>\n", (double)total_time / 60, total_cost, (double)total_cost * 60 / total_time);
  puts("</table>\n<hr>\n<input type=button value=Վերահաշվել onclick=\'recalc();\'></input>\n</body>\n</html>");
  return 0;
}
