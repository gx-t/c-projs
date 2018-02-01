#ifndef __RT_H__
#define __RT_H__
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <malloc.h>

#define TXT_BUFF_SIZE           4096

#include "sys.h"
#include "tv.h"
#include "fs.h"
#include "mnu.h"
#include "str.h"


struct RT_GLB
{
  MSG msg;
};

struct RT_API
{
  struct RT_GLB* pGlb;
  int (*RtMain)();
  void (*RtPassMsg)();
  void (*RtMessageLoop)();
  void (*RtExitProcess)();
};

extern struct RT_API g_rt;

#ifdef __DLL
struct EXPORT_API
{
  struct STR_API* pStr;
  struct SYS_API* pSys;
  struct RT_API* pRt;
  struct FS_API* pFs;
  struct MNU_API* pMnu;
  struct TV_API* pTv;
};

extern struct EXPORT_API g_api __attribute__ ((dllexport));
#endif //__DLL

#endif //__RT_H__
