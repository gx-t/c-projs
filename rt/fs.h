#ifndef __FS_H__
#define __FS_H__

struct FS_GLB
{
  DWORD dwFileCount;
  WIN32_FIND_DATA fd;
};

struct FS_API
{
  //global data:
  struct FS_GLB* pGlb;
  //services:
  void   (*FsNormalizePath)(PWSTR szPath);
  PWSTR  (*FsSkipNetworkBacklashes)(PWSTR szPath);
  PCWSTR (*FsGetExtension)(PCWSTR szPath);
  PWSTR  (*FsSelectDirAndFile)(HWND, PCWSTR, PWSTR);
  int    (*FsCopy)(HWND, PCWSTR szDest, PCWSTR szSrc);
  
  void   (*FsInitScan)();
  void   (*FsCollectFileNames)(PWSTR szScanRoot);
  void   (*FsSfdf)();
  void   (*FsSffd)(PWSTR szScanRoot0, PWSTR szScanRoot1);
  void   (*FsDelTempFiles)();
  
  //callbacks
  void (*fAddLogLine)(PCWSTR, ...);
  UINT (*fMsgBoxAndLog)(PCWSTR, PCWSTR, UINT);
  void (*fAddSfdfGroup)(PCWSTR);
  void (*fAddSfdfItem)(PCWSTR);
  void (*fAddSffdNewer)(PCWSTR szPath, PWSTR szRelPath);
  void (*fAddSffdBigger)(PCWSTR szPath, PWSTR szRelPath);
  void (*fAddSffdAbsent)(PCWSTR szPath, PWSTR szRelPath);
  void (*fSortLastGroup)();
  void (*fProgressTitle)();
  int  (*fCheckCancel)();
};

extern struct FS_API g_fs;

#endif //__FS_H__
