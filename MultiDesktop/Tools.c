#include "common.h"

int Tools()
{
  HANDLE hFind; 
  WIN32_FIND_DATA fd;
  LPWSTR p = fd.cFileName;
  STARTUPINFO si = {sizeof(STARTUPINFO)};
  PROCESS_INFORMATION pi;
  static LPCWSTR szToolDir = L"Tools";
  static LPCWSTR szToolExeFilter = L"*.exe";
  UINT uCmd = 1;
  HMENU hMenu = CreatePopupMenu();
  if(!hMenu)
    return 0;
  if(
    !SetCurrentDirectory(szToolDir) ||
    INVALID_HANDLE_VALUE == (hFind = FindFirstFile(szToolExeFilter, &fd))
    )
  {
    DestroyMenu(hMenu);
    return 0;
  }
  do {
    while(*p++);
    p -= 5;
    *p = 0;
    AppendTextMenu(hMenu, fd.cFileName, &uCmd);
  } while(FindNextFile(hFind, &fd));
  FindClose(hFind);
  uCmd = PopupTextMenu(hMenu);
  if(uCmd)
  {
    GetMenuString(hMenu
      , uCmd
      , fd.cFileName
      , sizeof(fd.cFileName) / sizeof(fd.cFileName[0])
      , MF_BYCOMMAND);
    GetStartupInfo(&si);
    *p = L'.';
    if(CreateProcess(0, (LPTSTR)fd.cFileName, 0, 0, 0, 0, 0, 0, &si, &pi))
    {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
  }
  DestroyMenu(hMenu);
  return 0;
}

