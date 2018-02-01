#include "common.h"

void MessageLoop()
{
  MSG msg;
  while(GetMessage(&msg, 0, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

