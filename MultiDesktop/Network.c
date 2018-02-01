#include "common.h"

static LPCWSTR szSockErrTitle = L"Socket error";
static LPCWSTR szErrInitSock  = L"Cannot initialise socket library";

int Network()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Shoutcast..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Shoutcast
  };
  
  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
  };

  WSADATA wsaData;

  if(WSAStartup(MAKEWORD(2, 2), &wsaData))
  {
    MessageBox(g_hMainWnd, szErrInitSock, szSockErrTitle, MB_OK | MB_ICONEXCLAMATION);
    return 0;
  }

  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();

  WSACleanup();
  return 0;
}

