#include "common.h"
#include "resource.h"

#define SEC_IN_24H                (24 * 60 * 60)
#define SEC_IN_HOUR               (60 * 60)
#define SEC_IN_MIN                (60)
#define WDATA(h_)                 ((struct SHOUT_DATA*)GetWindowLong(h_, GWL_USERDATA))

static LPCWSTR szErrFolderTitle = L"The dropped object is unusable";
static LPCWSTR szErrFolder =      L"Only directories may be dropped here";
static LPCWSTR szErrEnter =       L"There is no permission to enter the directory";
static LPCWSTR szErrCreate =      L"There is no permission to create file";
static LPCWSTR szOpen =           L"open";
static LPCWSTR szCritErrTitle =   L"Critical Error";
static LPCWSTR szErrSockCreate =  L"Cannot create socket";
static LPCWSTR szErrFindRadio =   L"Check the Radio Station URL";
static LPCWSTR szTitleFindRadio = L"Cannot find the radio server";
static LPCWSTR szErrEmptyURL =    L"The URL string is empty! The URL format is xxx:pp";
static LPCWSTR szErrConnRadio =   L"Check the internet status";
static LPCWSTR szTitleConnRadio = L"Cannot connect to server";
static LPCWSTR szRegAddrList    = L"Software\\ShahSoft\\Multidesktop\\Net\\Shout\\Ripp\\UrlLst";
static LPCWSTR szRegRipp        = L"Software\\ShahSoft\\Multidesktop\\Net\\Shout\\Ripp";
static LPCWSTR szRegUrlList     = L"UrlLst";
static LPCWSTR szRegActURL      = L"ActiveURL";
static LPCWSTR szRegCheckData[] = 
{
  L"0", L"1", L"2", L"3", L"4"
};
static LPCWSTR szRegStrData[]   =
{
  L"Proxy", L"Login", L"Password", L"Folder"
};

static LPCWSTR szErrNoDelUrl    = L"No URL in combo box to be deleted";
static LPCWSTR szErrNoDelTtl    = L"There is nothing to be deleted";

static UINT uCheckIdLst[] =
{
  IDC_CHECK_USEPROXY,
    IDC_CHECK_USEAUTH,
    IDC_CHECK_ASWINAMP,
    IDC_CHECK_RECONIFLOST,
    IDC_CHECK_LOG
};

static UINT uEditIdList[] =
{
  IDC_EDIT_PROXYURL,
    IDC_EDIT_LOGIN,
    IDC_EDIT_PASSWORD,
    IDC_EDIT_FOLDER
};

static UINT uEditSize[] =
{
  256, 64, 64, MAX_FS_PATH
};

struct SHOUT_DATA
{
  HANDLE hRunning;
  HWND hWnd;
  SOCKET sConn;
  int iReconnCount;
  HANDLE hLogFile;
  void (*LogWriteProc)(struct SHOUT_DATA* pData, LPCSTR szFormat, ...);
  WNDPROC OldEditProc;
};

static void EmptyLogProc(struct SHOUT_DATA* pData, LPCSTR szFormat, ...)
{
}

static void WriteLogProc(struct SHOUT_DATA* pData, LPCSTR szFormat, ...)
{
  static CHAR szNL[] = "\r\n";
  CHAR szBuff[0x400];
  DWORD dwBytesWritten;
  DWORD dwBytesToWrite;
  SYSTEMTIME st;
  va_list pArgList;
  GetLocalTime(&st);
  dwBytesToWrite = 
    wsprintfA(szBuff, "%02d.%02d.%02d\t", st.wHour, st.wMinute, st.wSecond);
  WriteFile(pData->hLogFile, szBuff, dwBytesToWrite, &dwBytesWritten, 0);
  va_start(pArgList, szFormat);
  dwBytesToWrite = wvsprintfA(szBuff, szFormat, pArgList);
  va_end (pArgList);
  WriteFile(pData->hLogFile, szBuff, dwBytesToWrite, &dwBytesWritten, 0);
  WriteFile(pData->hLogFile, szNL, sizeof(szNL) - 1, &dwBytesWritten, 0);
}

