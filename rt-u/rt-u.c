/*
21.05.2012:
-- renames sfdf_size_cmp to fs_size_cmp cause not sfdf specific
-- moved sort by hash to sfdf_sort from sfdf_output
-- renamed sfdf_sha256_cmp to fs_sha256_cmp cause not sfdf specific
-- deleted sfdf_dump_roots and moved body into sfdf_output
-- deleted !file_count check, sfdf_output argument: file_count (>=2 always)
-- finished sffd_sort()
-- added empty sffd_output() with arguments
22.05.2012
-- added *_process functions incapsulating idx_arr[file_count] related stuff
-- the *_output functions are called from in *_process
-- moved file count check (>=2) to rtfl_collect
-- added group_idx to FS_FILE_ENTRY, initialized at output
-- removed root list output
-- binary output instead of tcl. Prints FS_FILE_ENTRY blocks to stdout
23.05.2012
-- fixed group count calculation
-- added "sfdf" 4-byte signature at the beginning of output
25.05.2012
-- removed root directories storage in tmp file
-- added relative path offset to file record
-- in sffd_sort qsort is now applied to relative path only (faster)
31.05.2012
-- removed last "goto"
-- added rtfl_collect_roots, \0 separated root paths with extra \0 at the end
-- moved single root collection proc. to rtfl_scan_root
-- improved error handling
-- added root path indexing to rtfl_idx
01.06.2012
-- renamed to rt-u and added to code.google.com
04.06.2012
-- added bin2txt, bin2txt_main
-- added bin2txt_sfdf
-- wrote sffd_output content
09.06.2012
-- added argument: sfcl - reduced structure stdout output simple collecting
-- added function bin2txt_sfcl
11.06.2012
-- added simpler output structure: SFFD_OUTPUT_RECORD
-- completed sffd_output
-- adopted bin2txt_sffd to new output
-- completed bin2txt_sffd
-- added simpler output structure: SFDF_OUTPUT_RECORD
-- adopted bin2txt_sfdf to new output
-- removed unneeded fields from struct FS_FILE_ENTRY
-- removed test directory from svn - svn special files cause a lot of garbage in output
12.06.2012
-- added long operation status report support sending SIGUSR1 to process
-- added status report for sha256_file
-- added status report for rtfl_collect
-- sfcl: used regular collecting function
-- sfcl: adopted bin2txt_sfcl
14.06.2012
-- unified binary output
-- unified bin2txt
-- started regexp support in bin2txt
18.06.2012
-- finished simple (no highlight) regexp support
19.06.2012
-- fixed: printed regexp compilation error message to stdout instead of stderr
25.06.2012
-- rt_tmp_file: On Android /sdcard/rt-uXXXXXX or tmpfile for other OSs
-- included status char to regural expression processing
-- better error message for ERR_BIN2TXT_UNKNOWN_DATA
26.06.2012
-- special chars convert to escape sequence
-- added '"` to the list for escape sequence
-- fixed bug with number of files in sfcl output
28.06.2012
-- bug fix, mmap returns MAP_FAILED on error, not NULL
-- removed branch of data == 0 in rtfl_collect: collect output to stdout is not used
-- added macro CHECK_SIGN for input stream type check
-- added gds text dump functions and arguments support
-- added test for GDS dump
-- added status report to GDS scan loop
-- moved report callbacks to functions where they are called, made names shorter
29.06.2012
-- replaced pow(), removed math lib dependency
-- removed unused error codes
01.07.2012
-- started develop http server
02.07.2012
-- added log through stderr for client connections
-- added global union for tools
-- added client process counter
-- added maximum connection count limitation
04.07.2012
-- used STDOUT_FILENO etc. istead of numbers
14.07.2012
-- bug fix, sffd, calculation of the number of differend size file
-- bug fix, sffd, correct assignment of <>
20.07.2012
-- used MAP_PRIVATE instead of MAP_SHARED
-- compiled for Atmel AT92SAM7 board
21.07.2012
-- added missing MAP_FILE flag to mmap call
14.08.2012
-- replaced * -status with | -status and excluded status from regexp processing
02.07.2014
-- added jpeg-gps function
*/

