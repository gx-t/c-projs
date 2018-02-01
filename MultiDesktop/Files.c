#include "common.h"

static PCWSTR szRegDesk   = L"Desktop";
static PCWSTR szRegFavor  = L"Favorites";

static PCWSTR szGrab      = L"Grab Internet Shortcuts...";
static PCWSTR szErrOutF   = L"Cannot create the output file";
static PCWSTR szDelNote   = L"Would you like the shortcuts to be deleted after processing?";
static PCWSTR szOpen      = L"Open";
static PCWSTR szErrDesk   = L"Cannot move to users Desktop folder";
static PCWSTR szErrFavor  = L"Cannot move to users Favorites folder";
static PCWSTR szErrLoc    = L"Cannot move to selected folder";
static PCWSTR szErrMvFldr = L"Cannot change the current folder";

struct SCAN_DATA
{
  WIN32_FIND_DATA fd;
  HANDLE hOutFile;
  int iCol;
  BOOL bDelete;
  CHAR chBuff[0x400];
};

static void ProcessShortcut(struct SCAN_DATA* pData)
{
  static LPSTR szShCut = "\nURL";
  static CHAR szLnk0[] = "<a href=";
  static CHAR szLnk1[] = "> ";
  static CHAR szLnk2[] = " </a><br>\r\n";
  static CHAR szHexFmt[] = "&#%04d;";
  static CHAR szSpace[] = "&nbsp ";
  static WCHAR szExt[] = L".url";
  LPSTR cp1, cp2;
  LPWSTR wp1, wp2;
  int i;
  DWORD dwBytes = 0;
  HANDLE hFile;
  HANDLE hFind = FindFirstFile(L"*", &pData->fd);
  if(hFind == INVALID_HANDLE_VALUE)
    return;
  while(FindNextFile(hFind, &pData->fd))
  {
    if(pData->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if(
        pData->fd.cFileName[0] == L'.' &&
        pData->fd.cFileName[1] == L'.' &&
        pData->fd.cFileName[2] == 0
        )
        continue;
      if(SetCurrentDirectory(pData->fd.cFileName))
      {
        pData->iCol ++;
        ProcessShortcut(pData);
        pData->iCol --;
        SetCurrentDirectory(L"..");
      }
    }
    else
    {
      for(wp1 = pData->fd.cFileName, wp2 = szExt; *wp1; wp1 ++)
        wp2 = *wp1 == *wp2 ? wp2 + 1 : szExt;
      
      if(*wp2)//extension not found
        continue;//next file
      
      hFile =
        CreateFile(pData->fd.cFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        0);
      if(hFile == INVALID_HANDLE_VALUE)
        break;
      dwBytes = 0;
      ReadFile(hFile, pData->chBuff, sizeof(pData->chBuff) - 1, &dwBytes, 0);
      CloseHandle(hFile);
      if(pData->bDelete)
        DeleteFile(pData->fd.cFileName);
      
      pData->chBuff[dwBytes] = 0;
      
      for(cp1 = pData->chBuff, cp2 = szShCut; *cp2 && *cp1; cp1 ++)
        cp2 = *cp1 == *cp2 ? cp2 + 1 : szShCut;
      
      if(*cp2)//URL not found
        continue;//next file
      
      while(*cp1 && isspace(*cp1))//pass spaces
        cp1 ++;
      if(!*cp1 || *cp1 != '=')//spaces till the end or not '='
        continue;//next file
      cp1 ++;
      
      while(*cp1 && isspace(*cp1))//pass spaces
        cp1 ++;
      if(!*cp1)
        continue;
      
      cp2 = cp1;
      while(*cp2 && !isspace(*cp2))//run until first space
        cp2 ++;
      
      WriteFile(pData->hOutFile, szLnk0, sizeof(szLnk0) - 1, &dwBytes, 0);
      WriteFile(pData->hOutFile, cp1, (DWORD)cp2 - (DWORD)cp1, &dwBytes, 0);
      WriteFile(pData->hOutFile, szLnk1, sizeof(szLnk1) - 1, &dwBytes, 0);

      for(i = pData->iCol << 2; i; i --)
        WriteFile(pData->hOutFile, szSpace, 6, &dwBytes, 0);

      for(wp1 = pData->fd.cFileName; *wp1; wp1 ++);
      wp2 = wp1 - 4;
      
      for(wp1 = pData->fd.cFileName; wp1 < wp2; wp1 ++)
      {
        if(*wp1 > 255)
        {
          wsprintfA(pData->chBuff, szHexFmt, *wp1);
          WriteFile(pData->hOutFile, pData->chBuff, 7, &dwBytes, 0);
        }
        else
          WriteFile(pData->hOutFile, wp1, 1, &dwBytes, 0);
      }
      WriteFile(pData->hOutFile, szLnk2, sizeof(szLnk2) - 1, &dwBytes, 0);
    }
  }
  FindClose(hFind);
}