static void LoadReg(HWND hDlg)
{
  HKEY hRoot, hKey;
  WCHAR szBuff[MAX_FS_PATH];
  DWORD dwData;
  DWORD dwIndex = 0;
  DWORD dwType = REG_SZ;
  DWORD dwSize = sizeof(szBuff);
  HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_RADIOURL);
  if(ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, szRegRipp, &hRoot))
    return;
  if(ERROR_SUCCESS == RegOpenKey(hRoot, szRegUrlList, &hKey))
  {
    while(
      ERROR_SUCCESS ==
      RegEnumKey(hKey, dwIndex, szBuff, sizeof(szBuff) / sizeof(WCHAR))
      )
    {
      ComboBox_AddString(hCombo, szBuff);
      dwIndex ++;
    }
    if(
      ERROR_SUCCESS ==
      RegQueryValueEx(hKey, L"", 0, &dwType, (PBYTE)szBuff, &dwSize)
      )
    {
      ComboBox_SelectString(hCombo, 0, szBuff);
    }
    RegCloseKey(hKey);
  }

  for(dwIndex = 0; dwIndex < 5; dwIndex ++)
  {
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    dwData = 0;
    RegQueryValueEx(
      hRoot,
      szRegCheckData[dwIndex],
      0,
      &dwType,
      (PBYTE)&dwData,
      &dwSize
      );
    CheckDlgButton(hDlg, uCheckIdLst[dwIndex], dwData);
  }

  for(dwIndex = 0; dwIndex < sizeof(uEditIdList) / sizeof(UINT); dwIndex ++)
  {
    dwType = REG_SZ;
    dwSize = sizeof(szBuff);
    *szBuff = 0;
    RegQueryValueEx(
      hRoot,
      szRegStrData[dwIndex],
      0,
      &dwType,
      (PBYTE)szBuff,
      &dwSize
      );
    SetDlgItemText(hDlg, uEditIdList[dwIndex], szBuff);
  }
  RegCloseKey(hRoot);
  GetDlgItemText(hDlg, IDC_EDIT_FOLDER, szBuff, MAX_FS_PATH);
  SetCurrentDirectory(szBuff);
}

static void SaveReg(HWND hDlg)
{
  DWORD dwIndex, dwData;
  HKEY hRoot, hKey, hSubKey;
  WCHAR szBuff[MAX_FS_PATH];
  HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_RADIOURL);
  if(ERROR_SUCCESS != RegCreateKey(HKEY_CURRENT_USER, szRegRipp, &hRoot))
    return;
  
  if(
    GetWindowText(hCombo, szBuff, sizeof(szBuff) / sizeof(WCHAR)) &&
    ERROR_SUCCESS == RegCreateKey(hRoot, szRegUrlList, &hKey)
    )
  {
    RegSetValueEx(
      hKey,
      L"",
      0,
      REG_SZ,
      (PBYTE)szBuff,
      sizeof(WCHAR) * GetWindowTextLength(hCombo)
      );
    if(CB_ERR == ComboBox_FindString(hCombo, 0, szBuff))
    {
      ComboBox_AddString(hCombo, szBuff);
      if(ERROR_SUCCESS == RegCreateKey(hKey, szBuff, &hSubKey))
        RegCloseKey(hSubKey);
    }
    RegCloseKey(hKey);
  }
  
  for(dwIndex = 0; dwIndex < 5; dwIndex ++)
  {
    dwData = IsDlgButtonChecked(hDlg, uCheckIdLst[dwIndex]);
    RegSetValueEx(
      hRoot,
      szRegCheckData[dwIndex],
      0,
      REG_DWORD,
      (PBYTE)&dwData,
      sizeof(DWORD)
      );
  }
  for(dwIndex = 0; dwIndex < sizeof(uEditIdList) / sizeof(UINT); dwIndex ++)
  {
    dwData =
      GetDlgItemText(hDlg, uEditIdList[dwIndex], szBuff, uEditSize[dwIndex]);
    RegSetValueEx(
      hRoot,
      szRegStrData[dwIndex],
      0,
      REG_SZ,
      (PBYTE)szBuff,
      dwData * sizeof(WCHAR)
      );
  }
  RegCloseKey(hRoot);
}

