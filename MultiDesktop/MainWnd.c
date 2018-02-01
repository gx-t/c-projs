#include "common.h"

HWND g_hMainWnd;

void InitMainWindow()
{
  static LPCWSTR szClass = L"53a447d1-0e9f-4938-a640-bae6cf479ef6";
  WNDCLASS wc       = {0};
  wc.lpfnWndProc    = DefWindowProc;
  wc.hInstance      = GetModuleHandle(0);
  wc.lpszClassName  = szClass;
  RegisterClass(&wc);
  g_hMainWnd = CreateWindow(
    szClass,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    wc.hInstance,
    0
    );
  SetForegroundWindow(g_hMainWnd);
}

void DeleteMainWindow()
{
  CloseWindow(g_hMainWnd);
}

