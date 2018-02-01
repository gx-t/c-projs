#include "rt.h"

static struct SYS_HANDLE_TBL g_SysHndlTbl;
static struct SYS_PROC_TBL g_SysProcTbl;

static void SysLoadDlls()
{
  HANDLE hDll = g_SysHndlTbl.hKernel32Dll = GetModuleHandle(L"Kernel32.dll");
  g_SysProcTbl.CreateFileMapping = (HANDLE (CALLBACK *)(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCTSTR))GetProcAddress(hDll, "CreateFileMappingW");
  g_SysProcTbl.MapViewOfFile = (LPVOID (CALLBACK *)(HANDLE, DWORD, DWORD, DWORD, SIZE_T))GetProcAddress(hDll, "MapViewOfFile");
  g_SysProcTbl.UnmapViewOfFile = (BOOL (CALLBACK *)(LPCVOID))GetProcAddress(hDll, "UnmapViewOfFile");
}

static void SysUnloadDlls()
{
}

struct SYS_API g_sys =
{
  &g_SysHndlTbl
  , &g_SysProcTbl
  , SysLoadDlls
  , SysUnloadDlls
};