static DWORD TimeTo(HWND hWnd, UINT id)
{
  SYSTEMTIME stTime;
  int iTimeSec;

  DateTime_GetSystemtime(GetDlgItem(hWnd, id), &stTime);

  iTimeSec = (int)stTime.wSecond +
    SEC_IN_MIN * (int)stTime.wMinute +
    SEC_IN_HOUR * (int)stTime.wHour;

  GetLocalTime(&stTime);

  iTimeSec -= ((int)stTime.wSecond + SEC_IN_MIN * (int)stTime.wMinute +
    SEC_IN_HOUR * (int)stTime.wHour);

  if(iTimeSec < 0)
    iTimeSec += SEC_IN_24H;

  return (DWORD)iTimeSec * 1000;
}

static HANDLE CreateOutputFile(int iType)
{
  static const LPCWSTR szTypeList[] =
  {
    L"log",
      L"bin"
  };
  WCHAR wchFileName[32];
  TimeFileName(wchFileName, 0, szTypeList[iType]);
  return
    CreateFile(wchFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
}

static DWORD CALLBACK WorkThread(struct SHOUT_DATA* pData)
{
  HANDLE hOutFile;
  DWORD dwBytes;
  SOCKADDR_IN addr;
  HOSTENT *phost;
  int iPort;
  int iBytes;
  int iTotalBytes = 0;
  char szBuff[0x400];
  char* p1 = szBuff;
  char* p2 = p1;
  static char szHttp[] = "HTTP://";
  static char szGet[] = "GET /";
  static char szRoot[] = "/";
  static char szHttpVer[] = " HTTP/1.0\r\n";
  static char szAgentWinamp[] = "User-Agent: WinampMPEG/5.09\r\n";
  static char szAuth[] = "Proxy-Authorization: Basic ";
  static char szTail[] = "Accept: */*\r\nIcy-MetaData:1\r\nConnection: close\r\n\r\n";
  static LPCWSTR szRecvdFmt = L"%d byte received (Shoutcast Ripper)";

  ResetEvent(pData->hRunning);
  pData->hLogFile = CreateOutputFile(0);
  pData->LogWriteProc(pData, "Starting");
  if(IsDlgButtonChecked(pData->hWnd, IDC_CHECK_USESTOPTIMER))
    SetTimer(pData->hWnd, 2, TimeTo(pData->hWnd, IDC_TIME_STOP), 0);

  pData->iReconnCount =
    IsDlgButtonChecked(pData->hWnd, IDC_CHECK_RECONIFLOST) ? 32 : 1;
  while(pData->iReconnCount --)
  {
    if(pData->sConn)
      closesocket(pData->sConn);
    pData->sConn = socket(PF_INET, SOCK_STREAM, 0);
    if(IsDlgButtonChecked(pData->hWnd, IDC_CHECK_USEPROXY))
      GetDlgItemTextA(pData->hWnd, IDC_EDIT_PROXYURL, p1, 256);

    else
      GetDlgItemTextA(pData->hWnd, IDC_COMBO_RADIOURL, p1, 256);

    p1 = szHttp;
    for(; !((*p1 ^ *p2) & ~0x20); p1++, p2++);//pass 'http://' or 'HTTP://'
    if(*p1)
      p2 = szBuff;

    for(p1 = p2; *p1 && *p1 != ':'; p1++);//find first ':'
    if(*p1 == ':')
    {
      *p1++ = 0;//cut the URL, p2 contains the clear URL
      iPort = atoi(p1);
    }
    else
    {
      iPort = 80;
      for(p1 = p2; *p1 && *p1 != '/'; p1 ++);//first '/', after 'http://'!
      *p1 = 0;//cut the URL at first '/'
    }

    if(iPort < 1 || iPort > 0xFFFF)//assume port 80 if out of range
      iPort = 80;

    pData->LogWriteProc(pData, "Connecting to host: %s", p2);
    phost = gethostbyname(p2);//p2 is either radio or proxy URL
    if(!phost)
    {
      pData->LogWriteProc(pData, "Not found");
      break;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)iPort);
    addr.sin_addr.s_addr = *(long*)phost->h_addr_list[0];

    if(connect(pData->sConn, (SOCKADDR*)&addr, sizeof(SOCKADDR)))
    {
      pData->LogWriteProc(pData, "Cannot connect, reconnecting...");
      continue;//try to reconnect
    }

    for(p1 = szBuff, p2 = szGet; *p1 = *p2, *p2; p1++, p2++);//copy 'GET /'
    if(IsDlgButtonChecked(pData->hWnd, IDC_CHECK_USEPROXY))
    {
      p1 --;//URL begins in '/' position
      p1 += GetDlgItemTextA(pData->hWnd, IDC_COMBO_RADIOURL, p1, 256);
    }

    for(p2 = szHttpVer; *p1 = *p2, *p2; p1 ++, p2 ++);//append szHttpVer

    if(IsDlgButtonChecked(pData->hWnd, IDC_CHECK_ASWINAMP))
    {
      for(p2 = szAgentWinamp; *p1 = *p2, *p2; p1++, p2++);//append UA Winamp
    }
    if(
      IsDlgButtonChecked(pData->hWnd, IDC_CHECK_USEPROXY) &&
      IsDlgButtonChecked(pData->hWnd, IDC_CHECK_USEAUTH)
      )
    {
      for(p2 = szAuth; *p1 = *p2, *p2; p1++, p2++);//append szAuth

      p2 = szBuff + sizeof(szBuff) - 128;
      p2 += GetDlgItemTextA(pData->hWnd, IDC_EDIT_LOGIN, p2, 64);
      *p2 ++ = ':';
      p2 += GetDlgItemTextA(pData->hWnd, IDC_EDIT_PASSWORD, p2, 64);
      dwBytes = p2 - szBuff - sizeof(szBuff) + 128;
      p1 = Base64Encode(p1, szBuff + sizeof(szBuff) - 128, dwBytes);
      *p1 ++ = '\r';
      *p1 ++ = '\n';
    }
    for(p2 = szTail; *p1 = *p2, *p2; p1++, p2++);//append tail
    send(pData->sConn, szBuff, p1 - szBuff, 0);
    p1 = szBuff;
    iBytes = recv(pData->sConn, p1, 256, 0);
    if(iBytes < 8 || ' 002' != *(DWORD*)(szBuff + 4))
    {
      pData->LogWriteProc(pData, "Error in server response");
      break;//do not try to reconnect
    }

    iTotalBytes += iBytes;
    hOutFile = CreateOutputFile(1);
    if(hOutFile == INVALID_HANDLE_VALUE)
    {
      pData->LogWriteProc(pData, "Cannot create the output file");
      break;//do not try to reconnect
    }

    while(iBytes > 0)
    {
      WriteFile(hOutFile, p1, (DWORD)iBytes, &dwBytes, 0);
      wsprintf((LPWSTR)p1, szRecvdFmt, iTotalBytes);
      SetWindowText(pData->hWnd, (LPWSTR)p1);
      iBytes = recv(pData->sConn, p1, 256, 0);
      iTotalBytes += iBytes;      
    }
    if(pData->iReconnCount)
      pData->LogWriteProc(pData, "Error receiving data. Reconnecting...");
    CloseHandle(hOutFile);
  }

  if(pData->sConn)
    closesocket(pData->sConn);
  pData->sConn = 0;

  pData->LogWriteProc(pData, "Stopping. Received %d bytes", iTotalBytes);
  CloseHandle(pData->hLogFile);
  KillTimer(pData->hWnd, 2);
  EnableWindow(GetDlgItem(pData->hWnd, IDC_BTN_STOP), FALSE);
  EnableWindow(GetDlgItem(pData->hWnd, IDC_BTN_START), TRUE);
  SetEvent(pData->hRunning);
  return 0;
}

