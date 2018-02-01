#ifndef __MNU_H__
#define __MNU_H__

struct MNU_GLB
{
  DWORD dwDummy;
};

struct MNU_API
{
  struct MNU_GLB* pGlb;
  UINT (*MnuPopupMenu)(HWND, UINT, ...);
};

extern struct MNU_API g_mnu;

#endif //__MNU_H__
