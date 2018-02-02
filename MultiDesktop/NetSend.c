#include "common.h"
//#include <expdisp.h>
#include "resource.h"

#define MAX_COMPUTERNAME          32
#define MAX_MSG_LEN               256


static LPCWSTR szRegNetSendUsers = L"Software\\ShahSoft\\MultiDesktop\\NetSend\\Users";

#ifdef __GNUC__

//const static IID IID_IShellWindows = {0x85CB6900, 0x4D95, 0x11CF, 0x960C 0x0080C7F4, 0xEE85};
//const static CLSID CLSID_ShellWindows = {0x9BA05972, 0xF6A8, 0x11CF, 0xA442, 0x00A0C90A, 0x8F39};
//const static IID_IWebBrowser2 = {0xD30C1661, 0xCDAF, 0x11d0, 0x8A3E, 0x00C04FC9E26E};
//const static IID_IHTMLDocument2 = {0x332c4425, 0x26cb, 0x11d0, 0xb483, 0x00c04fd90119};

#endif //__GNUC__

struct UPD_CTRL_STRCT
{
  BOOL bIsChecked;
  BOOL bIsNameFound;
  WCHAR szName[MAX_COMPUTERNAME];
};

struct SEND_DATA
{
  HWND hWnd;
  WCHAR szText[MAX_MSG_LEN];
};

struct REMOVE_DATA
{
  HKEY hKey;
  BOOL bContinue;
};

static BOOL TreeCheckProc(
                          HWND hTree,
                          LPTVITEM pItem,
                          struct UPD_CTRL_STRCT* pUpdData
                          )
{
  hTree = 0;
  if((pItem->state >> 12)-1)
    pUpdData->bIsChecked = TRUE;
  if(!lstrcmpi(pUpdData->szName, pItem->pszText))
    pUpdData->bIsNameFound = TRUE;
  return !(pUpdData->bIsChecked && pUpdData->bIsNameFound);
}

static void UpdateControls(HWND hWnd)
{
  struct UPD_CTRL_STRCT UpdData = {0};
  HWND hTree = GetDlgItem(hWnd, IDC_TREE_USERS);
  HWND hTxtSend = GetDlgItem(hWnd, IDC_EDIT_MESSAGE);
  HWND hTxtName = GetDlgItem(hWnd, IDC_EDIT_USERNAME);
  HWND hBtnSend = GetDlgItem(hWnd, IDC_BTN_SEND);
  HWND hBtnAdd = GetDlgItem(hWnd, IDC_BTN_ADDUSER);
  HWND hBtnRemove = GetDlgItem(hWnd, IDC_BTN_REMOVEUSER);
  GetWindowText(hTxtName, UpdData.szName, MAX_COMPUTERNAME);
  EnumTreeItems(hTree, (TREE_ITEM_ENUM_PROC)TreeCheckProc, &UpdData);
  EnableWindow(hBtnRemove, UpdData.bIsChecked);
  EnableWindow(
    hBtnSend,
    UpdData.bIsChecked && GetWindowTextLength(hTxtSend)
    );
  EnableWindow(
    hBtnAdd,
    (!UpdData.bIsNameFound) && GetWindowTextLength(hTxtName)
    );
}

static void FillTree(HWND hWnd)
{
  HWND hTree = GetDlgItem(hWnd, IDC_TREE_USERS);
  HKEY hKey;
  TCHAR szName[MAX_COMPUTERNAME];
  DWORD dwIndex = 0;
  TVINSERTSTRUCT tvis = {0, TVI_LAST};
  tvis.item.mask = TVIF_TEXT;
  tvis.item.pszText = szName;

  if(
    ERROR_SUCCESS !=
    RegOpenKey(HKEY_CURRENT_USER, szRegNetSendUsers, &hKey)
    )
    return;
  while(
    ERROR_SUCCESS ==
    RegEnumKey(hKey, dwIndex, szName, sizeof(szName) / sizeof(szName[0]))
    )
  {
    TreeView_InsertItem(hTree, &tvis);
    dwIndex ++;
  }
  RegCloseKey(hKey);
}

