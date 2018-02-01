#include "common.h"

struct UDATA
{
  DWORD dwLen;
  DWORD dwMaxLen;
  PVOID pData;
};
typedef void (CALLBACK *SYSTEMFUNCTION032)(struct UDATA*, struct UDATA*);

static SYSTEMFUNCTION032 SystemFunction032;
int Rc4EncrypDecrypt()
{
  static LPCWSTR szAdvapi = L"Advapi32.dll";
  static LPCSTR szSystemFunction032 = "SystemFunction032";
//  HWND hProgress;
  HMODULE hDll = GetModuleHandle(szAdvapi);
  if(!hDll)
    return 0;
  SystemFunction032 =
    (SYSTEMFUNCTION032)GetProcAddress(hDll, szSystemFunction032);
  if(!SystemFunction032)
    return 0;/*
  hProgress = CreateWindowEx(
    WS_EX_APPWINDOW | WS_EX_TOOLWINDOW,
    PROGRESS_CLASS,
    L"Test Progress",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    300,
    40,
    g_hMainWnd,
    0,
    GetModuleHandle(0),
    0
    );
  ShowWindow(hProgress, SW_SHOW);
  UpdateWindow(hProgress);
  MessageLoop();*/
  return 0;
}

