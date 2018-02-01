#include "common.h"


static int ToJavaUni()
{
  return 0;
}

static int ToCppUni()
{
  return 0;
}

void TxtFormat()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. To Java unicode",
      L"&2. To C++ unicode"
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      ToJavaUni,
      ToCppUni
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
}