static void SetEditLimits(HWND hWnd)
{
  Edit_LimitText(GetDlgItem(hWnd, IDC_EDIT_MESSAGE), MAX_MSG_LEN - 1);
  Edit_LimitText(GetDlgItem(hWnd, IDC_EDIT_USERNAME), MAX_COMPUTERNAME - 1);
}

static BOOL SendProc(HWND hTree, LPTVITEM pItem, struct SEND_DATA* pData)
{
  static LPCWSTR szRepFmt = L"Sending... %s";
  static LPCWSTR szErrFmt = L"Cannot send the message to %s\nDo you want to continue?";
  static LPCWSTR szErrTitle = L"Error sending message";
  WCHAR szReport[MAX_PATH];
  hTree = 0;
  if(!((pItem->state >> 12)-1))
    return TRUE;
  wsprintf(szReport, szRepFmt, pItem->pszText);
  SetWindowText(pData->hWnd, szReport);
  if(NERR_Success != NetMessageBufferSend(
    0,
    pItem->pszText,
    0,
    (PBYTE)pData->szText,
    sizeof(WCHAR) * lstrlen(pData->szText)
    ))
  {
    wsprintf(szReport, szErrFmt, pItem->pszText);
    if(
      IDCANCEL ==
      MessageBox(
      pData->hWnd,
      szReport,
      szErrTitle,
      MB_OKCANCEL | MB_ICONEXCLAMATION
      )
      )
      return FALSE;
  }
  return TRUE;
}

static void Send(HWND hWnd)
{
  WCHAR szTitle[MAX_COMPUTERNAME];
  struct SEND_DATA Data;
  Data.hWnd = hWnd;
  GetDlgItemText(hWnd, IDC_EDIT_MESSAGE, Data.szText, MAX_MSG_LEN);
  GetWindowText(hWnd, szTitle, MAX_COMPUTERNAME);
  EnumTreeItems(GetDlgItem(hWnd, IDC_TREE_USERS), (TREE_ITEM_ENUM_PROC)SendProc, &Data);
  SetWindowText(hWnd, szTitle);
}

static BOOL RemoveProc(
                       HWND hTree,
                       LPTVITEM pItem,
                       struct REMOVE_DATA* pData
                       )
{
  if((pItem->state >> 12)-1)
  {
    TreeView_DeleteItem(hTree, pItem->hItem);
    RegDeleteKey(pData->hKey, pItem->pszText);
    pData->bContinue = TRUE;
    return FALSE;
  }
  return TRUE;
}

static void AddUser(HWND hWnd, HWND hTree, HKEY hKey)
{
  WCHAR szName[MAX_COMPUTERNAME];
  HKEY hSubKey;
  TVINSERTSTRUCT tvis = {0, TVI_LAST};
  tvis.item.mask = TVIF_TEXT;
  tvis.item.pszText = szName;
  GetDlgItemText(hWnd, IDC_EDIT_USERNAME, szName, MAX_COMPUTERNAME);
  TreeView_InsertItem(hTree, &tvis);
  if(
    ERROR_SUCCESS != RegCreateKey(hKey, szName, &hSubKey)
    )
    return;
  RegCloseKey(hSubKey);
}

static void MngUser(HWND hWnd, WORD wBtnId)
{
  struct REMOVE_DATA Data = {0, TRUE};
  HWND hTree;
  if(
    ERROR_SUCCESS !=
    RegOpenKey(HKEY_CURRENT_USER, szRegNetSendUsers, &Data.hKey)
    )
    return;
  hTree = GetDlgItem(hWnd, IDC_TREE_USERS);
  if(wBtnId == IDC_BTN_ADDUSER)
  {
    AddUser(hWnd, hTree, Data.hKey);
    RegCloseKey(Data.hKey);
    return;
  }
  while(Data.bContinue)
  {
    Data.bContinue = FALSE;
    EnumTreeItems(hTree, (TREE_ITEM_ENUM_PROC)RemoveProc, &Data);
  }
  RegCloseKey(Data.hKey);
}

