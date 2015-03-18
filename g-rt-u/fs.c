#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include "rt.h"


static gboolean FsIsPathIntersect(const char* path1, const char* path2)
{
#ifdef WIN32
  const char* p1 = path1;
  const char* p2 = path2;
  for(; *p1 && *p2 && *p1 == *p2; p1 ++, p2 ++);
  return (!*p1 && (!*p2 || *p2 == '\\')) || (!*p2 && (!*p1 || *p1 == '\\'));
#else
  const char* p1 = g_path_skip_root(path1);
  const char* p2 = g_path_skip_root(path2);
  if(!*p1 || !*p2)
    return TRUE;
  for(; *p1 && *p2 && *p1 == *p2; p1 ++, p2 ++);
  return (!*p1 && (!*p2 || *p2 == '/')) || (!*p2 && (!*p1 || *p1 == '/'));
#endif
}

struct FS_COLLECT_NODE
{
  struct FS_COLLECT_NODE* next;
  char* name;
  size_t name_len;
};

struct FS_COLLECT
{
  gboolean bcont;
  struct FS_COLLECT_NODE* top;
  int f;
  unsigned count;
};

struct FS_FILE_ENTRY
{
  off_t size;
  time_t time;
  union
  {
    unsigned char b[32];
    unsigned u[4];
  }sh;
  unsigned name_len;
  char name[];
};

#define FS_FEHSZ    (sizeof(off_t) + sizeof(time_t) + 32 + sizeof(unsigned))

static void FsDumpChain(struct FS_COLLECT* fc)
{
  struct stat st;
  struct FS_FILE_ENTRY fe = {0};
  struct FS_COLLECT_NODE* node = fc->top;
  struct FS_COLLECT_NODE* last = node;
  while(node)
  {
    last = node;
    fe.name_len += node->name_len;
    node = node->next;
  }
  stat(last->name, &st);
  fe.size = st.st_size;
  fe.time = st.st_mtime;
  if(FS_FEHSZ != write(fc->f, &fe, FS_FEHSZ))
  {
    fc->bcont = FALSE;
    return;
  }
  node = fc->top;
  while(node)
  {
    if(node->name_len != write(fc->f, node->name, node->name_len))
    {
      fc->bcont = FALSE;
      return;
    }
    node = node->next;
  }
  fc->count++;
}

static void FsCollectDirScanForDiffFiles(struct FS_COLLECT* fc, struct FS_COLLECT_NODE* parent)
{
  struct FS_COLLECT_NODE node = {0};
  DIR* dir = opendir(".");
  struct dirent* de = 0;
  if(!dir)
  {
    g_tv.AddLogLine(parent->name, "Cannot scan directory");
    return;
  }
  parent->next = &node;
  while(fc->bcont && (de = readdir(dir)))
  {
    if((de->d_name[0] == '.' && (de->d_name[1] == 0 || (de->d_name[1] == '.' && de->d_name[2] == 0))))
      continue;
    node.next = 0;
    node.name = de->d_name;
    node.name_len = strlen(de->d_name) + 1;
    if(de->d_type == DT_DIR)
    {
      if(!chdir(de->d_name))
      {
        node.name[node.name_len - 1] = '/';
        FsCollectDirScanForDiffFiles(fc, &node);
        chdir("..");
      }
      else
        g_tv.AddLogLine(de->d_name, "Cannot enter directory");
      continue;
    }
    FsDumpChain(fc);
  }
  closedir(dir);
  parent->next = 0;
}

static gboolean FsScanForDiffFilesProcDir(char* path, struct FS_COLLECT* fc)
{
  struct FS_COLLECT_NODE node = {0, path, strlen(path) + 1};
  fc->top = &node;
  if(!chdir(path))
  {
    FsCollectDirScanForDiffFiles(fc, &node);
    chdir("..");
  }
  else
    g_tv.AddLogLine(path, "Cannot enter directory");
  return fc->bcont;
}

static gboolean FsScanForDiffFilesProcFile(char* path, struct FS_COLLECT* fc)
{
  struct FS_COLLECT_NODE node = {0, path, strlen(path) + 1};
  fc->top = &node;
  FsDumpChain(fc);
  return fc->bcont;
}

static void FsCollectIdx(struct FS_FILE_ENTRY** pe, struct FS_FILE_ENTRY* data, unsigned count)
{
  while(count --)
  {
    unsigned char* p = (unsigned char*)data;
    *pe = data;
    p += (FS_FEHSZ + data->name_len);
    data = (struct FS_FILE_ENTRY*)p;
    pe ++;
  }
}

static void FsScanForDiffFiles()
{
}

static void FsDumpFromIdx(struct FS_FILE_ENTRY** pe, unsigned count)
{
  while(count --)
  {
    printf(">>%d. %s (s=%ld, t=%ld)\n", count, (*pe)->name, (*pe)->size, (*pe)->time);
    pe ++;
  }
}

