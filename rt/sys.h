#ifndef __SYS_H__
#define __SYS_H__

struct SYS_HANDLE_TBL
{
  HANDLE hKernel32Dll;
};

struct SYS_PROC_TBL
{
  HANDLE (CALLBACK *CreateFileMapping)(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCTSTR);
  LPVOID (CALLBACK *MapViewOfFile)(HANDLE, DWORD, DWORD, DWORD, SIZE_T);
  BOOL (CALLBACK *UnmapViewOfFile)(LPCVOID);
};

struct SYS_API
{
  struct SYS_HANDLE_TBL* pSysHndlTbl;
  struct SYS_PROC_TBL* pSysProcTbl;
  void (*SysLoadDlls)();
  void (*SysUnloadDlls)();
};

extern struct SYS_API g_sys;

#endif //__SYS_H__