static INT_PTR CALLBACK DlgProc(
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
    SetEditLimits(hWnd);
    FillTree(hWnd);
    if(lParam)
      SetDlgItemText(hWnd, IDC_EDIT_MESSAGE, (LPCWSTR)lParam);
    break;
  case WM_NOTIFY:
    UpdateControls(hWnd);
    switch(((LPNMHDR)lParam)->code)
    {
    case TVN_SELCHANGED:
      ((LPNMTREEVIEW)lParam)->itemNew.lParam;
      break;
    }
    break;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_EDIT_USERNAME:
    case IDC_EDIT_MESSAGE:
      UpdateControls(hWnd);
      break;
    case IDC_BTN_SEND:
      Send(hWnd);
      break;
    case IDC_BTN_ADDUSER:
    case IDC_BTN_REMOVEUSER:
      MngUser(hWnd, LOWORD(wParam));
      break;
    case IDCANCEL:
      EndDialog(hWnd, 0);
      break;
    }
    break;
  default:
    break;
  }
  return 0;
}

static int UserList()
{
  DialogBox(GetModuleHandle(0), (LPCTSTR)IDD_NETSEND, g_hMainWnd, DlgProc);
  return 0;
}

static HWND MsngrGetStat()
{
  static LPCWSTR szMsngrWnd = L"Messenger Service ";
  HWND hWnd = FindWindow(0, szMsngrWnd);
  if(hWnd)
  {
    hWnd = GetWindow(hWnd, GW_CHILD);
    hWnd = GetWindow(hWnd, GW_HWNDNEXT);
  }
  return hWnd;
}

static int CopyText()
{
  HWND hWnd = MsngrGetStat();
  if(!hWnd)
    return 0;
  WndTxtToClipboard(hWnd);
  return 0;
}

static LPCSTR szLinkFileName = "Links.html";

static BOOL MsngrDoUrl(BOOL bOpen)
{
  static LPCSTR szFmtLink = "<a href=\"%s\">\"%s\"</a><br>\n";
  DWORD  dwSize;
  LPSTR szBuff, p, p1, p2;
  FILE* logFile;
  static LPCSTR szHttp = "http://";
  HWND hStat = MsngrGetStat();
  if(!hStat)
    return FALSE;
  dwSize = GetWindowTextLengthA(hStat) + 1;
  szBuff = (LPSTR)_alloca(dwSize);
  GetWindowTextA(hStat, szBuff, dwSize);
  for(p = szBuff; *p; p ++)
  {
    p1 = (LPSTR)szHttp;
    p2 = p;
    while(*p1 && *p && *p1 == *p)
    {
      p1 ++;
      p ++;
    }
    if(!*p1)
    {
      if(bOpen)
      {
        while(*p)
        {
          if(
            *p == '\r' ||
            *p == '\n' ||
            *p == ' ' ||
            *p == '\t'
            )
          {
            *p = 0;
            break;
          }
          p ++;
        }
        p = p2;
        p --;
        for(;
          *p == '\r' || *p == '\n' || *p == ' ' || *p == '\t';
          *p = 0, p --
          );
        logFile = fopen(szLinkFileName, "at");
        if(logFile)
        {
          fprintf(logFile, szFmtLink, p2, szBuff);
          fclose(logFile);
        }
        ShellExecuteA(GetDesktopWindow(), "open", p2, 0, 0, SW_NORMAL);
      }
      return TRUE;
    }
  }
  return FALSE;
}

static int OpenUrl()
{
  MsngrDoUrl(TRUE);
  return 0;
}

static int OpenUrlHistory()
{
  static LPCSTR szOpen = "open";
  ShellExecuteA(g_hMainWnd, szOpen, szLinkFileName, 0, 0, SW_SHOW);
  return 0;
}

