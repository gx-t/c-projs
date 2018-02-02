#include "rt.h"

static UINT MnuPopupMenu(HWND hWnd, UINT uCount, ...)
{
  va_list pArgList;
  POINT pt;
  RECT rc;
  HMENU hMenu;
  UINT uCmd = 0;
  HTREEITEM hSelItem = TreeView_GetSelection(hWnd);
  if(!hSelItem)
    return 0;
  hMenu = CreatePopupMenu();
  if(!hMenu)
    return 0;
  va_start(pArgList, uCount);
  for(uCmd = 0; uCmd < uCount; uCmd ++)
  {
    PCWSTR szText = va_arg(pArgList, PCWSTR);
    AppendMenu(hMenu, szText ? MF_STRING : MF_SEPARATOR, uCmd + 1, szText);
  }
  va_end (pArgList);
  if(TreeView_GetItemRect(hWnd, hSelItem, &rc, TRUE))
  {
    pt.x = (rc.left + rc.right) >> 1;
    pt.y = (rc.top + rc.bottom) >> 1;
    ClientToScreen(hWnd, &pt);
    uCmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, 0);
  }
  DestroyMenu(hMenu);
  return uCmd;
}

struct MNU_API g_mnu =
{
  0 //struct MNU_GLB* pGlb
  , MnuPopupMenu
};