static void OnDelUrl(HWND hWnd)
{
  WCHAR szUrl[256];
  HKEY hRoot, hKey;
  HWND hCombo = GetDlgItem(hWnd, IDC_COMBO_RADIOURL);
  GetWindowText(hCombo,  szUrl, 256);
  if(!*szUrl)
  {
    MessageBox(hWnd, szErrNoDelUrl, szErrNoDelTtl, MB_OK | MB_ICONINFORMATION);
    return;
  }
  ComboBox_DeleteString(hCombo, ComboBox_GetCurSel(hCombo));
  if(ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, szRegRipp, &hRoot))
    return;
  if(ERROR_SUCCESS == RegOpenKey(hRoot, szRegUrlList, &hKey))
  {
    RegDeleteKey(hKey, szUrl);
    RegCloseKey(hKey);
  }
  RegCloseKey(hRoot);
}

static void OnStart(HWND hWnd)
{
  EnableWindow(GetDlgItem(hWnd, IDC_BTN_STOP), TRUE);
  EnableWindow(GetDlgItem(hWnd, IDC_BTN_START), FALSE);

  SaveReg(hWnd);
  QueueUserWorkItem(
    (LPTHREAD_START_ROUTINE)WorkThread,
    (PVOID)GetWindowLong(hWnd, GWL_USERDATA),
    WT_EXECUTELONGFUNCTION
    );
}

