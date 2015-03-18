#ifndef __FS_H__
#define __FS_H__

struct FS
{
  gboolean (*IsPathIntersect) (const char* path1, const char* path2);
  void (*ScanForDiffFiles)    ();
  void (*ScanForDupFiles)     ();
};
extern struct FS g_fs;

#endif //__FS_H__