static int CopyUrl()
{
  static LPCWSTR szNoUrlTtl = L"Nothing to send";
  static LPCWSTR szNoUrl = L"Make sure that something is open in Internet Explorer";
  HRESULT hRes;
  IShellWindows* pIShellWnds = 0;
  IDispatch* pDispBrowser = 0;
  IWebBrowser2 *pBrowser = 0;
  IDispatch* pDispDoc = 0;
  IHTMLDocument2* pDoc = 0;
  long lCount = 0;
  VARIANT idx = {VT_I4};
  BSTR szUrl = 0;
  BSTR szTitle = 0;
  BOOL bDoTrack = FALSE;
  MENUITEMINFO ItemInfo = {sizeof(MENUITEMINFO)};
  HMENU hMenu = CreatePopupMenu();
  if(!hMenu)
    return 0;

  CoInitialize(0);
  hRes =
    CoCreateInstance(
    &CLSID_ShellWindows,
    0,
    CLSCTX_LOCAL_SERVER,
    &IID_IShellWindows,
    (LPVOID*)&pIShellWnds);
  if(S_OK == hRes)
  {
    pIShellWnds->lpVtbl->get_Count(pIShellWnds, &lCount);
    while(lCount --)
    {
      idx.lVal = lCount;
      hRes = pIShellWnds->lpVtbl->Item(pIShellWnds, idx, &pDispBrowser);
      if(hRes == S_OK)
      {
        hRes =
          pDispBrowser->lpVtbl->QueryInterface(
          pDispBrowser,
          &IID_IWebBrowser2,
          (LPVOID*)&pBrowser);
        pDispBrowser->lpVtbl->Release(pDispBrowser);
        if(hRes == S_OK)
        {
          hRes = pBrowser->lpVtbl->get_Document(pBrowser, &pDispDoc);
          pBrowser->lpVtbl->Release(pBrowser);
          if(hRes == S_OK)
          {
            hRes =
              pDispDoc->lpVtbl->QueryInterface(
              pDispDoc,
              &IID_IHTMLDocument2,
              &pDoc);
            pDispDoc->lpVtbl->Release(pDispDoc);
            if(hRes == S_OK)
            {
              szUrl = szTitle = 0;
              pDoc->lpVtbl->get_title(pDoc, &szTitle);
              if(szTitle)
              {
                bDoTrack = TRUE;
                pDoc->lpVtbl->get_URL(pDoc, &szUrl);
                if(szUrl)
                  AppendMenu(hMenu, MF_STRING, (UINT)szUrl, szTitle);
                SysFreeString(szTitle);
              }
              pDoc->lpVtbl->Release(pDoc);
            }
          }
        }
      }
    }
    pIShellWnds->lpVtbl->Release(pIShellWnds);
  }
  CoUninitialize();
  if(bDoTrack)
  {
    szUrl = (BSTR)PopupTextMenu(hMenu);
    if(szUrl)
      DialogBoxParam(
      GetModuleHandle(0),
      (LPCTSTR)IDD_NETSEND,
      g_hMainWnd,
      DlgProc,
      (LPARAM)szUrl
      );

    lCount = GetMenuItemCount(hMenu);
    while(lCount --)
    {
      GetMenuItemInfo(hMenu, (UINT)lCount, TRUE, &ItemInfo);
      SysFreeString((BSTR)ItemInfo.wID);
    }
  }
  else
    MessageBox(g_hMainWnd, szNoUrl, szNoUrlTtl, MB_OK | MB_ICONINFORMATION);
  DestroyMenu(hMenu);
  return 0;
}

int NetSend()
{
  DWORD dwAttr;
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Send / User list...",
      0,
      L"&2. Copy messenger text",
      L"&3. Open URL",
      L"&4. Copy URL From IE...",
      0,
      L"&5. Open URL history"
  };

  static UINT uFlags[] =
  {
    MF_SEPARATOR,
      MF_STRING,
      MF_SEPARATOR,
      MF_STRING,
      MF_STRING,
      MF_STRING,
      MF_SEPARATOR,
      MF_STRING
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      UserList,
      0,
      CopyText,
      OpenUrl,
      CopyUrl,
      0,
      OpenUrlHistory
  };

  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , uFlags
      , DispatchTable
  };

  uFlags[3] |= MsngrGetStat() ? 0 : MF_GRAYED;
  uFlags[4] |= MsngrDoUrl(FALSE) ? 0 : MF_GRAYED;

  GotoDataFolder();
  dwAttr = GetFileAttributesA(szLinkFileName);
  if(dwAttr == (DWORD)-1 || dwAttr == FILE_ATTRIBUTE_DIRECTORY)
    uFlags[6] |= MF_GRAYED;

  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();
  return 0;
}