static void OnStop(HWND hWnd)
{
  struct SHOUT_DATA* pData = WDATA(hWnd);
  pData->iReconnCount = 0;
  if(pData->sConn)
  {
    closesocket(pData->sConn);
    pData->sConn = 0;
  }
  WaitForSingleObject(pData->hRunning, INFINITE);
}

static void OnTimer(HWND hWnd, WPARAM id)
{
  struct SHOUT_DATA* pData = WDATA(hWnd);
  if(id == 1)
  {
    if(IsWindowEnabled(GetDlgItem(pData->hWnd, IDC_BTN_START)))
      OnStart(hWnd);
    CheckDlgButton(hWnd, IDC_CHECK_USESTARTTIMER, BST_UNCHECKED);
  }
  else
  {
    OnStop(hWnd);
    CheckDlgButton(hWnd, IDC_CHECK_USESTOPTIMER, BST_UNCHECKED);
  }
}

static void OnDropFolder(HWND hWnd, HDROP hDrop)
{
  WCHAR chPath[MAX_FS_PATH + 1];
  DWORD dwAttr;
  HANDLE hTestFile;
  UUID uid = {0};
  LPWSTR pchTestFile = 0;
  DragQueryFile(hDrop, 0, chPath, sizeof(chPath) / sizeof(WCHAR));
  DragFinish(hDrop);
  dwAttr = GetFileAttributes(chPath);
  if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
  {
    MessageBox(hWnd,
      szErrFolder,
      szErrFolderTitle,
      MB_OK | MB_ICONINFORMATION);
    return;
  }
  if(!SetCurrentDirectory(chPath))
  {
    MessageBox(hWnd,
      szErrEnter,
      szErrFolderTitle,
      MB_OK | MB_ICONINFORMATION);
    return;
  }
  UuidCreate(&uid);
  UuidToString(&uid, &pchTestFile);
  hTestFile = CreateFile(pchTestFile, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
  if(hTestFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hTestFile);
    DeleteFile(pchTestFile);
    SetWindowText(hWnd, chPath);
  }
  else
    MessageBox(hWnd,
    szErrEnter,
    szErrCreate,
    MB_OK | MB_ICONINFORMATION);
  RpcStringFree(&pchTestFile);
}

static void OnExplorer(HWND hWnd)
{
  WCHAR chPath[MAX_FS_PATH + 1];
  GetDlgItemText(hWnd,
    IDC_EDIT_FOLDER,
    chPath,
    sizeof(chPath) / sizeof(WCHAR));
  if(!(GetFileAttributes(chPath) & FILE_ATTRIBUTE_DIRECTORY))
  {
    GetCurrentDirectory(sizeof(chPath), chPath);
  }
  ShellExecute(hWnd, szOpen, chPath, 0, 0, SW_SHOWNORMAL);
}

