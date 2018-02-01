#include "common.h"

extern LPCWSTR szHtmlFmt;

struct HTML_HEADER
{
  LPSTR szData;
  LPSTR szBase;
  DWORD dwBaseLen;
  LPSTR szHtml;
  DWORD dwHtmlLen;
};

static void CrackHtmlHeader(struct HTML_HEADER* pHeader)
{
  static LPCSTR szStart = "StartHTML:";
  static LPCSTR szEnd = "EndHTML:";
  static LPCSTR szSource = "SourceURL:";
  LPSTR s1, s2;
  DWORD dwStart;
  pHeader->dwHtmlLen = 0;
  pHeader->szHtml = pHeader->szData;
  for(s1 = (LPSTR)pHeader->szData; *s1; s1 ++)
  {
    for(s2 = (LPSTR)szStart; *s2 && *s1 == *s2; s1 ++, s2 ++);
    if(!*s2)
      break;
  }
  dwStart = 0;
  for(; *s1 != '\r' && *s1 != '\n'; s1 ++)
    dwStart *= 10, dwStart += (*s1 - '0');
  pHeader->szHtml += dwStart;
  for(; s1 < pHeader->szHtml; s1 ++)
  {
    for(s2 = (LPSTR)szEnd; *s2 && *s1 == *s2; s1 ++, s2 ++);
    if(!*s2)
      break;;
  }
  pHeader->dwHtmlLen = 0;
  for(; *s1 != '\r' && *s1 != '\n'; s1 ++)
    pHeader->dwHtmlLen *= 10, pHeader->dwHtmlLen += (*s1 - '0');
  pHeader->dwHtmlLen -= dwStart;
  for(; s1 < pHeader->szHtml; s1 ++)
  {
    s2 = (LPSTR)szSource;
    for(; *s2 && *s1 == *s2; s1 ++, s2 ++);
    if(!*s2)
      break;
  }
  pHeader->szBase = s1;
  pHeader->dwBaseLen = 0;
  for(; *s1 != '\r' && *s1 != '\n'; s1 ++)
    pHeader->dwBaseLen ++;
}

static void SaveToHtml(LPCSTR pch, PVOID pContext)
{
  static LPCWSTR szHtmFlt = L"HTML Files (*.htm,*.html)\0*.html;*.htm\0\0";
  static LPCWSTR szTitle = L"Save Clipboard To Html";
  static LPCWSTR szHtml = L"html";
  static LPCSTR szBase = "<BASE href=";
  static LPCSTR szBaseEnd = ">\r\n";
  static LPCWSTR szErrCreate = L"Maybe you have not permission to write in this folder or the disk is full";
  static LPCWSTR szErrTitle = L"Cannot write the output html file!";

  HANDLE hFile;
  DWORD dwBytesWritten;
  WCHAR szFileName[MAX_PATH + 1];
  struct HTML_HEADER Head = {0};
  static OPENFILENAME ofn = { sizeof(OPENFILENAME) };
  pContext = 0;
  
  *szFileName = 0;
  ofn.hwndOwner = g_hMainWnd;
  ofn.hInstance = GetModuleHandle(0);
  ofn.Flags = OFN_OVERWRITEPROMPT;
  ofn.lpstrFilter = szHtmFlt;
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = szTitle;
  ofn.lpstrDefExt = szHtml;
  if(!GetSaveFileName(&ofn))
    return;
  hFile = CreateFile(
    szFileName,
    GENERIC_WRITE,
    FILE_SHARE_READ,
    0,
    CREATE_ALWAYS,
    FILE_FLAG_SEQUENTIAL_SCAN,
    0);
  if(hFile == INVALID_HANDLE_VALUE)
  {
    MessageBox(
      g_hMainWnd,
      szErrCreate,
      szErrTitle,
      MB_OK | MB_ICONEXCLAMATION
      );
    return;
  }
  Head.szData = (LPSTR)pch;
  CrackHtmlHeader(&Head);
  WriteFile(hFile, szBase, lstrlenA(szBase), &dwBytesWritten, 0);
  WriteFile(hFile, Head.szBase, Head.dwBaseLen, &dwBytesWritten, 0);
  WriteFile(hFile, szBaseEnd, lstrlenA(szBaseEnd), &dwBytesWritten, 0);

  WriteFile(hFile, Head.szHtml, Head.dwHtmlLen, &dwBytesWritten, 0);
  CloseHandle(hFile);
}

static int ClipHtmlSave()
{
  ClipGetHtml(SaveToHtml, 0);
  return 0;
}

static void ParseForLinks(PCSTR szHtml, HWND hEdit)
{
  static LPCSTR szHref = "href";
  LPSTR s1, s2;
  CHAR szBuff[1024];
  struct HTML_HEADER Header = {0};
  Header.szData = (LPSTR)szHtml;
  CrackHtmlHeader(&Header);
  for(s1 = Header.szHtml; *s1; s1 ++)
  {
    s2 = (LPSTR)szHref;
    for(; *s2 == *s1 || *s2 == *s1 + 'a' - 'A'; s1 ++, s2 ++);
    if(!*s2)
    {
      for(; (*s1 && *s1 <= 0x27) || *s1 == '='; s1 ++);
      s2 = szBuff;
      for(; *s1 && *s1 > 0x27; s1 ++)
      {
        *s2++ = *s1;
      }
      *s2++ = '\r';
      *s2++ = '\n';
      *s2 = 0;
      SendMessageA(hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
      SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)szBuff);
      SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
    }
  }
}

static BOOL CALLBACK NotepadMainWndFindProc(HWND hWnd, LPARAM lParam)
{
  lParam = 0;
  hWnd = GetWindow(hWnd, GW_CHILD);
  if(!hWnd)
    return FALSE;
  ClipGetHtml((void(*)(PCSTR, PVOID))ParseForLinks, hWnd);
  Edit_SetModify(hWnd, TRUE);
  return FALSE;
}

static int ExtractLinks()
{
  static WCHAR szNotepad[16];
  PROCESS_INFORMATION pi = {0};
  STARTUPINFO si = {sizeof(STARTUPINFO)};
  lstrcpy(szNotepad, L"Notepad.exe");
  GetStartupInfo(&si);
  if(!CreateProcess(0, szNotepad, 0, 0, 0, 0, 0, 0, &si, &pi))
    return 0;
  WaitForInputIdle(pi.hProcess, INFINITE);
  EnumThreadWindows(pi.dwThreadId, NotepadMainWndFindProc, 0);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return 0;
}

int Html()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Save html content...",
      L"&2. Extract links..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      ClipHtmlSave,
      ExtractLinks
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

