#include "common.h"


void EditPrintf(HWND hEdit, PCWSTR szFormat, ...)
{
  WCHAR   szBuffer [0x10000];
  va_list pArgList;
  va_start(pArgList, szFormat);
  wvsprintf(szBuffer, szFormat, pArgList);
  va_end (pArgList);
  SendMessage(hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessage (hEdit, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
  SendMessage (hEdit, EM_SCROLLCARET, 0, 0);
}

void EditPrintfA(HWND hEdit, PCSTR szFormat, ...)
{
  CHAR szBuffer [0x10000];
  va_list pArgList;
  va_start(pArgList, szFormat);
  wvsprintfA(szBuffer, szFormat, pArgList);
  va_end (pArgList);
  SendMessageA(hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
  SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
}

