#include "common.h"

static PCWSTR szRegShell   = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
static PCWSTR szKeyAppData = L"AppData";



BOOL GotoShellFolder(PCWSTR szFolder)
{
  WCHAR wchBuff[MAX_PATH];
  HKEY hKey = 0;
  DWORD dwType = REG_SZ;
  DWORD dwSize = sizeof(wchBuff);
  RegOpenKey(HKEY_CURRENT_USER, szRegShell, &hKey);
  *wchBuff = 0;
  RegQueryValueEx(hKey, szFolder, 0, &dwType, (PBYTE)wchBuff, &dwSize);
  RegCloseKey(hKey);
  return SetCurrentDirectory(wchBuff);
}

BOOL GotoDataFolder()
{
  if(!GotoShellFolder(szKeyAppData))
    return FALSE;
  CreateDirectory(g_szAppName, 0);
  return SetCurrentDirectory(g_szAppName);
}

BOOL Location(PCWSTR szTitle, PDWORD pwdErrCode, UINT uFlags)
{
  WCHAR wchPath[0x8000];
  LPITEMIDLIST lpid;
  BROWSEINFO bi = {0};
  LPMALLOC pMalloc = 0;
  BOOL bResult = FALSE;
  *pwdErrCode = 2;
  bi.hwndOwner = g_hMainWnd;
  bi.lpszTitle = szTitle;
  bi.ulFlags = uFlags;
  lpid = SHBrowseForFolder(&bi);
  if(lpid)
  {
    if(
      SHGetPathFromIDList(lpid, wchPath) &&
      NOERROR == SHGetMalloc(&pMalloc)
      )
    {
      pMalloc->lpVtbl->Free(pMalloc, lpid);
      pMalloc->lpVtbl->Release(pMalloc);
      bResult = SetCurrentDirectory(wchPath);
      *pwdErrCode = bResult;
    }
  }
  return bResult;
}


void TimeFileName(PWSTR szNameBuff, PCWSTR szPref, PCWSTR szPost)
{
  static const PCWSTR szFmt = L"%s%04d.%02d.%02d-%02d.%02d.%02d%s";
  SYSTEMTIME st;
  GetLocalTime(&st);
  wsprintf(szNameBuff, szFmt, szPref, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, szPost);
}