static int fFsSortBySize(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  if((*p1)->size < (*p2)->size)
    return -1;
  if((*p1)->size > (*p2)->size)
    return 1;
  return 0;
}

static int fFsSortByTime(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  if((*p1)->time < (*p2)->time)
    return 0;
  if((*p1)->time > (*p2)->time)
    return 1;
  return 0;
}

static int tmp_test(void* ctx, long long count)
{
  return 1;
}

static int FsCollectSha256(struct FS_FILE_ENTRY** pe, unsigned fc)
{
  int count = 0;
  int loop = 1;
  qsort(pe, fc, sizeof(void*), (int (*)(const void*, const void*))fFsSortBySize);
  struct FS_FILE_ENTRY** p1 = pe;
  struct FS_FILE_ENTRY** p2 = pe;
  while(--fc)
  {
    p2 ++;
    if((*p1)->size == (*p2)->size)
    {
      if(loop)
      {
        loop = 0;
        count ++;
        g_sha256.FileSha256((*p1)->sh.b, (*p1)->name, tmp_test, 0);
      }
      count ++;
      g_sha256.FileSha256((*p2)->sh.b, (*p2)->name, tmp_test, 0);
    }
    else
      loop = 1;
    p1 = p2;
  }
  return count;
}

static int FsSha256Cmp(struct FS_FILE_ENTRY** p1, struct FS_FILE_ENTRY** p2)
{
  return memcmp((*p2)->sh.b, (*p1)->sh.b, 32);
}

int FsCollecSameFiles(struct FS_FILE_ENTRY** pe, unsigned fc, unsigned sc)
{
  int count = 0;
  int loop = 1;
  struct FS_FILE_ENTRY** p1 = pe;
  struct FS_FILE_ENTRY** p2 = pe;
  qsort(pe, fc, sizeof(void*), (int (*)(const void*, const void*))FsSha256Cmp);
  while(--sc)
  {
    p2 ++;
    if(!FsSha256Cmp(p1, p2))
    {
      if(loop)
      {
        loop = 0;
        count ++;
        printf("==%d-%ld, %s\n", count, (*p1)->size, (*p1)->name);
      }
      count ++;
      printf("==%d-%ld, %s\n", count, (*p2)->size, (*p2)->name);
    }
    else
      loop = 1;
    p1 = p2;
  }
  return count;
}

static unsigned FsCollectFileList(struct FS_FILE_ENTRY** fe, off_t* size)
{
  struct stat st;
  struct FS_COLLECT fc = {TRUE, 0};
  FILE* f = tmpfile();
  if(!f)
  {
    g_tv.AddLogLine("ERROR.", "Cannot create temp file");
    return 0;
  }
  fc.f = fileno(f);
  g_tv.fForEachProc = (gboolean(*)(char*, gpointer))FsScanForDiffFilesProcDir;
  if(g_tv.ForEachValue("0:0:0:0", &fc))
  {
    g_tv.fForEachProc = (gboolean(*)(char*, gpointer))FsScanForDiffFilesProcFile;
    g_tv.ForEachValue("0:0:0:1", &fc);
  }
  fstat(fc.f, &st);
  if(!st.st_size || fc.count < 2)
  {
    fclose(f);
    g_tv.AddLogLine("At least 2 file are needed", "Invalid input");
    return 0;
  }
  *fe = (struct FS_FILE_ENTRY*)mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fc.f, 0);
  *size = st.st_size;
  fclose(f);
  return fc.count;
}

static void FsScanForDupFiles()
{
  unsigned fc;
  off_t size = 0;
  struct FS_FILE_ENTRY* data = 0;
  if((fc = FsCollectFileList(&data, &size)))
  {
    struct FS_FILE_ENTRY* pe[fc];
    FsCollectIdx(pe, data, fc);
//    FsDumpFromIdx(pe, fc);
    FsCollecSameFiles(pe, fc, FsCollectSha256(pe, fc));
//    FsDumpFromIdx(pe, fc);
//    FsDumpFromIdx(pe, fc);
    /*
    printf(">>COUNT: %d\n", fc);
    struct FS_FILE_ENTRY* idx_arr[fc];
    FsCollectIdx(data, idx_arr, fc);
    qsort(idx_arr, fc, sizeof(char*), (int (*)(const void*, const void*))fFsSortBySize);
    FsDumpFromIdx(idx_arr, fc);
    qsort(idx_arr, fc, sizeof(char*), (int (*)(const void*, const void*))fFsSortByTime);
    FsDumpFromIdx(idx_arr, fc);*/
    munmap(data, size);
  }
}

struct FS g_fs =
{
  .IsPathIntersect    = FsIsPathIntersect
  , .ScanForDiffFiles = FsScanForDiffFiles
  , .ScanForDupFiles  = FsScanForDupFiles
};