/*
WORKFLOW sfdf:
  1. Scan each of input directories, colect file size, time, full path
  2. Create indexes for colelcted files
  3. Sort files by size
  4. Group files by equal size if 2
  5. Calculate hash code (sha256) for each file of each group
  6. Sort by hash code
  7. Group files by equal hash code if 2 or more files have same hash
  8. Output the colected groups

WORKFLOW sffd:
  1. Scan each of input directories, colect file size, time, full path
  2. Create indexes for colelcted files
  3. Mark file groups based on scan roots
  4. Sort files of each group by relative path
  ...
*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <regex.h>

enum
{
  ERR_OK = 0
  , ERR_NO_FUNC
  , ERR_UNKNOWN_FUNC
////////////////////////////////////////////////////////////////////////////////
//sfdf
  , ERR_SFDF_NO_ARG
//sffd
  , ERR_SFFD_COUNT_ARGS
//common
  , ERR_CREATE_TMP_FILE
  , ERR_OPEN_DIR
  , ERR_WRITE_FILE
  , ERR_CHDIR_UP
  , ERR_NOT_ENOUGH_INPUT
  , ERR_CHDIR
  , ERR_COLLECT_ROOTS
  , ERR_NO_FILE_INFO
  , ERR_MAPPING
  , ERR_PATH_TOO_DEEP
  , ERR_CORRUPTED_INPUT
  , ERR_EMPTY_INPUT
//bin2txt
  , ERR_BIN2TXT_COUNT_ARGS
  , ERR_BIN2TXT_UNKNOWN_DATA
  , ERR_BIN2TXT_REGEX
//sfcl
  , ERR_SFCL_NO_ARG
//gds
  , ERR_GDS_COUNT_ARGS
  , ERR_GDS_UNKNOWN_SUBFUNCTION
  , ERR_GDS_DUMP_UNKNOWN_OUTPUTTYPE
  , ERR_GDS_DTYPE
  , ERR_GDS_DLENGTH
//http
  , ERR_HTTP_COUNT_ARGS
  , ERR_HTTP_INVALID_PORT
  , ERR_HTTP_INVALID_HOME_DIR
  , ERR_HTTP_BIND_LISTEN
  , ERR_HTTP_ACCEPT
  , ERR_HTTP_CONNECTION_CLOSED
//jpeg
  , ERR_JPEG_INVALID_STREAM
  , ERR_JPEG_NO_EXIF
  , ERR_JPEG_EXIF_HEADER
  , ERR_JPEG_EXIF
};

static FILE* rt_tmp_file()
{
#ifdef ANDROID
  char tmpl[] = "/sdcard/rt-uXXXXXX";
  int tmp = mkstemp(tmpl);
  if(tmp == -1) return 0;
  return fdopen(tmp, "r+");
#else
  return tmpfile();
#endif
}

static void rt_stdout_indent(int indent)
{
  while(indent--)
    printf("  ");
}

static short rt_byte_array_to_short(unsigned char* ss)
{
  return (ss[0] << 8) + ss[1];
}

static int rt_byte_array_to_int(unsigned char* ss)
{
  return (ss[0] << 24) + (ss[1] << 16) + (ss[2] << 8) + ss[3];
}

static double rt_byte_array_to_double(unsigned char* ss)
{
  int sign = *ss & 0x80;
  int exp = (*ss++ & 0x7f) - 64;
  unsigned long long imantissa = *ss++;
  imantissa <<= 8;
  imantissa += *ss++;
  imantissa <<= 8;
  imantissa += *ss++;
  imantissa <<= 8;
  imantissa += *ss++;
  imantissa <<= 8;
  imantissa += *ss++;
  imantissa <<= 8;
  imantissa += *ss++;
  imantissa <<= 8;
  imantissa += *ss;
  double dmantissa = (double)imantissa / ((unsigned long long)1 << 56);
  double res = 1;
  if(exp < 0)
  {
    while(exp ++)
      res /= 16;
  }
  else
  {
    while(exp --)
      res *= 16;
  }
  res *= dmantissa;
  if(sign)
    res = -res;
  return res;
}

static void rt_empty_report() { fputs("", stderr); }

struct RT_STATUS
{
  void (*report)();
  const char* path;
  off_t byte;
  time_t time;
  unsigned count;
}static g_status = {rt_empty_report, ""};

char* rt_adate()
{
  long nt;
  char *ss, *p;
  time(&nt);
  p = ss = asctime(localtime(&nt));
  while(*p != '\n') p ++;
  *p = 0;
  return ss;  
}

union
{
  struct HTTP_GLOBALS
  {
    int conn_count;
  }*http;
}g_glb = {0};

////////////////////////////////////////////////////////////////////////////////
//SHA256

struct sha256_ctx
{
  unsigned total[2];
  unsigned state[8];
  unsigned char buffer[64];
};

//******************************************************************************
//sha256_starts
static void sha256_starts(struct sha256_ctx *ctx)
{
  ctx->total[0] = 0;
  ctx->total[1] = 0;
  
  ctx->state[0] = 0x6A09E667;
  ctx->state[1] = 0xBB67AE85;
  ctx->state[2] = 0x3C6EF372;
  ctx->state[3] = 0xA54FF53A;
  ctx->state[4] = 0x510E527F;
  ctx->state[5] = 0x9B05688C;
  ctx->state[6] = 0x1F83D9AB;
  ctx->state[7] = 0x5BE0CD19;
}

#define  Ch(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
#define  Maj(x, y, z)  (((x) & (y)) ^ ((z) & ((x) ^ (y))))
#define  Rot32(x, s)  (((x) >> s) | ((x) << (32 - s)))
#define  SIGMA0(x)  (Rot32(x, 2) ^ Rot32(x, 13) ^ Rot32(x, 22))
#define  SIGMA1(x)  (Rot32(x, 6) ^ Rot32(x, 11) ^ Rot32(x, 25))
#define  sigma0(x)  (Rot32(x, 7) ^ Rot32(x, 18) ^ ((x) >> 3))
#define  sigma1(x)  (Rot32(x, 17) ^ Rot32(x, 19) ^ ((x) >> 10))

//******************************************************************************
//sha256_process
static void sha256_process(struct sha256_ctx *ctx, unsigned char data[64])
{
  static const unsigned SHA256_K[64] =
  {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };

  unsigned a, b, c, d, e, f, g, h, t, T1, T2, W[64], *H = ctx->state;
  unsigned char* cp = data;
    
  for (t = 0; t < 16; t++, cp += 4)
    W[t] = (cp[0] << 24) | (cp[1] << 16) | (cp[2] << 8) | cp[3];
    
  for (t = 16; t < 64; t++)
    W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

  a = H[0]; b = H[1]; c = H[2]; d = H[3];
  e = H[4]; f = H[5]; g = H[6]; h = H[7];
    
  for (t = 0; t < 64; t++)
  {
    T1 = h + SIGMA1(e) + Ch(e, f, g) + SHA256_K[t] + W[t];
    T2 = SIGMA0(a) + Maj(a, b, c);
    h = g; g = f; f = e; e = d + T1;
    d = c; c = b; b = a; a = T1 + T2;
  }
  H[0] += a; H[1] += b; H[2] += c; H[3] += d;
  H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

//******************************************************************************
//sha256_update
static void sha256_update(struct sha256_ctx *ctx, unsigned char *input, unsigned length)
{
  unsigned left, fill;

  if(!length) return;

  left = ctx->total[0] & 0x3F;
  fill = 64 - left;

  ctx->total[0] += length;
  ctx->total[0] &= 0xFFFFFFFF;

  if( ctx->total[0] < length )
    ctx->total[1]++;

  if( left && length >= fill )
  {
    memcpy((void*)(ctx->buffer + left), (void*)input, fill);
    sha256_process(ctx, ctx->buffer);
    length -= fill;
    input  += fill;
    left = 0;
  }

  while(length >= 64)
  {
    sha256_process(ctx, input);
    length -= 64;
    input  += 64;
  }

  if(length)
    memcpy((void *)(ctx->buffer + left), (void *)input, length);
}

//******************************************************************************
//sha256_finish
static void sha256_finish(struct sha256_ctx *ctx, unsigned char digest[32])
{
  static unsigned char sha256_padding[64] =
  {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  unsigned last, padn, high, low, t, *p;
  unsigned char msglen[8], *cp;

  high = (ctx->total[0] >> 29) | (ctx->total[1] <<  3);
  low  = (ctx->total[0] <<  3);

  msglen[0] = (unsigned char)(high >> 24);
  msglen[1] = (unsigned char)(high >> 16);
  msglen[2] = (unsigned char)(high >> 8);
  msglen[3] = (unsigned char)high;
  msglen[4] = (unsigned char)(low >> 24);
  msglen[5] = (unsigned char)(low >> 16);
  msglen[6] = (unsigned char)(low >> 8);
  msglen[7] = (unsigned char)low;

  last = ctx->total[0] & 0x3F;
  padn = (last < 56) ? (56 - last) : (120 - last);

  sha256_update(ctx, sha256_padding, padn);
  sha256_update(ctx, msglen, 8);

  cp = digest;
  p = ctx->state;
  for(t = 0; t < 8; t ++, p ++, cp += 4)
  {
    cp[0] = (unsigned char)(*p >> 24);
    cp[1] = (unsigned char)(*p >> 16);
    cp[2] = (unsigned char)(*p >> 8);
    cp[3] = (unsigned char)*p;
  }
}

struct FS_FILE_ENTRY
{
  unsigned short rec_len;
  unsigned char flg_hashed : 1;
  off_t size;
  time_t time;
  unsigned char hash[32];
  unsigned group_idx;
  unsigned root_idx;
  unsigned short rel_path_offset;
  char name[];
}__attribute__((__packed__));

//******************************************************************************
//sha256_file_report
static void sha256_file_report()
{
  fprintf(stderr, "--- hashing --- %llu %d --- file:\n%s\n", g_status.byte, g_status.count, g_status.path);
}

//******************************************************************************
//sha256_file
static void sha256_file(struct FS_FILE_ENTRY* pe, unsigned* selected_count)
{
  struct sha256_ctx ctx;
  unsigned char buf[0x100];
  size_t len;
  FILE* f = fopen(pe->name, "r");
  if(!f)
  {
    perror(pe->name);
    return;
  }
  sha256_starts(&ctx);
  g_status.path = pe->name;
  g_status.byte = 0;
  g_status.count ++;
  g_status.report = sha256_file_report;
  while((len = fread(buf, 1, 0x100, f)))
  {
    sha256_update(&ctx, buf, len);
    g_status.byte += len;
  }
  g_status.report = rt_empty_report;
  if(ferror(f))
  {
    perror(pe->name);
    fclose(f);
    return;
  }
  sha256_finish(&ctx, pe->hash);
  if(selected_count) ++(*selected_count);
  pe->flg_hashed = 1;
  fclose(f);
}

struct FS_COLLECT
{
  int err;
  unsigned* file_count;
  FILE* fd;
  unsigned root_idx;
  unsigned short rel_path_offset;
};

////////////////////////////////////////////////////////////////////////////////
//******************************************************************************
//fs_upfilt
static int fs_upfilt(const char* nm)
{
  return (*nm == '.' && !nm[1]) || (*nm == '.' && nm[1] == '.' && !nm[2]);
}

#define BUFF_SIZE 0x10000

//******************************************************************************
//rtfl_collect_file
static void rtfl_collect_file(struct FS_COLLECT* ff, const char* file_name)
{
  char path[BUFF_SIZE];
  size_t path_len = 1;
  struct stat st;
  struct FS_FILE_ENTRY fe = {sizeof(struct FS_FILE_ENTRY)};
  fe.root_idx = ff->root_idx;
  if(-1 == lstat(file_name, &st))
  {
    perror(file_name);
    return;
  }
  g_status.path = file_name;
  g_status.count ++;
  fe.size = st.st_size;
  fe.time = st.st_mtime;
  path_len += strlen(realpath(file_name, path));
  fe.rec_len += path_len;
  fe.rel_path_offset = ff->rel_path_offset;
  if(sizeof(fe) != fwrite(&fe, 1, sizeof(fe), ff->fd) || path_len != fwrite(path, 1, path_len, ff->fd))
  {
    ff->err = ERR_WRITE_FILE;
    perror("Cannot write to file list file");
    return;
  }
  (*ff->file_count) ++;
}

//******************************************************************************
//rtfl_collect_dir
static void rtfl_collect_dir(struct FS_COLLECT* ff)
{
  struct dirent *de = 0;
  DIR* pd = opendir(".");
  if(!pd)
  {
    perror("Cannot open directory to scan. Giving up");
    ff->err = ERR_OPEN_DIR;
    return;
  }
  while(!ff->err && (de = readdir(pd)))
  {
    if(de->d_type == DT_DIR)
    {
      if(fs_upfilt(de->d_name))
        continue;
      if(!chdir(de->d_name))
      {
        g_status.path = de->d_name;
        rtfl_collect_dir(ff);
        if(chdir(".."))
        {
          ff->err = ERR_CHDIR_UP;
          perror(0);
        }
      }
      else
        perror(de->d_name);
    }
    else if(de->d_type == DT_REG)
      rtfl_collect_file(ff, de->d_name);
  }
  closedir(pd);
}

//******************************************************************************
//rtfl_relative_offset
static void rtfl_relative_offset(struct FS_COLLECT* ff)
{
  char path[BUFF_SIZE];
  if(!getcwd(path, sizeof(path)))
  {
    perror("The path is too deep");
    ff->err = ERR_PATH_TOO_DEEP;
  }
  ff->rel_path_offset = strlen(path) + 1;
}

//******************************************************************************
//rtfl_collect_roots
static int rtfl_collect_roots(FILE* fd, char* init_dir, int root_count, char** root_arr)
{
  char buff[BUFF_SIZE];
  while(root_count -- && !ferror(fd))
  {
    if(chdir(*root_arr))
    {
      perror(*root_arr);
      return ERR_CHDIR;
    }
    if(!getcwd(buff, sizeof(buff)))
    {
      perror(buff);
      return ERR_COLLECT_ROOTS;
    }
    if(chdir(init_dir))
    {
      perror(init_dir);
      return ERR_CHDIR;
    }
    fprintf(fd, "%s", buff);
    fputc(0, fd);
    root_arr ++;
  }
  fputc(0, fd);
  if(ferror(fd))
  {
    perror("Error collecting roots");
    return ERR_COLLECT_ROOTS;
  }
  return ERR_OK;
}

//******************************************************************************
//rtfl_scan_root
static void rtfl_scan_root(struct FS_COLLECT* ff, char* root, char* init_dir)
{
  if(chdir(root))
  {
    perror(root);
    ff->err = ERR_CHDIR;
    return;
  }
  if((rtfl_relative_offset(ff), ff->err) || (rtfl_collect_dir(ff), ff->err))
    return;
  if(chdir(init_dir))
  {
    perror(init_dir);
    ff->err = ERR_CHDIR;
    return;
  }
  ff->root_idx ++;
}

//******************************************************************************
//rtfl_collect_report
static void rtfl_collect_report()
{
  fprintf(stderr, "--- collecting --- %d --- file:\n%s\n", g_status.count, g_status.path);
}

//******************************************************************************
//rtfl_collect
static int rtfl_collect(unsigned char** data, off_t* len, unsigned* file_count, int root_count, char** root_arr)
{
  struct stat st = {0};
  struct FS_COLLECT ff = {ERR_OK, file_count};
  char* init_dir = 0;
  int i;
  *file_count = 0;
  *data = 0;
  *len = 0;
  ff.fd = rt_tmp_file();
  if(!ff.fd)
  {
    perror("Cannot create temp file");
    return ERR_CREATE_TMP_FILE;
  }
  init_dir = getcwd(0, 0);
  fprintf(stderr, "Collecting roots, %d total\n", root_count);
  ff.err = rtfl_collect_roots(ff.fd, init_dir, root_count, root_arr);
  ff.root_idx = 0;
  g_status.count = 0;
  g_status.report = rtfl_collect_report;
  for(i = 0; !ff.err && i < root_count; i ++)
  {
    g_status.path = root_arr[i];
    fprintf(stderr, "Collecting file list for input: %s\n", root_arr[i]);
    rtfl_scan_root(&ff, root_arr[i], init_dir);
  }
  g_status.report = rt_empty_report;
  free(init_dir);
  if(ff.err)
  {
    if(ff.fd != stdout) fclose(ff.fd);
    return ff.err;
  }
  fflush(ff.fd);
  fstat(fileno(ff.fd), &st);
  if(!st.st_size)
  {
    fprintf(stderr, "No file information collected, empty or inaccessable inputs\n");
    fclose(ff.fd);
    return ERR_NO_FILE_INFO;
  }
  *len = st.st_size;
  *data = mmap(0, st.st_size, PROT_WRITE | PROT_READ, MAP_FILE | MAP_PRIVATE, fileno(ff.fd), 0);
  if(ff.fd != stdout) fclose(ff.fd);
  if(MAP_FAILED == *data)
  {
    puts("111");
    perror(0);
    return ERR_MAPPING;
  }
  if(*file_count < 2)
  {
    fprintf(stderr, "At least 2 files are needed to process.\n");
    munmap(*data, st.st_size);
    return ERR_NOT_ENOUGH_INPUT;
  }
  return ERR_OK;
}

//******************************************************************************
//rtfl_idx_report
static void rtfl_idx_report()
{
  fprintf(stderr, "--- indexing --- %d --- file:\n%s\n", g_status.count, g_status.path);
}

//******************************************************************************
//rtfl_idx
static void rtfl_idx(unsigned file_count, unsigned char* data, struct FS_FILE_ENTRY** idx_arr, char** roots)
{
  fprintf(stderr, "Indexing the list of %d collected files...\n", file_count);
  g_status.count = 0;
  g_status.report = rtfl_idx_report;
  while(*data)
  {
    g_status.path = (char*)data;
    *roots ++ = (char*)data;
    while(*data) data++;
    data ++;
  }
  data ++;
  struct FS_FILE_ENTRY** idx = idx_arr;
  while(file_count --)
  {
    g_status.count ++;
    *idx = (struct FS_FILE_ENTRY*)data;
    g_status.path = (*idx)->name;
    data += (*idx)->rec_len;
    idx ++;
  }
  g_status.report = rt_empty_report;
}

//******************************************************************************
//fs_size_cmp
static int fs_size_cmp(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  if((*p1)->size < (*p2)->size)
    return -1;
  if((*p1)->size > (*p2)->size)
    return 1;
  return 0;
}

//******************************************************************************
//fs_sha256_cmp
static int fs_sha256_cmp(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  return memcmp((*p2)->hash, (*p1)->hash, 32);
}

//******************************************************************************
//fs_rel_path_cmp
static int fs_rel_path_cmp(const char* name, struct FS_FILE_ENTRY** p2)
{
  return strcmp(name, &(*p2)->name[(*p2)->rel_path_offset]);
}


//******************************************************************************
//sfdf_sort
static void sfdf_sort(unsigned file_count, struct FS_FILE_ENTRY** idx_arr, unsigned* selected_count)
{
  if(!file_count)
    return;
  struct FS_FILE_ENTRY** p1 = idx_arr;
  struct FS_FILE_ENTRY** p2 = idx_arr;
  int loop = 1;
  *selected_count = 0;
  fprintf(stderr, "Sorting %d files by size...\n", file_count);
  qsort(idx_arr, file_count, sizeof(void*), (int (*)(const void*, const void*))fs_size_cmp);
  fprintf(stderr, "Calculating hashes for equal size files...\n");
  int i = file_count;
  g_status.count = 0;
  while(--i)
  {
    p2 ++;
    if((*p1)->size == (*p2)->size)
    {
      if(loop)
      {
        loop = 0;
        sha256_file(*p1, selected_count);
      }
      sha256_file(*p2, selected_count);
    }
    else
      loop = 1;
    p1 = p2;
  }
  fprintf(stderr, "Sorting %d files by hash, extracting identical files...\n", *selected_count);
  qsort(idx_arr, file_count, sizeof(void*), (int (*)(const void*, const void*))fs_sha256_cmp);
}


//******************************************************************************
//sfdf_output
struct RTFL_OUTPUT_RECORD
{
  char status;
  unsigned short path_len;
}__attribute__((__packed__));

static void sfdf_output(unsigned selected_count, struct FS_FILE_ENTRY** idx_arr)
{
  printf("sfdf");
  if(!selected_count)
    return;
  int count = 0;
  int group = -1;
  int loop = 1;
  struct FS_FILE_ENTRY** p1 = idx_arr;
  struct FS_FILE_ENTRY** p2 = idx_arr;
  while(--selected_count)
  {
    p2 ++;
    struct RTFL_OUTPUT_RECORD fe = {0};
    if(!fs_sha256_cmp(p1, p2))
    {
      if(loop)
      {
        loop = 0;
        count ++;
        group ++;
        fe.status = '|';
        fe.path_len = (*p1)->rec_len - sizeof(struct FS_FILE_ENTRY);
        fwrite(&fe, sizeof(fe), 1, stdout);
        fwrite((*p1)->name, fe.path_len, 1, stdout);
      }
      count ++;
      fe.status = '+';
      fe.path_len = (*p2)->rec_len - sizeof(struct FS_FILE_ENTRY);
      fwrite(&fe, sizeof(fe), 1, stdout);
      fwrite((*p2)->name, fe.path_len, 1, stdout);
    }
    else
      loop = 1;
    p1 = p2;
  }
  fprintf(stderr, "Found %d group(s) of identical files, %d files total.\n", ++group, count);
}

//******************************************************************************
//sfdf_process
static int sfdf_process(unsigned file_count, unsigned char* data, unsigned root_count)
{
  unsigned selected_count = 0;
  struct FS_FILE_ENTRY* idx_arr[file_count];
  char* roots[root_count];
  rtfl_idx(file_count, data, idx_arr, roots);
  sfdf_sort(file_count, idx_arr, &selected_count);
  sfdf_output(selected_count, idx_arr);
  return ERR_OK;
}

//******************************************************************************
//sfdf_main
static int sfdf_main(int argc, char* argv[])
{
  if(argc < 2)
  {
    fprintf(stderr, "sfdf: No input paths\n");
    fprintf(stderr, "rt-u sfdf path1 [, path2] [,...]\n");
    return ERR_SFDF_NO_ARG;
  }
  argc --;
  argv ++;
  unsigned char* data = 0;
  off_t len = 0;
  unsigned file_count = 0;
  int err = rtfl_collect(&data, &len, &file_count, argc, argv);
  if(err)
    return err;
  err = sfdf_process(file_count, data, (unsigned)argc);
  munmap(data, len);
  return err;
}

//******************************************************************************
//rtfl_name_cmp
static int rtfl_name_cmp(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  return strcmp((*p1)->name + (*p1)->rel_path_offset, (*p2)->name + (*p2)->rel_path_offset);
}

//******************************************************************************
//sffd_sort
static void sffd_sort(unsigned file_count, struct FS_FILE_ENTRY** idx_arr, unsigned root_count, unsigned* by_root_counts, struct FS_FILE_ENTRY*** idx_by_root)
{
  int i;
  unsigned curr_idx = (unsigned)-1;
  fprintf(stderr, "Grouping %d files by %d roots...\n", file_count, root_count);
  for(i = 0; i < root_count; i ++)
  {
    idx_by_root[i] = 0;
    by_root_counts[i] = 0;
  }
  for(i = 0; i < file_count; i ++)
  {
    if(curr_idx != idx_arr[i]->root_idx)
    {
      curr_idx = idx_arr[i]->root_idx;
      idx_by_root[curr_idx] = &idx_arr[i];
    }
    by_root_counts[curr_idx] ++;
  }
  fprintf(stderr, "Sorting each group content by file path...\n");
  for(i = 0; i < root_count; i ++)
    qsort(idx_by_root[i], by_root_counts[i], sizeof(void*), (int (*)(const void*, const void*))rtfl_name_cmp);
}

//******************************************************************************
//sffd_output
static void sffd_output(unsigned file_count, struct FS_FILE_ENTRY** idx_arr, unsigned root_count, char* roots[], unsigned* by_root_counts, struct FS_FILE_ENTRY*** idx_by_root)
{
  unsigned i, j, k, scount = 0, acount = 0, ccount = 0;
  fprintf(stderr, "Calculating hashes for equal size files...\n");
  printf("sffd");
  g_status.count = 0;
  for(j = 0; j < root_count; j ++)
  {
    for(k = 0; k < by_root_counts[j]; k ++)
    {
      int loop = 1;
      struct FS_FILE_ENTRY* curr = idx_by_root[j][k];
      for(i = 0; i < root_count; i ++)
      {
        if(i == j)
          continue;
        struct RTFL_OUTPUT_RECORD out_rec0 = {'|', curr->rec_len - sizeof(struct FS_FILE_ENTRY)};
        struct FS_FILE_ENTRY** found = bsearch(&curr->name[curr->rel_path_offset], idx_by_root[i], by_root_counts[i], sizeof(void*), (int (*)(const void*, const void*))fs_rel_path_cmp);
        if(found)
        {
          struct RTFL_OUTPUT_RECORD out_rec1 = {'=', (*found)->rec_len - sizeof(struct FS_FILE_ENTRY)};
          if((*found)->size != curr->size)
          {
            ccount ++;
            scount ++;
            out_rec1.status = (*found)->size < curr->size ? '>' : '<';
            if(loop)
            {
              fwrite(&out_rec0, 1, sizeof(out_rec0), stdout);
              fwrite(curr->name, 1, out_rec0.path_len, stdout);
              loop = 0;
            }
            fwrite(&out_rec1, 1, sizeof(out_rec1), stdout);
            fwrite((*found)->name, 1, out_rec1.path_len, stdout);
          }
          else
          {
            if(!curr->flg_hashed)
              sha256_file(curr, 0);
            if(!(*found)->flg_hashed)
              sha256_file(*found, 0);
            if(fs_sha256_cmp(&curr, found))
            {
              ccount ++;
              if(loop)
              {
                fwrite(&out_rec0, 1, sizeof(out_rec0), stdout);
                fwrite(curr->name, 1, out_rec0.path_len, stdout);
                loop = 0;
              }
              fwrite(&out_rec1, 1, sizeof(out_rec1), stdout);
              fwrite((*found)->name, 1, out_rec1.path_len, stdout);
            }
          }
        }
        else
        {
          acount ++;
          if(loop)
          {
            fwrite(&out_rec0, 1, sizeof(out_rec0), stdout);
            fwrite(curr->name, 1, out_rec0.path_len, stdout);
            loop = 0;
          }
          out_rec0.path_len = strlen(roots[i]) + strlen(&curr->name[curr->rel_path_offset]) + 2;
          out_rec0.status = '-';
          fwrite(&out_rec0, 1, sizeof(out_rec0), stdout);
          printf("%s/%s", roots[i], &curr->name[curr->rel_path_offset]);
          putchar(0);
        }
      }
    }
  }
  fprintf(stderr, "Found %d different size, %d different content, %d absent of %d total files cross scanning.\n", scount, ccount, acount, file_count);
}

//******************************************************************************
//sffd_process
static int sffd_process(unsigned file_count, unsigned char* data, unsigned root_count)
{
  struct FS_FILE_ENTRY* idx_arr[file_count];
  struct FS_FILE_ENTRY** idx_by_root[root_count];
  unsigned by_root_counts[root_count];
  char* roots[root_count];
  rtfl_idx(file_count, data, idx_arr, roots);
  sffd_sort(file_count, idx_arr, root_count, by_root_counts, idx_by_root);
  sffd_output(file_count, idx_arr, root_count, roots, by_root_counts, idx_by_root);
  return ERR_OK;
}

//******************************************************************************
//sffd_main
static int sffd_main(int argc, char* argv[])
{
  if(argc < 3)
  {
    fprintf(stderr, "sffd: Few input paths.\n");
    fprintf(stderr, "rt-u sffd path1 path2 [path3 ...]\n");
    return ERR_SFFD_COUNT_ARGS;
  }
  argc --;
  argv ++;
  unsigned char* data = 0;
  off_t len = 0;
  unsigned file_count = 0;
  int err = rtfl_collect(&data, &len, &file_count, argc, argv);
  if(err)
    return err;
  err = sffd_process(file_count, data, (unsigned)argc);
  munmap(data, len);
  return err;
}

//******************************************************************************
//rt_path_escape
#define IS_TO_ESCAPE(p_) (*p == '\\' || *p == '&' || *p == '$' || *p == '\'' || *p == '\"' || *p == '`')

static void rt_path_escape(char* buff)
{
  char* p = buff;
  int shift = 0;
  for(; *p; p ++) shift += IS_TO_ESCAPE(*p);
  for(; p >= buff; p --)
  {
    *(p + shift) = *p;
    if(IS_TO_ESCAPE(*p)) *(p + --shift) = '\\';
  }
}

//******************************************************************************
//bin2txt_main_re
static int bin2txt_main_re(char* pattern)
{
  struct RTFL_OUTPUT_RECORD fe;
  regex_t re;
  char buff[BUFF_SIZE];
  int res;
  if((res = regcomp(&re, pattern, REG_EXTENDED | REG_NEWLINE | REG_NOSUB)))
  {
    regerror(res, &re, buff, BUFF_SIZE);
    fputs(buff, stderr);
    return ERR_BIN2TXT_REGEX;
  }
  res = 0;
  while(1 == fread(&fe, sizeof(fe), 1, stdin) && 1 == fread(buff, fe.path_len, 1, stdin) && !ferror(stdin))
  {
    if(!regexec(&re, buff, 0, 0, 0))
    {
      res ++;
      rt_path_escape(buff);
      putchar(fe.status);
      puts(buff);
    }
  }
  puts("");
  regfree(&re);
  if(ferror(stdin))
  {
    perror(0);
    return ERR_CORRUPTED_INPUT;
  }
  fprintf(stderr, "bin2txt: %d matches found.\n", res);
  return ERR_OK;
}

//******************************************************************************
//bin2txt_main
#define CHECK_SIGN(_s, s_) (_s[0]==s_[0]&&_s[1]==s_[1]&&_s[2]==s_[2]&&_s[3]==s_[3])

static int bin2txt_main(int argc, char* argv[])
{
  char sign[4];
  struct RTFL_OUTPUT_RECORD fe;
  char buff[BUFF_SIZE];
  if(argc != 1 && argc != 2)
  {
    fprintf(stderr, "bin2txt: Improper number of arguments.\n");
    fprintf(stderr, "rt-u bin2txt < binary_file [> text_file]\n");
    fprintf(stderr, "rt-u bin2txt *regexp* < binary_file [> text_file]\n");
    return ERR_BIN2TXT_COUNT_ARGS;
  }
  if(1 != fread(sign, 4, 1, stdin)
    || (!CHECK_SIGN(sign, "sfdf") && !CHECK_SIGN(sign, "sffd") && !CHECK_SIGN(sign, "sfcl")))
  {
    fprintf(stderr, "Unsupported input data type. Only data generated by rt-u can be used as input.\n");
    fprintf(stderr, "Supported data generated by: sfdf, sffd, sfcl\n");
    return ERR_BIN2TXT_UNKNOWN_DATA;
  }
  if(argc == 2)
    return bin2txt_main_re(*++argv);
  while(1 == fread(&fe, sizeof(fe), 1, stdin) && 1 == fread(buff, fe.path_len, 1, stdin) && !ferror(stdin))
  {
    rt_path_escape(buff);
    putchar(fe.status);
    puts(buff);
  }
  puts("");

  if(ferror(stdin))
  {
    perror(0);
    return ERR_CORRUPTED_INPUT;
  }
  return ERR_OK;
}

//******************************************************************************
//sfcl_output
static void sfcl_output(unsigned file_count, struct FS_FILE_ENTRY** idx_arr)
{
  printf("sfcl");
  if(!file_count)
    return;
  unsigned count = 0;
  while(file_count --)
  {
    struct RTFL_OUTPUT_RECORD fe = {0};
    fe.status = '+';
    fe.path_len = (*idx_arr)->rec_len - sizeof(struct FS_FILE_ENTRY);
    fwrite(&fe, sizeof(fe), 1, stdout);
    fwrite((*idx_arr)->name, fe.path_len, 1, stdout);
    idx_arr ++;
    count ++;
  }
  fprintf(stderr, "%d files total.\n", count);
}

//******************************************************************************
//sfcl_process
static int sfcl_process(unsigned file_count, unsigned char* data, unsigned root_count)
{
  struct FS_FILE_ENTRY* idx_arr[file_count];
  char* roots[root_count];
  rtfl_idx(file_count, data, idx_arr, roots);
  sfcl_output(file_count, idx_arr);
  return ERR_OK;
}

//******************************************************************************
//sfcl_main
static int sfcl_main(int argc, char* argv[])
{
  if(argc < 2)
  {
    fprintf(stderr, "sfcl: No input paths\n");
    fprintf(stderr, "rt-u sfcl path1 [, path2] [,...]\n");
    return ERR_SFCL_NO_ARG;
  }
  argc --;
  argv ++;
  unsigned char* data = 0;
  off_t len = 0;
  unsigned file_count = 0;
  int err = rtfl_collect(&data, &len, &file_count, argc, argv);
  if(err)
    return err;
  err = sfcl_process(file_count, data, (unsigned)argc);
  munmap(data, len);
  return err;
}

//******************************************************************************
//gds_get_record_class
struct GDS_RECORD_CLASS
{
  const char* name;
  const char* descr;
  const unsigned char dtype;
  const char* dname;
  int (*dump)(const struct GDS_RECORD_CLASS*, unsigned char*, unsigned short, int*);
  short lmin;
  short lmax;
  const unsigned char flg;
};

enum
{
  GDS_NONE,
  GDS_PUSH,
  GDS_POP
};

static int gds_ignore(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  return ERR_OK;
}

static int gds_no_data_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s //%s\n", rc->name, rc->dname, rc->descr);
  return ERR_OK;
}

static int gds_bit_array_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  int i;
  unsigned short sh = (gds[0] << 8) + gds[1];
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, {", rc->name, rc->dname);
  for(i = 0; i < 15; i ++)
  {
    printf("%d, ", ((sh >> i) & 1));
  }
  printf("%d} //%s\n", (sh >> 15) & 1, rc->descr);
  return ERR_OK;
}

static int gds_short_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, %d //%s\n", rc->name, rc->dname, rt_byte_array_to_short(gds), rc->descr);
  return ERR_OK;
}

static int gds_int_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, %d //%s\n", rc->name, rc->dname, rt_byte_array_to_int(gds), rc->descr);
  return ERR_OK;
}

static int gds_double_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, %g //%s\n", rc->name, rc->dname, rt_byte_array_to_double(gds), rc->descr);
  return ERR_OK;
}

static int gds_string_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, \"%s\" //%s\n", rc->name, rc->dname, gds, rc->descr);
  return ERR_OK;
}

static int gds_mod_access_info_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  short td[12];
  int i;
  for(i = 0; i < sizeof(td) / sizeof(td[0]); i ++)
  {
    td[i] = rt_byte_array_to_short(gds);
    gds += 2;
  }
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, {%d/%02d/%02d - %02d:%02d:%02d, %d/%02d/%02d - %02d:%02d:%02d} //%s\n", rc->name, rc->dname,
    1900 + td[0], td[1], td[2], td[3], td[4], td[5], 1900 + td[6], td[7], td[8], td[9], td[10], td[11], rc->descr);
  return ERR_OK;
}

static int gds_units_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, {%g, %g} //%s\n", rc->name, rc->dname, rt_byte_array_to_double(&gds[0]), rt_byte_array_to_double(&gds[8]), rc->descr);
  return ERR_OK;
}

static int gds_xy_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  if(len % 8)
  {
    fprintf(stderr, "ERROR: corrupted GDS file - invalid record length for %s (%d bytes).", rc->name, len);
    return ERR_GDS_DLENGTH;
  }
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, ", rc->name, rc->dname);
  while(len)
  {
    printf("{%d, %d} ", rt_byte_array_to_int(gds), rt_byte_array_to_int(gds + 4));
    len -= 8;
    gds += 8;
  }
  printf("//%s\n", rc->descr);
  return ERR_OK;
  
}

static int gds_fonts_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, {\"%s\", \"%s\", \"%s\", \"%s\"} //%s\n", rc->name, rc->dname, gds, gds + 44, gds + 88, gds + 132, rc->descr);
  return ERR_OK;
}

static int gds_col_row_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, %d %d //%s\n", rc->name, rc->dname, rt_byte_array_to_short(gds), rt_byte_array_to_short(gds + 2), rc->descr);
  return ERR_OK;
}

static int gds_ref_libs_to_text(const struct GDS_RECORD_CLASS* rc, unsigned char* gds, unsigned short len, int* indent)
{
  rt_stdout_indent(*indent);
  printf("R:%s, D:%s, {\"%s\", \"%s\"} //%s\n", rc->name, rc->dname, gds, gds + 44, rc->descr);
  return ERR_OK;
}


static const struct GDS_RECORD_CLASS* gds_get_record_class(unsigned char id)
{
  static const struct GDS_RECORD_CLASS clsArr[] =
  {
    {"HEADER", "Start of stream, contains version number of stream file", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x00
    {"BGNLIB", "Beginning of library, plus mod and access dates", 0x02, "INTEGER_2", gds_mod_access_info_to_text, 24, 24, GDS_PUSH}, //0x01
    {"LIBNAME", "The name of the library", 0x06, "STRING", gds_string_to_text, 1, 512}, //0x02
    {"UNITS", "Size of db unit in user units and size of db unit in meters", 0x05, "REAL_8", gds_units_to_text, 16, 16}, //0x03
    {"ENDLIB", "End of the library", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_POP}, //0x04
    {"BGNSTR", "Begin structure, plus create and mod dates", 0x02, "INTEGER_2", gds_mod_access_info_to_text, 24, 24, GDS_PUSH}, //0x05
    {"STRNAME", "Name of a structure", 0x06, "STRING", gds_string_to_text, 1, 0x100}, //0x06 !!! lmax 32 in spec
    {"ENDSTR", "End of a structure", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_POP}, //0x07
    {"BOUNDARY", "The beginning of a BOUNDARY element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x08
    {"PATH", "The beginning of a PATH element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x09
    {"SREF", "The beginning of an SREF element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x0a
    {"AREF", "The beginning of an AREF element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x0b
    {"TEXT", "The beginning of a TEXT element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x0c
    {"LAYER", "Layer specification", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x0d
    {"DATATYPE", "Datatype specification", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x0e
    {"WIDTH", "Width specification, negative means absolute", 0x03, "INTEGER_4", gds_int_to_text, 4, 4}, //0x0f
    {"XY", "An array of XY coordinates", 0x03, "INTEGER_4", gds_xy_to_text, 8, 800}, //0x10
    {"ENDEL", "The end of an element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_POP}, //0x11
    {"SNAME", "The name of a referenced structure", 0x06, "STRING", gds_string_to_text, 1, 512}, //0x12
    {"COLROW", "Columns and rows for an AREF", 0x02, "INTEGER_2", gds_col_row_to_text, 4, 4}, //0x13
    {"TEXTNODE", "\"Not currently used\" per GDSII Stream Format Manual, Release 6.0", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x14
    {"NODE", "The beginning of a NODE element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x15
    {"TEXTTYPE", "Texttype specification", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x16
    {"PRESENTATION", "Text origin and font specification", 0x01, "BIT_ARRAY", gds_bit_array_to_text, 2, 2}, //0x17
    {"SPACING", "\"Discontinued\" per GDSII Stream Format Manual, Release 6.0", 0xff, "UNKNOWN", gds_ignore, 0, 512}, //0x18
    {"STRING", "Character string", 0x06, "STRING", gds_string_to_text, 1, 512}, //0x19
    {"STRANS", "Refl, absmag, and absangle for SREF, AREF and TEXT", 0x01, "BIT_ARRAY", gds_bit_array_to_text, 2, 2}, //0x1a
    {"MAG", "Magnification, 1 is the default", 0x05, "REAL_8", gds_double_to_text, 8, 8}, //0x1b
    {"ANGLE", "Angular rotation factor", 0x05, "REAL_8", gds_double_to_text, 8, 8}, //0x1c
    {"UINTEGER", "User integer, used only in V2.0, translates to userprop 126 on instream", 0xff, "UNKNOWN", gds_ignore, 0, 512}, //0x1d
    {"USTRING", "User string, used only in V2.0, translates to userprop 127 on instream", 0xff, "UNKNOWN", gds_ignore, 0, 512}, //0x1e
    {"REFLIBS", "Names of the reference libraries", 0x06, "STRING", gds_ref_libs_to_text, 2*44, 2*44}, //0x1f
    {"FONTS", "Names of the textfont definition files", 0x06, "STRING", gds_fonts_to_text, 4*44, 4*44}, //0x20
    {"PATHTYPE", "Type of path ends", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x21
    {"GENERATIONS", "Number of deleted or backed up structures to retain", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x22
    {"ATTRTABLE", "Name of the attribute definition file", 0x06, "STRING", gds_string_to_text, 1, 44}, //0x23
    {"STYPTABLE", "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 0x06, "STRING", gds_string_to_text, 1, 512}, //0x24
    {"STRTYPE", "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x25
    {"ELFLAGS", "Flags for template and exterior data", 0x01, "BIT_ARRAY", gds_bit_array_to_text, 2, 2}, //0x26
    {"ELKEY", "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 0x03, "INTEGER_4", gds_int_to_text, 4, 4}, //0x27
    {"LINKTYPE", "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 0xff, "UNKNOWN", gds_ignore, 0, 512}, //0x28
    {"LINKKEYS", "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 0xff, "UNKNOWN", gds_ignore, 0, 512}, //0x29
    {"NODETYPE", "Nodetype specification", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x2a
    {"PROPATTR", "Property number", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x2b
    {"PROPVALUE", "Property value", 0x06, "STRING", gds_string_to_text, 1, 512}, //0x2c
    {"BOX", "The beginning of a BOX element", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_PUSH}, //0x2d
    {"BOXTYPE", "Boxtype specification", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x2e
    {"PLEX", "Plex number and plexhead flag", 0x03, "INTEGER_4", gds_int_to_text, 4, 4}, //0x2f
    {"BGNEXTN", "Path extension beginning for pathtype 4 in CustomPlus", 0x03, "INTEGER_4", gds_int_to_text, 4, 4, GDS_PUSH}, //0x30
    {"ENDTEXTN", "Path extension end for pathtype 4 in CustomPlus", 0x03, "INTEGER_4", gds_int_to_text, 4, 4, GDS_POP}, //0x31
    {"TAPENUM", "Tape number for multi-reel stream file, you've got a really old file here", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x32
    {"TAPECODE", "Tape code to verify that you've loaded a reel from the proper set", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x33
    {"STRCLASS", "Calma use only, non-Calma programs should not use, or set to all 0", 0x01, "BIT_ARRAY", gds_bit_array_to_text, 2, 2}, //0x34
    {"RESERVED", "Used to be NUMTYPES per GDSII Stream Format Manual, Release 6.0", 0x03, "INTEGER_4", gds_int_to_text, 4, 4}, //0x35
    {"FORMAT", "Archive or Filtered flag", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x36
    {"MASK", "Only in filtered streams, lists layer and datatype mask used", 0x06, "STRING", gds_string_to_text, 1, 512, GDS_PUSH}, //0x37
    {"ENDMASKS", "The end of mask descriptions", 0x00, "NO_DATA", gds_no_data_to_text, 0, 0, GDS_POP}, //0x38
    {"LIBDIRSIZE", "Number of pages in library director, a GDSII thing...", 0x02, "INTEGER_2", gds_short_to_text, 2, 2}, //0x38
    {"SRFNAME", "Sticks rule file name", 0x06, "STRING", gds_string_to_text, 0, 512}, //0x39
    {"LIBSECUR", "Access control list stuff for CalmaDOS, ancient!", 0x02, "INTEGER_2", gds_ignore, 2, 2}, //0x3a
  };
  return &clsArr[id];
}

//******************************************************************************
//gds_dump_report
static void gds_scan_report()
{
  fprintf(stderr, "--- scanning GDS --- %d records scanned --- %llu bytes remain\n", g_status.count, g_status.byte);
}

//******************************************************************************
//gds_dump_txt
static int gds_dump_txt()
{
  int res = ERR_OK;
  unsigned short len;
  unsigned short dlen;
  int indent = 0;
  struct stat st = {0};
  fstat(0, &st);
  if(!st.st_size)
  {
    fprintf(stderr, "ERROR: empty GDS file or not redirected from file\n");
    return ERR_EMPTY_INPUT;
  }
  unsigned char* gds = (unsigned char*)mmap(0, st.st_size, PROT_READ, MAP_SHARED, 0, 0);
  if(MAP_FAILED == gds)
  {
    perror(0);
    return ERR_MAPPING;
  }
  fprintf(stderr, "The input GDS file size is %llu bytes, scanning...\n", st.st_size);
  g_status.byte = st.st_size;
  g_status.count = 0;
  g_status.report = gds_scan_report;
  for(; res == ERR_OK && (len = (gds[0] << 8) + gds[1]) && g_status.byte > 0; gds += len, g_status.byte -= len)
  {
    const struct GDS_RECORD_CLASS* rc = gds_get_record_class(gds[2]);
    if(gds[3] != rc->dtype)
    {
      fprintf(stderr, "ERROR: illegal data type(%02x) for %s type record. Must be %02x\n", gds[3], rc->name, rc->dtype);
      return ERR_GDS_DTYPE;
    }
    dlen = len - 4;
    if(dlen < rc->lmin || dlen > rc->lmax)
    {
      fprintf(stderr, "ERROR: corrupted GDS file - invalid record length for %s (%d bytes).", rc->name, dlen);
      res = ERR_GDS_DLENGTH;
      break;
    }
    indent -= (rc->flg == GDS_POP);
    res = rc->dump(rc, gds + 4, dlen, &indent);
    indent += (rc->flg == GDS_PUSH);
    g_status.count ++;
  }
  munmap(gds, st.st_size);
  g_status.report = rt_empty_report;
  return res;
}

//******************************************************************************
//gds_main
static int gds_main(int argc, char* argv[])
{
  static const char* gds_usage = "rt-u gds dump text < gds_file [> text_file]";
  if(argc != 3)
  {
    fprintf(stderr, "gds: improper number of arguments.\n%s\n", gds_usage);
    return ERR_GDS_COUNT_ARGS;
  }
  argc --;
  argv ++;
  if(!strcmp("dump", *argv))
  {
    argc --;
    argv ++;
    if(!strcmp("text", *argv))
      return gds_dump_txt();
    fprintf(stderr, "gds dump: unsupported output type: %s\n%s\n", *argv, gds_usage);
    return ERR_GDS_DUMP_UNKNOWN_OUTPUTTYPE;
  }
  fprintf(stderr, "gds: unknown subfunction: %s\n%s\n", *argv, gds_usage);
  return ERR_GDS_UNKNOWN_SUBFUNCTION;
}

//******************************************************************************
//http_process
static int http_process()
{
  char buff[0x10000];
  while(fgets(buff, sizeof(buff), stdin) && *buff != '\r')
  {
    fputs(buff, stderr);
    fflush(stderr);
  }
  return ERR_OK;
}

//******************************************************************************
//http_init_log
static void http_init_log()
{
  char lfname[32];
  sprintf(lfname, ".logs/%d", getpid());
  int lf = open(lfname, O_CREAT | O_WRONLY | O_APPEND, 0644);
  if(lf == -1)
  {
    perror(lfname);
    return;
  }
  dup2(lf, STDERR_FILENO);
  close(lf);
}

//******************************************************************************
//http_wait_client
#define HTTP_MAX_CONN   128
static int http_wait_client(int ss)
{
  struct sockaddr_in a;
  int pid = 0;
  socklen_t sz = sizeof(struct sockaddr_in);
  fprintf(stderr, "%s waiting for client\n", rt_adate());
  int s = accept(ss, (struct sockaddr *)&a, &sz);
  if(s < 0)
  {
    perror(rt_adate());
    return ERR_HTTP_ACCEPT;
  }
  if(HTTP_MAX_CONN == g_glb.http->conn_count)
  {
    fprintf(stderr, "%s conn_count=HTTP_MAX_CONN (%d). Rejecting.\n", rt_adate(), HTTP_MAX_CONN);
    close(s);
    return ERR_OK;
  }
  if((pid = fork()))
  {
    fprintf(stderr, "%s accept %d %d %s:%d %d\n", rt_adate(), pid, s, inet_ntoa(a.sin_addr), ntohs(a.sin_port), ++g_glb.http->conn_count);
    close(s);
    return ERR_OK;
  }
  close(ss);
  dup2(s, STDIN_FILENO);
  dup2(s, STDOUT_FILENO);
  close(s);
  http_init_log();
  exit(http_process());
}

//******************************************************************************
//http_init_server
#define HTTP_LISTEN_COUNT 5

static int http_init_server(char* home, int port, int* ss)
{
  struct sockaddr_in sin;
  int on = 1;
  if(port < 1 || port > 65535)
  {
    fprintf(stderr, "Invalid port number. Must be any number between 1 and 65535.\n");
    return ERR_HTTP_INVALID_PORT;
  }
  if(chdir(home))
  {
    perror(home);
    return ERR_HTTP_INVALID_HOME_DIR;
  }
  mkdir(".logs", 0744);
  *ss = socket(AF_INET, SOCK_STREAM, 0);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  setsockopt(*ss, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  if(bind(*ss, (struct sockaddr*) &sin, sizeof(sin)) < 0 || listen(*ss, HTTP_LISTEN_COUNT) < 0)
  {
    close(*ss);
    perror("http server initialization");
    return ERR_HTTP_BIND_LISTEN;
  }
  fprintf(stderr, "%s Starting listen on port %d\n", rt_adate(), port);
  return ERR_OK;
}

//******************************************************************************
//http_sig_child
static void http_sig_child(int sig)
{
  fprintf(stderr, "%s close %d\n", rt_adate(), --g_glb.http->conn_count);
  signal(SIGCHLD, http_sig_child);
}

//******************************************************************************
//http_main
static int http_main(int argc, char* argv[])
{
  struct HTTP_GLOBALS glb = {0};
  int res = ERR_OK;
  int ss = -1;
  static const char* http_usage = "rt-u http home port";
  if(argc != 3)
  {
    fprintf(stderr, "Improper number of arguments.\n%s\n", http_usage);
    return ERR_HTTP_COUNT_ARGS;
  }
  argc --;
  argv ++;
  if((res = http_init_server(argv[0], atoi(argv[1]), &ss)))
    return res;
  g_glb.http = &glb;
  signal(SIGCHLD, http_sig_child);
  while(!(res = http_wait_client(ss)));
  signal(SIGCHLD, SIG_IGN);
  close(ss);
  return res;
}

  //little endian read unsigned short
static unsigned read_u16_le()
{
    return (unsigned)fgetc(stdin) | (unsigned)fgetc(stdin) << 8;
}
//big endian read unsigned short
static unsigned read_u16_be()
{
    return (unsigned)fgetc(stdin) << 8 | (unsigned)fgetc(stdin);
}
//little endian read unsigned int
static unsigned read_u32_le(char* data)
{
    return (unsigned)fgetc(stdin) | (unsigned)fgetc(stdin) << 8 |
        (unsigned)fgetc(stdin) << 16 | (unsigned)fgetc(stdin) << 24;
}
//big endian read unsigned int
static unsigned read_u32_be(char* data)
{
    return (unsigned)fgetc(stdin) << 24 | (unsigned)fgetc(stdin) << 16 |
        (unsigned)fgetc(stdin) << 8 | (unsigned)fgetc(stdin);
}

//******************************************************************************
//jpeg_main
static int jpeg_main(int argc, char* argv[])
{
  //endian-dependent read for unsigned short and int
  unsigned (*read_u16)();
  unsigned (*read_u32)();
  unsigned i, j, a;
  //JPEG always begins with 0xFF, 0xD8
  if(0xff != fgetc(stdin) || 0xd8 != fgetc(stdin))
  {
    fputs("Invalid JPEG input stream - must begin with 0xff, 0xd8.\n", stderr);
    return ERR_JPEG_INVALID_STREAM;
  }
  //up to 19 sections in JPEG
  for(i = 0; i < 19; i ++)
  {
    //maximum 7 padding bytes in row
    for(j = 0; j < 8 && 0xFF == (a = fgetc(stdin)); j ++);
    if(a == 0xff)
    {
      fputs("Too many padding bytes in input stream.\n", stderr);
      return ERR_JPEG_INVALID_STREAM;
    }
    //setion length is always big endian
    int sec_len = fgetc(stdin) << 8 | fgetc(stdin);
    if(sec_len < 2)
    {
      fputs("Invalid section length.\n", stderr);
      return ERR_JPEG_INVALID_STREAM;
    }
    //pass section if not EXIF
    if(a != 0xe1)
    {
      char data[sec_len - 2];
      fread(data, 1, sizeof(data), stdin);
      continue;
    }
    //EXIF header: 0x45, 0x78, 0x69, 0x66, 0x00, 0x00
    if(fgetc(stdin) != 0x45 || fgetc(stdin) != 0x78 || fgetc(stdin) != 0x69 || fgetc(stdin) != 0x66 || fgetc(stdin) != 0x00 || fgetc(stdin) != 0x00)
    {
      fputs("Invalid EXIF header.\n", stderr);
      return ERR_JPEG_EXIF_HEADER;
    }
    if('M' == fgetc(stdin) && 'M' == fgetc(stdin))
    {
      read_u16 = read_u16_be;
      read_u32 = read_u32_be;
    }
    else
    {
      read_u16 = read_u16_le;
      read_u32 = read_u32_le;
    }
    //EXIF data start: 0x2a, 0x08
    if(0x2a != read_u16() || 0x08 != read_u32())
    {
      fputs("Invalid EXIF data start.\n", stderr);
      return ERR_JPEG_EXIF;
    }
    //number of EXIF directories
    unsigned num_dirs = read_u16();
    //search GPS data directory
    for(j = 0; j < num_dirs; j ++)
    {
      unsigned tag = read_u16();
      unsigned format = read_u16();
      if(format > 12)
      {
        fputs("Illegal format code in EXIF dir.\n", stderr);
        return ERR_JPEG_EXIF;
      }
      //unsigned comp = read_u32();
      //if tag is not GPSINFO - go ahead
      if(tag != 0x8825) continue;
      //static const unsigned bytes_per_format[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};
      //unsigned byte_count = comp * bytes_per_format[format];
      //if byte count is more than 4 the value is value offset, else it is value itself
      char data[48];
      fread(data, 1, sizeof(data) - 1, stdin);
      data[sizeof(data) - 1] = 0;
      fputs(data, stderr);
      
    }
    return ERR_OK;
  }
  return ERR_JPEG_NO_EXIF;
}

//******************************************************************************
//rt_sig_usr1
static void rt_sig_usr1(int sig)
{
  g_status.report();
  signal(SIGUSR1, rt_sig_usr1);
}

//==============================================================================
//******************************************************************************
//show_usage
static void show_usage()
{
  fprintf(stderr, "Usage:\n\trt-u sfdf path1 [, path2] [,...]\n");
  fprintf(stderr, "\trt-u sffd path1 path2 [, path3] [,...]\n");
  fprintf(stderr, "\trt-u sfcl path1 [, path2] [,...]\n");
  fprintf(stderr, "\trt-u bin2txt < binary_file [> text_file]\n");
  fprintf(stderr, "\trt-u bin2txt *regexp* < binary_file [> text_file]\n");
  fprintf(stderr, "\trt-u gds dump text < gds_file [> text_file]\n");
  fprintf(stderr, "\trt-u http home port\n");
  fprintf(stderr, "\tjpeg-gps < JPEG file [> coordinates text file]\n");
  fprintf(stderr, "Functions:\n\tsfdf = Scan For Duplicate Files\n");
  fprintf(stderr, "\tsffd = Scan For Different Files\n");
  fprintf(stderr, "\tsfcl = Collect File Information\n");
  fprintf(stderr, "\tbin2txt = Convert binary (generated) file to text\n");
  fprintf(stderr, "\tgds = GDS file operations\n");
  fprintf(stderr, "\thttp = HTTP server\n");
  fprintf(stderr, "\tjpeg-gps = extract EXIF GPS coordinate information from JPEG file\n");
}

//******************************************************************************
//main
int main(int argc, char* argv[])
{
  signal(SIGUSR1, rt_sig_usr1);
  if(argc < 2)
  {
    fprintf(stderr, "No function specified\n");
    show_usage();
    return ERR_NO_FUNC;
  }
  argc --;
  argv ++;
  if(!strcmp("sfcl", *argv))
    return sfcl_main(argc, argv);
  if(!strcmp("sfdf", *argv))
    return sfdf_main(argc, argv);
  if(!strcmp("sffd", *argv))
    return sffd_main(argc, argv);
  if(!strcmp("bin2txt", *argv))
    return bin2txt_main(argc, argv);
  if(!strcmp("gds", *argv))
    return gds_main(argc, argv);
  if(!strcmp("http", *argv))
    return http_main(argc, argv);
  if(!strcmp("jpeg-gps", *argv))
    return jpeg_main(argc, argv);
  fprintf(stderr, "Unknown function: %s\n", *argv);
  show_usage();
  return ERR_UNKNOWN_FUNC;
}

