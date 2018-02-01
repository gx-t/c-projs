#include "common.h"

int Encryption()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. RC4 encript/decrypt..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Rc4EncrypDecrypt
  };

  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
  };

  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();
  return 0;
}
