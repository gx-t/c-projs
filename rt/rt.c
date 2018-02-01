#include "rt.h"

static struct RT_GLB g_glb;

static void RtPassMsg()
{
  while(PeekMessage(&g_glb.msg, 0, 0, 0, PM_REMOVE))
  {
    DispatchMessage(&g_glb.msg);
  } 
}

static void RtMessageLoop()
{
  while(GetMessage(&g_glb.msg, 0, 0, 0))
  {
    DispatchMessage(&g_glb.msg);
  }
}

int RtMain()
{
  g_sys.SysLoadDlls();
  g_tv.TvInit();
  RtMessageLoop();
  g_tv.TvFree();
  g_sys.SysUnloadDlls();
  ExitProcess(0);//SHBrowseForFolder
  return 0;
}

static void RtExitProcess()
{
  ExitProcess(0);
}

struct RT_API g_rt =
{
  &g_glb
  , RtMain
  , RtPassMsg
  , RtMessageLoop
  , RtExitProcess
};
#ifdef __DLL
struct EXPORT_API g_api =
{
  &g_str
    , &g_sys
    , &g_rt
    , &g_fs
    , &g_mnu
    , &g_tv
};

int RtDlEntry()
{
  return 1;
}
#endif //__DLL