static LRESULT CALLBACK DropEditProc(HWND hWnd,
                                     UINT msg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
  if(msg == WM_DROPFILES)
    OnDropFolder(hWnd, (HDROP)wParam);
 
  return WDATA(GetParent(hWnd))->OldEditProc(hWnd, msg, wParam, lParam);
}

static BOOL CALLBACK ShoutDlgProc(HWND hWnd,
                                  UINT msg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
  int i;
  switch(msg)
  {
  case WM_INITDIALOG:
    for(i = 0; i < sizeof(uEditIdList) / sizeof(UINT); i ++)
      Edit_LimitText(GetDlgItem(hWnd, uEditIdList[i]), uEditSize[i] - 1);
    ComboBox_LimitText(GetDlgItem(hWnd, IDC_COMBO_RADIOURL), 255);

    LoadReg(hWnd);
    ((struct SHOUT_DATA*)lParam)->OldEditProc =
      (WNDPROC)SetWindowLong(GetDlgItem(hWnd, IDC_EDIT_FOLDER),
      GWL_WNDPROC,
      (LONG)DropEditProc);
    ((struct SHOUT_DATA*)lParam)->hWnd = hWnd;
    SetWindowLong(hWnd, GWL_USERDATA, lParam);
    break;
  case WM_TIMER:
    KillTimer(hWnd, wParam);
    OnTimer(hWnd, wParam);
    break;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_BTN_DELURL:
      OnDelUrl(hWnd);
      break;
    case IDC_BTN_START:
      OnStart(hWnd);
      break;
    case IDC_BTN_STOP:
      OnStop(hWnd);
      break;
    case IDC_CHECK_RECONIFLOST:
      WDATA(hWnd)->iReconnCount =
        BST_CHECKED == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) ? 32 : 1;
      break;
    case IDC_CHECK_USESTARTTIMER:
      if(BST_CHECKED == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
        SetTimer(hWnd, 1, TimeTo(hWnd, IDC_TIME_START), 0);
      else
        KillTimer(hWnd, 1);
      break;
    case IDC_CHECK_USESTOPTIMER:
      if(
        BST_CHECKED == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) &&
        IsWindowEnabled(GetDlgItem(hWnd, IDC_BTN_STOP))
        )
        SetTimer(hWnd, 2, TimeTo(hWnd, IDC_TIME_STOP), 0);
      else
        KillTimer(hWnd, 2);
      break;
    case IDC_BTN_EXPLORER:
      OnExplorer(hWnd);
      break;
    case IDC_CHECK_LOG:
      WDATA(hWnd)->LogWriteProc =
        (BST_CHECKED ==
        SendMessage(
        (HWND)lParam,
        BM_GETCHECK,
        0,
        0
        )) ? WriteLogProc : EmptyLogProc;
      break;
    }
    break;
  case WM_NOTIFY:
    if(((LPNMHDR)lParam)->code == DTN_DATETIMECHANGE)
    {
      if(wParam == IDC_TIME_START)
        SendMessage(
        GetDlgItem(hWnd, IDC_CHECK_USESTARTTIMER),
        BM_SETCHECK, BST_UNCHECKED, 0
        );
      else if(wParam == IDC_TIME_STOP)
        SendMessage(
        GetDlgItem(hWnd, IDC_CHECK_USESTOPTIMER),
        BM_SETCHECK, BST_UNCHECKED, 0
        );
    }
    break;
  case WM_CLOSE:
    OnStop(hWnd);
    EndDialog(hWnd, 0);
    break;
  }
  return FALSE;
}

static int ShoutRipp()
{
  struct SHOUT_DATA data = {0};
  data.hRunning = CreateEvent(0, TRUE, TRUE, 0);
  data.LogWriteProc = WriteLogProc;
  data.iReconnCount = 1;
  DialogBoxParam(GetModuleHandle(0),
    (LPCWSTR)IDD_SHOUTRIPPER,
    0,
    ShoutDlgProc,
    (LPARAM)&data);
  CloseHandle(data.hRunning);
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//Menu
int Shoutcast()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Ripper"
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      ShoutRipp
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