static void GrabShortcuts()
{
  struct SCAN_DATA Data;
  static PCWSTR szPost = L".html";
  static CHAR szHtmlBegin[] = "<html>\r\n";
  static CHAR szHtmlEnd[] = "</html>";
  WCHAR wchFile[32];
  DWORD dwBytes;

  TimeFileName(wchFile, 0, szPost);
  
  Data.bDelete = IDYES ==
    MessageBox(g_hMainWnd, szDelNote, szGrab, MB_YESNO | MB_ICONEXCLAMATION);
  
  Data.hOutFile = CreateFile(wchFile, GENERIC_WRITE, FILE_SHARE_READ, 0,
    CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
  
  if(Data.hOutFile == INVALID_HANDLE_VALUE)
  {
    MessageBox(g_hMainWnd, szErrOutF, szGrab, MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  Data.iCol = 0;
  WriteFile(Data.hOutFile, szHtmlBegin, sizeof(szHtmlBegin) - 1, &dwBytes, 0);
  ProcessShortcut(&Data);

  WriteFile(Data.hOutFile, szHtmlEnd, sizeof(szHtmlEnd) - 1, &dwBytes, 0);
  CloseHandle(Data.hOutFile);
  ShellExecute(0, szOpen, wchFile, L".", 0, SW_SHOWNORMAL);
}

static int Desktop()
{
  if(!GotoShellFolder(szRegDesk))
  {
    MessageBox(g_hMainWnd, szErrDesk, szErrMvFldr, MB_OK | MB_ICONEXCLAMATION);
    return 0;
  }
  GrabShortcuts();
  ShellExecute(0, szOpen, L".", 0, 0, SW_SHOWNORMAL);
  return 0;
}

static int Favorites()
{
  if(!GotoShellFolder(szRegFavor))
  {
    MessageBox(g_hMainWnd, szErrFavor, szErrMvFldr, MB_OK | MB_ICONEXCLAMATION);
    return 0;
  }
  GrabShortcuts();
  ShellExecute(0, szOpen, L".", 0, 0, SW_SHOWNORMAL);
  return 0;
}

static int FromLocation()
{
  DWORD dwErrCode;
  if(!Location(szGrab, &dwErrCode, BIF_RETURNONLYFSDIRS | BIF_EDITBOX))
  {
    if(!dwErrCode)
      MessageBox(g_hMainWnd, szErrLoc, szErrMvFldr, MB_OK | MB_ICONEXCLAMATION);
    return 0;
  }
  GrabShortcuts();
  ShellExecute(0, szOpen, L".", 0, 0, SW_SHOWNORMAL);
  return 0;
}

static int Shortcuts()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. From desktop",
      L"&2. From IE favorites",
      L"&3. From location..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Desktop,
      Favorites,
      FromLocation
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
  return mt.SelProc();
}

int Files()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Internet shortcuts to file...",
	  L"&2. Scan files..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Shortcuts,
	  ScanFiles
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
