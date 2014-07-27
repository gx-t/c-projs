#include "sox.h"
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

static int stop = 0;

static void ctrl_c(int sig)
{
  fprintf(stderr, "Ctrl+C\n");
  stop = 1;
}

static void output_message(unsigned level, const char *filename, const char *fmt, va_list ap)
{
  char const * const str[] = {"FAIL", "WARN", "INFO", "DBUG"};
  if(sox_globals.verbosity < level)
    return;
  char base_name[128];
  sox_basename(base_name, sizeof(base_name), filename);
  fprintf(stderr, "%s %s: ", str[min(level - 1, 3)], base_name);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

enum
{
  ERR_OK = 0
  , ERR_ARGC
  , ERR_INIT
  , ERR_INPUT
  , ERR_OUTPUT
};

static const char* argv_0 = 0;

static int show_usage(int err_code)
{
  fprintf(stderr, "Usage:\n%s <audio file>\n", argv_0);
  return err_code;
}
/*
static int process_data(sox_sample_t* data, size_t sample_count, unsigned channels)
{
  return ERR_OK;
}*/
 
int main(int argc, char * argv[])
{
  argv_0 = argv[0];
  if(argc != 2) return show_usage(1);
  sox_globals.output_message_handler = output_message;
  sox_globals.verbosity = 1;

  if(sox_init() != SOX_SUCCESS) return ERR_INIT;
  sox_format_t *in = sox_open_read(argv[1], NULL, NULL, NULL);
  if(!in)
  {
    sox_quit();
    return ERR_INPUT;
  }
  // Change "alsa" for alternative audio device driver ("oss", "ao", "coreaudio", etc.)
  sox_format_t *out = sox_open_write("default", &in->signal, NULL, "alsa", NULL, NULL);
  if(!out)
  {
    sox_close(in);
    sox_quit();
    return ERR_OUTPUT;
  }

  size_t block_size = in->signal.rate / 10 * in->signal.channels;
  sox_sample_t samples[block_size];
  signal(SIGINT, ctrl_c);
  size_t number_read;
  int result = ERR_OK;
  static const char line[] = "================================================================================";
  while(!stop && (number_read = sox_read(in, samples, block_size)))
  {
    int i;
    double avrg = 0;
    for(i = 0; i < number_read; i++)
      avrg += fabs(SOX_SAMPLE_TO_FLOAT_64BIT(samples[i],));
    avrg /= number_read;
    int offset = (1 - avrg) * (sizeof(line) - 1);
/*    result = process_data(samples, number_read, in->signal.channels);
    if(result) break;*/
    number_read = sox_write(out, samples, number_read);
    printf("%s\n", line + offset);
  }
  printf("\n");
  fprintf(stderr, "Stopping...\n");
  sox_close(out);
  sox_close(in);
  sox_quit();
  return result;
}

