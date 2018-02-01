#include "common.h"

#pragma data_seg("shared")
BOOL g_bInMenu = FALSE;
#pragma data_seg()

void AppendTextMenu(HMENU hMenu, PCWSTR szText, PUINT puItemId)
{
  AppendMenu(
    hMenu,
    szText ? MF_STRING : MF_SEPARATOR,
    (*puItemId)++,
    szText
    );
}

UINT PopupTextMenu(HMENU hMenu)
{
  POINT pt;
  UINT uCmd;
  g_bInMenu = TRUE;
  GetCursorPos(&pt);
  uCmd = TrackPopupMenu(
    hMenu,
    TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
    pt.x,
    pt.y,
    0,
    g_hMainWnd,
    0
    );
  g_bInMenu = FALSE;
  return uCmd;
}

void GetMenuItemText(HMENU hMenu, UINT uCmd, PWSTR szBuff, UINT uBuffLen)
{
  MENUITEMINFO  info = { sizeof(MENUITEMINFO), MIIM_STRING };
  info.dwTypeData = szBuff;
  info.cch        = uBuffLen;
  GetMenuItemInfo(hMenu, uCmd, FALSE, &info);
}

void PopupMenu(PMENU_TYPE pm)
{
  HMENU hMenu;
  HKEY hKey;
  POINT pt;
  UINT i = 1;
  DWORD dwIndex = 0;
  PREG_MENU prm = 0;
  PTEXT_MENU ptm = (PTEXT_MENU)pm;

  hMenu = CreatePopupMenu();
  if(!hMenu)
  {
    ptm->SelProc = EmptyProc;
    return;
  }
  g_bInMenu = TRUE;

  if(ptm->puFlags)
  {
    for(i = 1; i < ptm->uCount; i ++)
    {
      AppendMenu(
        hMenu,
        ptm->puFlags[i],
        i,
        ptm->pszItemArr[i]);
    }
  }
  else
  {
    for(i = 1; i < ptm->uCount; i ++)
    {
      AppendMenu(
        hMenu,
        ptm->pszItemArr[i] ? MF_STRING : MF_SEPARATOR,
        i,
        ptm->pszItemArr[i]);
    }
  }
  if(*pm == MT_REG_MENU)
  {
    prm = (PREG_MENU)pm;
    if(ERROR_SUCCESS == RegOpenKey(prm->hKeyRoot, prm->mb.szItemTextBuff, &hKey))
    {
      while(ERROR_SUCCESS == RegEnumKey(hKey, dwIndex ++, prm->mb.szItemTextBuff, prm->mb.dwItemTextBuffLen))
      {
        AppendMenu(
          hMenu,
          MF_STRING | prm->InitCheckProc(prm),
          i ++,
          prm->mb.szItemTextBuff);
      }
      RegCloseKey(hKey);
    }
  }
  GetCursorPos(&pt);
  i = TrackPopupMenu(
    hMenu,
    TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
    pt.x,
    pt.y,
    0,
    g_hMainWnd,
    0
    );
  
  if(ptm->szItemTextBuff)
  {
    *ptm->szItemTextBuff = 0;
    GetMenuItemText(hMenu, i, ptm->szItemTextBuff, ptm->dwItemTextBuffLen);
  }

  DestroyMenu(hMenu);
  if(ptm && i < ptm->uCount)
    ptm->SelProc = ptm->ProcTable[i];
  g_bInMenu = FALSE;
}
