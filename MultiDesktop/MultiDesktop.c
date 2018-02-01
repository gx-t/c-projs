#include "common.h"
#include "resource.h"

extern BOOL g_bInMenu;

void main()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Manage desktops...",
      0,
      L"&2. Net Send...",
      L"&3. Clipboard...",
      L"&4. Encryption...",
      L"&5. Network...",
      L"&6. Files...",
      0,
      L"&7. Separate Tools..."
  };

  static int (*DispatchTable[])()=
  {
    EmptyProc,
      DesktopList,
      0,
      NetSend,
      Clipboard,
      Encryption,
      Network,
      Files,
      0,
      Tools
  };

  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
  };

  INITCOMMONCONTROLSEX icex =
  {
    sizeof(INITCOMMONCONTROLSEX),
      ICC_TREEVIEW_CLASSES | ICC_DATE_CLASSES
  };

  if(g_bInMenu)
    return;

  InitCommonControlsEx(&icex);
  InitMainWindow();
  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();
  DeleteMainWindow();
  ExitProcess(0);
}

