#include "common.h"
#include "resource.h"

#define MAX_DESKTOP_NAME          32


static LPWSTR szRegKey = L"Software\\ShahSoft\\MultiDesktop\\Desktops";
static LPCWSTR szErrCreate = L"Cannot create new desktop.\nMaybe too many desktops has already been created.";
static LPCWSTR szWinlogon = L"Winlogon";
static LPCWSTR szErrTitle = L"Error creating new desktop";
static LPCWSTR szHotFmt   = L"&%d. ";

static HDESK CreateNewDesktop(HWND hEdit, LPWSTR szName)
{
  HDESK hDesk;
  WCHAR szNameBuff[MAX_DESKTOP_NAME];
  WCHAR szExplorer[16];
  PROCESS_INFORMATION pi;
  STARTUPINFO si = {sizeof(STARTUPINFO)};
  lstrcpy(szExplorer, L"explorer.exe");
  if(!szName)
  {
    szName = szNameBuff;
    GetWindowText(hEdit, (LPTSTR)szName, MAX_DESKTOP_NAME);
  }
  hDesk = CreateDesktop(szName, 0, 0, 0, MAXIMUM_ALLOWED, 0);
  if(!hDesk)
  {
    MessageBox(
      g_hMainWnd,
      szErrCreate,
      szErrTitle,
      MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL
      );
    return 0;
  }
  GetStartupInfo(&si);
  si.lpDesktop = szName;
  if(CreateProcess(0, szExplorer, 0, 0, 0, 0, 0, 0, &si, &pi))
  {
    WaitForInputIdle(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return hDesk;
}

struct FILL_ENUM_PARAM
{
  HMENU hMenu;
  UINT uCmd;
  PWSTR szCurrDesk;
};

static BOOL CALLBACK MenuFillProc(
                                  LPWSTR szName,
                                  struct FILL_ENUM_PARAM* pParam
                                  )
{
  UINT uFlags;
  WCHAR wch;
  if(!*szName)
    return FALSE;
  uFlags = lstrcmpi(szName, szWinlogon) ? MF_STRING : MF_STRING | MF_GRAYED;
  if(!lstrcmpi(pParam->szCurrDesk, szName))
    uFlags |= (MF_CHECKED | MF_DISABLED);
  wch = *szName;
  wsprintf(szName - 4, szHotFmt, pParam->uCmd);
  *szName = wch;
  AppendMenu(pParam->hMenu, uFlags, pParam->uCmd ++, szName - 4);
  return TRUE;
}

static int DesktopSwitch()
{
  WCHAR szName[MAX_DESKTOP_NAME + 4];
  HDESK hDesk;
  HWINSTA hWinSta = GetProcessWindowStation();
  struct FILL_ENUM_PARAM Param;
  Param.hMenu = CreatePopupMenu();
  if(!Param.hMenu)
    return 0;
  Param.uCmd = 1;
  Param.szCurrDesk = szName + 4;
  *szName = 0;
  hDesk = OpenInputDesktop(0, FALSE, DESKTOP_ENUMERATE);
  if(hDesk)
  {
    GetUserObjectInformation(hDesk, UOI_NAME, szName, sizeof(szName), 0);
    CloseDesktop(hDesk);
  }
  EnumDesktops(hWinSta, (DESKTOPENUMPROC)MenuFillProc, (LPARAM)&Param);
  Param.uCmd = PopupTextMenu(Param.hMenu);
  *szName = 0;
  GetMenuString(Param.hMenu, Param.uCmd, szName, MAX_DESKTOP_NAME, MF_BYCOMMAND);
  DestroyMenu(Param.hMenu);
  hDesk = OpenDesktop(Param.szCurrDesk, 0, 0, DESKTOP_SWITCHDESKTOP);
  if(hDesk)
  {
    SwitchDesktop(hDesk);
    CloseDesktop(hDesk);
  }
  return 0;
}

static int DesktopLoad()
{
  HKEY hKey;
  WCHAR szName[MAX_DESKTOP_NAME];
  DWORD dwIndex = 0;
  HDESK hDesk;
  if(
    ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, szRegKey, &hKey)
    )
    return 0;
  while(
    ERROR_SUCCESS ==
    RegEnumKey(hKey, dwIndex, szName, sizeof(szName))
    )
  {
    hDesk = OpenDesktop(szName, 0, 0, 0);
    if(!hDesk && lstrcmpi(szName, szWinlogon))
      hDesk = CreateNewDesktop(0, szName);
    CloseDesktop(hDesk);
    dwIndex ++;
  }
  RegCloseKey(hKey);
  return 0;
}

static void CheckName(HWND hEdit, HWND hOk)
{
  TCHAR szName[MAX_DESKTOP_NAME];
  HDESK hDesk;
  GetWindowText(hEdit, szName, MAX_DESKTOP_NAME);
  hDesk = OpenDesktop(szName, 0, FALSE, DESKTOP_ENUMERATE);
  EnableWindow(hOk, !hDesk && *szName);
  if(hDesk)
  {
    CloseDesktop(hDesk);
  }
}

static INT_PTR CALLBACK NewDesktopDlgProc(
                                      HWND hWnd,
                                      UINT msg,
                                      WPARAM wParam,
                                      LPARAM lParam
                                      )
{
  switch(msg)
  {
  case WM_INITDIALOG:
    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
    SetFocus(GetDlgItem(hWnd, IDC_EDIT_NEWDESKTOP));
    Edit_LimitText(
      GetDlgItem(hWnd, IDC_EDIT_NEWDESKTOP),
      MAX_DESKTOP_NAME - 1
      );
    break;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_EDIT_NEWDESKTOP:
      CheckName((HWND)lParam, GetDlgItem(hWnd, IDOK));
      break;
    case IDOK:
      CreateNewDesktop(GetDlgItem(hWnd, IDC_EDIT_NEWDESKTOP), 0);
    case IDCANCEL:
      EndDialog(hWnd, 0);
      break;
    }
    break;
  }
  return 0;
}

static int DesktopCreate()
{
  DialogBox(
    GetModuleHandle(0),
    (LPCTSTR)IDD_NEWDESKTOP,
    g_hMainWnd,
    NewDesktopDlgProc
    );
  return 0;
}

static BOOL CALLBACK SaveProc(LPCWSTR szName, HKEY hKey)
{
  HKEY hSubKey;
  if(ERROR_SUCCESS == RegCreateKey(hKey, szName, &hSubKey))
    RegCloseKey(hSubKey);
  return TRUE;
}

static int DesktopSave()
{
  HKEY hKey;
  HWINSTA hWinSta;

  if(ERROR_SUCCESS != RegCreateKey(HKEY_CURRENT_USER, szRegKey, &hKey))
    return 0;
  hWinSta = GetProcessWindowStation();
  EnumDesktops(hWinSta, (DESKTOPENUMPROC)SaveProc, (LPARAM)hKey);
  RegCloseKey(hKey);
  return 0;
}

static int DesktopDelete()
{
  HKEY hKey;
  WCHAR szName[MAX_DESKTOP_NAME];
  if(ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, szRegKey, &hKey))
    return 0;
  while(ERROR_SUCCESS == RegEnumKey(hKey, 0, szName, sizeof(szName)))
    RegDeleteKey(hKey, szName);
  RegCloseKey(hKey);
  return 0;
}

int DesktopList()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Switch desktop...",
      0,
      L"&2. Create new desktop...",
      L"&3. Load from registry",
      L"&4. Save to registry",
      L"&5. Delete from registry"
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      DesktopSwitch,
      0,
      DesktopCreate,
      DesktopLoad,
      DesktopSave,
      DesktopDelete
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

