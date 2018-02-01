#include "common.h"

static LPCWSTR szHtmlFmt = L"HTML Format";

int Clipboard()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. HTML...",
      L"&2. Text..."
  };

  static UINT uFlags[] =
  {
    0,
      MF_STRING,
      MF_STRING
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Html,
      Text
  };

  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , uFlags
      , DispatchTable
  };

  UINT uFmtHtml = RegisterClipboardFormat(szHtmlFmt);

  uFlags[1] |= (IsClipboardFormatAvailable(uFmtHtml) ? 0 : MF_GRAYED);
  uFlags[2] |=
    (
    (IsClipboardFormatAvailable(CF_TEXT) |
    IsClipboardFormatAvailable(CF_UNICODETEXT)) ? 0 : MF_GRAYED
    );

  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();
  return 0;
}

int WndTxtToClipboard(HWND hWnd)
{
  HANDLE  hClipBuffer;
  PVOID   pClipBuffer;
  static LPCWSTR szTitle = L"Error copying to clipboard";
  static LPCWSTR szErrOpen = L"Cannot open the clipboard";
  static LPCWSTR szErrCopy = L"Cannot copy data to clipboard";
  static LPCWSTR szErrAlloc = L"Cannot allocate memory to copy";
  static LPCTSTR szErrEmpty = L"Cannot empty the clipboard";
  int     iCharCount = -1;
  if(!OpenClipboard(g_hMainWnd))
  {
    MessageBox(
      g_hMainWnd,
      szErrOpen,
      szTitle,
      MB_OK | MB_ICONERROR
      );
    return iCharCount;
  }
  if(EmptyClipboard())
  {
    iCharCount  = 1 + GetWindowTextLengthA(hWnd);
    hClipBuffer = GlobalAlloc(
      GMEM_MOVEABLE, iCharCount * sizeof(CHAR)
      );
    if(hClipBuffer)
    {
      pClipBuffer = GlobalLock(hClipBuffer);
      GetWindowTextA(hWnd, pClipBuffer, iCharCount);
      GlobalUnlock(hClipBuffer);
      if(!SetClipboardData(CF_TEXT, hClipBuffer))
      {
        iCharCount = -1;
        MessageBox(
          g_hMainWnd,
          szErrCopy,
          szTitle,
          MB_OK | MB_ICONERROR
          );
      }
      GlobalFree(hClipBuffer);
    }
    else
    {
      iCharCount = -1;
      MessageBox(
      g_hMainWnd,
      szErrAlloc,
      szTitle,
      MB_OK | MB_ICONERROR
      );
    }
  }
  else
  {
    MessageBox(
      g_hMainWnd,
      szErrEmpty,
      szTitle,
      MB_OK | MB_ICONERROR
      );
  }
  CloseClipboard();
  return iCharCount;
}

void ClipGetHtml(void (*ReadProc)(PCSTR, PVOID), PVOID pContext)
{
  HANDLE hClipData;
  UINT uFmtHtml = RegisterClipboardFormat(szHtmlFmt);
  if(!IsClipboardFormatAvailable(uFmtHtml))
    return;
  if(!OpenClipboard(g_hMainWnd))
    return;
  hClipData = GetClipboardData(uFmtHtml);
  CloseClipboard();
  if(hClipData)
  {
    ReadProc((LPCSTR)GlobalLock(hClipData), pContext);
    GlobalUnlock(hClipData);
  }
  else
    ReadProc(0, pContext);
}

void ClipGetData(void (*ReadProc)(SIZE_T, PVOID, PVOID), UINT uFmt, PVOID pContext, BOOL bSave)
{
  HANDLE hClipData;
  if(!IsClipboardFormatAvailable(uFmt))
    return;
  if(!OpenClipboard(g_hMainWnd))
    return;
  hClipData = GetClipboardData(uFmt);
  if(hClipData)
  {
    ReadProc(GlobalSize(hClipData), GlobalLock(hClipData), pContext);
    GlobalUnlock(hClipData);
    if(bSave)
      SetClipboardData(uFmt, hClipData);
  }
  else
    ReadProc(0, 0, pContext);
  CloseClipboard();
}

void ClipSetData(UINT uFmt, HANDLE hMem)
{
  if(!OpenClipboard(g_hMainWnd))
    return;
  EmptyClipboard();
  SetClipboardData(uFmt, hMem);
  CloseClipboard();
}

