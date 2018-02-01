#include "rt.h"

#define TVM_SETLINECOLOR            (TV_FIRST + 40)
#define TreeView_SetLineColor(hwnd, clr) (COLORREF)SendMessage((hwnd), TVM_SETLINECOLOR, 0, (LPARAM)(clr))

#define TVM_GETLINECOLOR            (TV_FIRST + 41)
#define TreeView_GetLineColor(hwnd) (COLORREF)SendMessage((hwnd), TVM_GETLINECOLOR, 0, 0)


enum
{
  F_TV_EMPTY = 0
  , F_TV_SCANROOT
  , F_TV_SCANPATH
  , F_TV_SFDF_GROUP
  , F_TV_SFFD_GROUP
  , F_TV_SFFD_FILE
  , F_TV_SFDF_FILE
  , F_TV_SFFD_ROOT
  , F_TV_OPENSELECT_ABSENT
  , F_TV_OPENSELECT_BIGGER
  , F_TV_OPENSELECT_NEWER
  , F_TV_STARTSFDFSCAN
  , F_TV_SFDF_RES
  , F_TV_STARTSFFDSCAN
  , F_TV_LOGS
  , F_TV_LAST = 15
};

struct TVI_CLASS
{
  void (*fTvDblClick)(HTREEITEM);
  void (*fTvKbdEnter)(HTREEITEM);
};

static void fTvEmpty(HTREEITEM hi);
static void fTvScanRoot();
static void fTvScanPath(HTREEITEM hi);
static void fTvSfdfGroup(HTREEITEM hi);
static void fTvSffdGroup(HTREEITEM hi);
static void fTvSffdFile(HTREEITEM hi);
static void fTvSfdfFile(HTREEITEM hi);
static void fTvSffdRoot(HTREEITEM hi);
static void fTvOpenSelectAbsent(HTREEITEM hi);
static void fTvOpenSelectBigger(HTREEITEM hi);
static void fTvOpenSelectNewer(HTREEITEM hi);
static void fTvStartSfdfScan();
static void fTvSfdfRes(HTREEITEM hi);
static void fTvStartSffdScan();
static void fTvLogs();

static struct TVI_CLASS g_EvtTbl[F_TV_LAST];

static struct TV_GLB g_glb;

static UINT TvMsgBox(PCWSTR szText, PCWSTR szTitle, UINT uType)
{
  return MessageBox(g_glb.hTree, szText, szTitle, uType);
}

static HTREEITEM TvAddChildItemByName(HTREEITEM hi, PCWSTR szName, LPARAM lTviData)
{
  TVINSERTSTRUCT tvis = {0};

  tvis.item.mask    = TVIF_PARAM | TVIF_TEXT;
  tvis.item.pszText = (PWSTR)szName;
  tvis.item.lParam  = lTviData;
  tvis.hParent      = hi;
  return TreeView_InsertItem(g_glb.hTree, &tvis);
}

static void TvAddLogLine(PCWSTR szFmt, ...)
{
  HTREEITEM hLineItem;
  WCHAR wchBuff[TXT_BUFF_SIZE];
  va_list pArgList;
  va_start(pArgList, szFmt);
  wvsprintf(wchBuff, szFmt, pArgList);
  va_end(pArgList);
  hLineItem = TvAddChildItemByName(g_glb.hScanLog, wchBuff, F_TV_EMPTY);
  TreeView_SelectItem(g_glb.hTree, hLineItem);
}

static UINT TvMsgBoxAndLog(PCWSTR szText, PCWSTR szTitle, UINT uType)
{
  TvAddLogLine(g_str.pTbl->szNoInputLog, szTitle, szText);
  return TvMsgBox(szText, szTitle, uType);
}

static void TvInitEventTable()
{
  g_glb.pEvtTbl = g_EvtTbl;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_EMPTY].fTvDblClick                 = fTvEmpty;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_EMPTY].fTvKbdEnter                 = fTvEmpty;
  
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SCANROOT].fTvDblClick              = fTvScanRoot;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SCANROOT].fTvKbdEnter              = fTvScanRoot;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SCANPATH].fTvDblClick              = fTvScanPath;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SCANPATH].fTvKbdEnter              = fTvScanPath;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_GROUP].fTvDblClick            = fTvSfdfGroup;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_GROUP].fTvKbdEnter            = fTvSfdfGroup;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_GROUP].fTvDblClick            = fTvSffdGroup;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_GROUP].fTvKbdEnter            = fTvSffdGroup;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_FILE].fTvDblClick             = fTvSffdFile;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_FILE].fTvKbdEnter             = fTvSffdFile;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_FILE].fTvDblClick             = fTvSfdfFile;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_FILE].fTvKbdEnter             = fTvSfdfFile;
  
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_ROOT].fTvDblClick             = fTvSffdRoot;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFFD_ROOT].fTvKbdEnter             = fTvSffdRoot;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_ABSENT].fTvDblClick     = fTvOpenSelectAbsent;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_ABSENT].fTvKbdEnter     = fTvOpenSelectAbsent;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_BIGGER].fTvDblClick     = fTvOpenSelectBigger;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_BIGGER].fTvKbdEnter     = fTvOpenSelectBigger;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_NEWER].fTvDblClick      = fTvOpenSelectNewer;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_OPENSELECT_NEWER].fTvKbdEnter      = fTvOpenSelectNewer;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_STARTSFDFSCAN].fTvDblClick         = fTvStartSfdfScan;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_STARTSFDFSCAN].fTvKbdEnter         = fTvStartSfdfScan;
  
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_RES].fTvDblClick              = fTvSfdfRes;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_SFDF_RES].fTvKbdEnter              = fTvSfdfRes;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_STARTSFFDSCAN].fTvDblClick         = fTvStartSffdScan;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_STARTSFFDSCAN].fTvKbdEnter         = fTvStartSffdScan;

  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_LOGS].fTvDblClick                  = fTvLogs;
  ((struct TVI_CLASS*)g_glb.pEvtTbl)[F_TV_LOGS].fTvKbdEnter                  = fTvLogs;
}

static void TvSetState(enum TV_STATE st)
{
  g_glb.st = st;
  PCWSTR szTreeTitle;
  switch(st)
  {
    case TVS_IDLE:
      szTreeTitle = g_str.pTbl->szTitle;
      break;
    case TVS_SCANNING:
      szTreeTitle = g_str.pTbl->szScanning;
      break;
    case TVS_CANCEL:
      szTreeTitle = g_str.pTbl->szCancelled;
      break;
    default:
      return;
  }
  SetWindowText(g_glb.hTree, szTreeTitle);
}

static PWSTR TvGetItemText(HTREEITEM hi, WCHAR wchBuff[TXT_BUFF_SIZE])
{
  TVITEM tvi = {TVIF_TEXT};
  tvi.pszText = wchBuff;
  tvi.cchTextMax = TXT_BUFF_SIZE;
  tvi.hItem = hi;
  TreeView_GetItem(g_glb.hTree, &tvi);
  return wchBuff;
}

static HTREEITEM TvFindChildItemByName(HTREEITEM hi, PCWSTR szName)
{
  WCHAR wchName[TXT_BUFF_SIZE];
  hi = TreeView_GetChild(g_glb.hTree, hi);
  while(hi)
  {
    TvGetItemText(hi, wchName);
    if(!lstrcmpW(wchName, szName))
      break;
    hi = TreeView_GetNextSibling(g_glb.hTree, hi);
  }
  return hi;
}

static HTREEITEM TvFindChildItemByNameOrAdd(HTREEITEM hi, PCWSTR szName, LPARAM lTvItemData, int iExpand)
{
  HTREEITEM hci = TvFindChildItemByName(hi, szName);
  if(!hci)
  {
    hci = TvAddChildItemByName(hi, szName, lTvItemData);
    if(iExpand)
      TreeView_Expand(g_glb.hTree, hi, TVE_EXPAND);
  }
  return hci;
}

static void TvDeleteChildren(HTREEITEM hItem)
{
  HTREEITEM hChild;
  while((hChild = TreeView_GetChild(g_glb.hTree, hItem)))
    TreeView_DeleteItem(g_glb.hTree, hChild);
}

static void TvAddScanPath(PWSTR szRoot)
{
  g_fs.FsNormalizePath(szRoot);
  HTREEITEM hti = g_glb.hScanRoot;
  PWSTR p = g_fs.FsSkipNetworkBacklashes(szRoot);
  PCWSTR szPath = p;
  TvAddLogLine(g_str.pTbl->szAdded, szRoot);
  while(*p)
  {
    if(*p == '\\')
    {
      *p = 0;
      hti = TvFindChildItemByNameOrAdd(hti, szPath, F_TV_SCANPATH, 1);
      *p = L'\\';
      szPath = p + 1;
    }
    p ++;
  }
  if(*szPath)
    hti = TvFindChildItemByNameOrAdd(hti, szPath, F_TV_SCANPATH, 1);
  TvDeleteChildren(hti);
}

static void TvAddPathComplex(HTREEITEM hRoot, PCWSTR szPath, PWSTR szRelPath, LPARAM lParamBase, LPARAM lParamLeave)
{
  WCHAR wch = *szRelPath;
  *szRelPath = 0;
  HTREEITEM hti = TvFindChildItemByNameOrAdd(hRoot, szPath, lParamBase, 0);
  *szRelPath = wch;
  PWSTR p = szRelPath;
  PCWSTR p1 = p;
  while(*p)
  {
    if(*p == '\\')
    {
      *p = 0;
      hti = TvFindChildItemByNameOrAdd(hti, p1, lParamLeave, 0);
      *p = L'\\';
      p1 = p + 1;
    }
    p ++;
  }
  if(*szPath && *p1)
    hti = TvFindChildItemByNameOrAdd(hti, p1, lParamLeave, 0);
}

static int TvInProgressWarning()
{
  if(g_glb.st == TVS_IDLE)
    return 0;
  SetWindowText(g_glb.hTree, g_str.pTbl->szPause);
  if(IDYES == TvMsgBox(g_str.pTbl->szWaitScanEnd, g_str.pTbl->szScanInProg, MB_OK | MB_YESNO | MB_ICONQUESTION))
  {
    TvSetState(TVS_CANCEL);
    TvAddLogLine(g_str.pTbl->szCancelled);
  }
  else
    TvSetState(g_glb.st);
  return 1;
}

static void TvExpandInput()
{
  TreeView_Expand(g_glb.hTree, TreeView_GetRoot(g_glb.hTree), TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hScanRoot, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hScanLog, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hSfdfStartScan, TVE_COLLAPSE);
  TreeView_Expand(g_glb.hTree, g_glb.hSffdStartScan, TVE_COLLAPSE);
}

static void TvFileDrop(HDROP hDrop)
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  if(TvInProgressWarning())
    return;
  UINT uCount = DragQueryFile(hDrop, (UINT)-1, 0, 0);
  while(uCount --)
  {
    DragQueryFile(hDrop, uCount, wchPath, TXT_BUFF_SIZE);
    TvAddScanPath(wchPath);
  }
  DragFinish(hDrop);
  TvExpandInput();
}

static PWSTR TvGetItemFullPath(HTREEITEM hRoot, HTREEITEM hi, PWSTR szBuff)
{
  HTREEITEM htiStack[MAX_PATH];
  HTREEITEM* phti = htiStack;
  PWSTR pPathIter = szBuff;
  if(!hRoot)
    hRoot = TreeView_GetParent(g_glb.hTree, hi);
  while(hi != hRoot)
  {
    *phti ++ = hi;
    hi = TreeView_GetParent(g_glb.hTree, hi);
  }
  while(htiStack != phti --)
  {
    TvGetItemText(*phti, pPathIter);
    while(*++pPathIter);
    if(*(pPathIter - 1) != L'\\')
      *pPathIter++ = L'\\';
  }
  pPathIter --;
  *pPathIter = 0;
  if(GetFileAttributes(szBuff) & FILE_ATTRIBUTE_DIRECTORY)
    *pPathIter++ = L'\\';
  *pPathIter = 0;
  return pPathIter;
}

static void TvOpenPathInExplorer(HTREEITEM hRoot, HTREEITEM hi)
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  PWSTR p = wchPath;
  p += wsprintf(p, L"%s", L"/n, /select, \"");
  p = TvGetItemFullPath(hRoot, hi, p);
  *p++ = L'\"';
  *p = 0;
  ShellExecute(g_glb.hTree, L"open", L"explorer", wchPath, 0, SW_SHOWNORMAL);  
}

static void TvOpenFile(HTREEITEM hRoot, HTREEITEM hi)
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  TvGetItemFullPath(hRoot, hi, wchPath);
  ShellExecute(g_glb.hTree, L"open", wchPath, 0, 0, SW_SHOWNORMAL);
}

static int TvIsItemDeleteException(HTREEITEM hi)
{
  return
    hi == g_glb.hSffdNewRoots[0]
    || hi == g_glb.hSffdNewRoots[1]
    || hi == g_glb.hSffdBigRoots[0]
    || hi == g_glb.hSffdBigRoots[1]
    || hi == g_glb.hSffdAbsentRoots[0]
    || hi == g_glb.hSffdAbsentRoots[1];
}

static void TvAskAndDeleteFile(HTREEITEM hRoot, HTREEITEM hi)
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  SHFILEOPSTRUCT op;
  HTREEITEM hParent = TreeView_GetParent(g_glb.hTree, hi);
  PWSTR p = TvGetItemFullPath(hRoot, hi, wchPath);
  *(p + (*(p - 1) == L'\\' ? -1 : 1)) = 0;
//  if(IDYES != TvMsgBox(wchPath, szAskDelete, MB_YESNO | MB_ICONQUESTION))
//    return;
  op.hwnd = g_glb.hTree;
  op.wFunc = FO_DELETE;
  op.pFrom = wchPath;
  op.pTo = 0;
  op.fFlags = 0;
  op.fAnyOperationsAborted = 0;
  op.hNameMappings = 0;
  op.lpszProgressTitle = g_str.pTbl->szDeleting;
  if(!SHFileOperation(&op))
  {
    if(!op.fAnyOperationsAborted)
    {
      TreeView_DeleteItem(g_glb.hTree, hi);
      TvAddLogLine(g_str.pTbl->szDeleted, wchPath);
    }
  }
  else
  {
    if(GetFileAttributes(wchPath) != (DWORD)-1)
      TvMsgBoxAndLog(wchPath, g_str.pTbl->szErrDel, MB_OK | MB_ICONERROR);
    else
      TreeView_DeleteItem(g_glb.hTree, hi);
  }
  if(!TvIsItemDeleteException(hParent)
      && !TreeView_GetChild(g_glb.hTree, hParent)
      && !op.fAnyOperationsAborted)
    TreeView_DeleteItem(g_glb.hTree, hParent);
}

static void TvFilePropDlg(HTREEITEM hRoot, HTREEITEM hi)
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  SHELLEXECUTEINFO si = {sizeof(SHELLEXECUTEINFO), SEE_MASK_INVOKEIDLIST, g_glb.hTree, L"properties", wchPath};
  TvGetItemFullPath(hRoot, hi, wchPath);
  ShellExecuteEx(&si);
}

static HTREEITEM TvGetSffdFileRoot(HTREEITEM hi)
{
  do{
    hi = TreeView_GetParent(g_glb.hTree, hi);
  }while(hi != g_glb.hSffdNew && hi != g_glb.hSffdBig && hi != g_glb.hSffdAbsent);
  return hi;
}

struct TvFsScanLeavesSfdf
{
  HTREEITEM hi;
  PWSTR szPath;
};

static void TvFsScanLeavesSfdf(struct TvFsScanLeavesSfdf* psc)
{
  HTREEITEM hci = TreeView_GetChild(g_glb.hTree, psc->hi);
  if(!hci)
  {
    TvGetItemFullPath(g_glb.hScanRoot, psc->hi, psc->szPath);
    g_fs.FsCollectFileNames(psc->szPath);
    return;
  }
  while(hci)
  {
    psc->hi = hci;
    TvFsScanLeavesSfdf(psc);
    hci = TreeView_GetNextSibling(g_glb.hTree, hci);
  }
}

static void TvGetDiffDataAndLog(HTREEITEM h0, PWSTR szPathBuff0, HTREEITEM h1, PWSTR szPathBuff1)
{
  TvGetItemFullPath(g_glb.hScanRoot, h0, szPathBuff0);
  TvGetItemFullPath(g_glb.hScanRoot, h1, szPathBuff1);
  TvAddLogLine(g_str.pTbl->szCmpDirs, szPathBuff0, szPathBuff1);
}

static void TvAddSffdOutRoots(PCWSTR szRoot0, PCWSTR szRoot1)
{
  g_glb.hSffdNewRoots[0] = TvAddChildItemByName(g_glb.hSffdNew, szRoot0, F_TV_SFFD_GROUP);
  g_glb.hSffdNewRoots[1] = TvAddChildItemByName(g_glb.hSffdNew, szRoot1, F_TV_SFFD_GROUP);
  g_glb.hSffdBigRoots[0] = TvAddChildItemByName(g_glb.hSffdBig, szRoot0, F_TV_SFFD_GROUP);
  g_glb.hSffdBigRoots[1] = TvAddChildItemByName(g_glb.hSffdBig, szRoot1, F_TV_SFFD_GROUP);
  g_glb.hSffdAbsentRoots[0] = TvAddChildItemByName(g_glb.hSffdAbsent, szRoot0, F_TV_SFFD_GROUP);
  g_glb.hSffdAbsentRoots[1] = TvAddChildItemByName(g_glb.hSffdAbsent, szRoot1, F_TV_SFFD_GROUP);
}

static void TvFsScanLeaveSffd()
{
  WCHAR wchPath0[TXT_BUFF_SIZE];
  WCHAR wchPath1[TXT_BUFF_SIZE];
  HTREEITEM hl = 0;
  HTREEITEM hi = g_glb.hScanRoot;
  while(TreeView_GetChild(g_glb.hTree, hi))
  {
    hi = TreeView_GetChild(g_glb.hTree, hi);
    hl = TreeView_GetNextSibling(g_glb.hTree, hi);
    if(hl)
    {
      while(TreeView_GetChild(g_glb.hTree, hl))
        hl = TreeView_GetChild(g_glb.hTree, hl);
      while(TreeView_GetChild(g_glb.hTree, hi))
        hi = TreeView_GetChild(g_glb.hTree, hi);
    }
  }
  TvGetDiffDataAndLog(hi, wchPath0, hl, wchPath1);
  TvAddSffdOutRoots(wchPath0, wchPath1);
  g_fs.FsSffd(wchPath0, wchPath1);
  TvGetDiffDataAndLog(hi, wchPath0, hl, wchPath1);
  g_fs.FsSffd(wchPath1, wchPath0);
}

static void TvDblClick()
{
  POINT pt;
  TVHITTESTINFO ht = {{0}};
  TVITEM tvi = {TVIF_PARAM};
  GetCursorPos(&pt);
  ScreenToClient(g_glb.hTree, &pt);
  ht.pt = pt;
  tvi.hItem = TreeView_HitTest(g_glb.hTree, &ht);
  if(tvi.hItem && (ht.flags & TVHT_ONITEM) && !TvInProgressWarning())
  {
    TreeView_GetItem(g_glb.hTree, &tvi);
    ((struct TVI_CLASS*)g_glb.pEvtTbl)[tvi.lParam & F_TV_LAST].fTvDblClick(tvi.hItem);
  }
}

static void TvKbdEnter()
{
  TVITEM tvi = {TVIF_PARAM};
  tvi.hItem = TreeView_GetSelection(g_glb.hTree);
  if(tvi.hItem && !TvInProgressWarning())
  {
    TreeView_GetItem(g_glb.hTree, &tvi);
    ((struct TVI_CLASS*)g_glb.pEvtTbl)[tvi.lParam & F_TV_LAST].fTvKbdEnter(tvi.hItem);
  }
}

static LRESULT CALLBACK TvProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
  case WM_LBUTTONDBLCLK:
    TvDblClick();
    return 0;
  case WM_KEYDOWN:
    switch(wParam)
    {
    case VK_RETURN:
      TvKbdEnter();
      break;
    }
    break;
  case WM_DROPFILES:
    TvFileDrop((HDROP)wParam);
    break;
  case WM_CLOSE:
    if(TvInProgressWarning())
      return 0;
    PostQuitMessage(0);
    break;
  default:
    break;
  }
  return CallWindowProc(g_glb.pOldProc, hWnd, msg, wParam, lParam);
}

static void TvInitPermanentItems()
{
  HTREEITEM hDirOp;
  TVINSERTSTRUCT tvis = {0};
  tvis.item.mask = TVIF_PARAM | TVIF_TEXT;

//the root item - directory scan operations:
  tvis.item.pszText = (PWSTR)g_str.pTbl->szDirScanOp;
  tvis.item.lParam = F_TV_EMPTY;
  hDirOp = tvis.hParent = TreeView_InsertItem(g_glb.hTree, &tvis);

//root for the input directory/file list:
  tvis.item.pszText = (PWSTR)g_str.pTbl->szRootToScan;
  tvis.item.lParam = F_TV_SCANROOT;
  g_glb.hScanRoot = TreeView_InsertItem(g_glb.hTree, &tvis);

//start scan for duplicate files:
  tvis.item.pszText = (PWSTR)g_str.pTbl->szSfdf;
  tvis.item.lParam = F_TV_STARTSFDFSCAN;
  tvis.hParent = g_glb.hSfdfStartScan = TreeView_InsertItem(g_glb.hTree, &tvis);

//root for found different file lists:
  tvis.item.lParam = F_TV_SFDF_RES;
  tvis.item.pszText = (PWSTR)g_str.pTbl->szDiff;
  g_glb.hSfdfScanResult = TreeView_InsertItem(g_glb.hTree, &tvis);

//start scan for different files:
  tvis.hParent = hDirOp;
  tvis.item.pszText = (PWSTR)g_str.pTbl->szSffd;
  tvis.item.lParam = F_TV_STARTSFFDSCAN;
  tvis.hParent = g_glb.hSffdStartScan = TreeView_InsertItem(g_glb.hTree, &tvis);
  
//scan results - files modified later:
  tvis.item.lParam = F_TV_SFFD_ROOT;
  tvis.item.pszText = (PWSTR)g_str.pTbl->szNewer;
  g_glb.hSffdNew = TreeView_InsertItem(g_glb.hTree, &tvis);

//scan results - files that are bigger:
  tvis.item.pszText = (PWSTR)g_str.pTbl->szBigger;
  g_glb.hSffdBig = TreeView_InsertItem(g_glb.hTree, &tvis);

//scan results - files that are absent in other scan dir:
  tvis.item.pszText = (PWSTR)g_str.pTbl->szAbsent;
  g_glb.hSffdAbsent = TreeView_InsertItem(g_glb.hTree, &tvis);
  

//logs root:
  tvis.hParent = 0;
  tvis.item.pszText = (PWSTR)g_str.pTbl->szLogs;
  tvis.item.lParam = F_TV_LOGS;
  g_glb.hScanLog = TreeView_InsertItem(g_glb.hTree, &tvis);

  g_glb.hSffdNewRoots[0] = g_glb.hSffdBigRoots[0] = g_glb.hSffdAbsentRoots[0] = 0;
  g_glb.hSffdNewRoots[1] = g_glb.hSffdBigRoots[1] = g_glb.hSffdAbsentRoots[1] = 0;

  TreeView_Expand(g_glb.hTree, hDirOp, TVE_EXPAND);
}

static void TvInitFs();

static void TvInit()
{
  HICON hIcon;
  INITCOMMONCONTROLSEX icc =
  {
    sizeof(INITCOMMONCONTROLSEX),
      ICC_TREEVIEW_CLASSES
  };
  HINSTANCE hExeInst = GetModuleHandle(0);
  InitCommonControlsEx(&icc);
  hIcon = LoadIcon(hExeInst, (PCWSTR)1);
  TvInitEventTable();
  g_glb.st = TVS_IDLE;
  g_glb.hTree = CreateWindowEx(WS_EX_ACCEPTFILES
    , WC_TREEVIEWW
    , g_str.pTbl->szTitle
    , WS_OVERLAPPEDWINDOW | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , 0
    , 0
    , hExeInst
    , 0);
  TreeView_SetBkColor(g_glb.hTree, 0x00FFFFFF);
  TreeView_SetLineColor(g_glb.hTree, 0x00FF0000);
  SendMessage(g_glb.hTree, WM_SETICON, 0, (LPARAM)hIcon);
  g_glb.pOldProc = (WNDPROC)SetWindowLong(g_glb.hTree , GWL_WNDPROC, (LONG)TvProc);
  ShowWindow(g_glb.hTree, SW_SHOWNORMAL);
  UpdateWindow(g_glb.hTree);
  TvInitPermanentItems();
  TvInitFs();
  g_glb.hLastGroup = 0;
}

static void TvFree()
{
}

static void TvAddSfdfGroup(PCWSTR szPath)
{
  PCWSTR szExt = g_fs.FsGetExtension(szPath);
  if(!szExt)
    szExt = g_str.pTbl->szNoExt;
  HTREEITEM hExtItem = TvFindChildItemByNameOrAdd(g_glb.hSfdfScanResult, szExt, F_TV_SFDF_GROUP, 0);
  g_glb.hLastGroup = TvAddChildItemByName(hExtItem, szPath, F_TV_SFDF_GROUP);
}

static void TvAddSfdfItem(PCWSTR szPath)
{
  TvAddChildItemByName(g_glb.hLastGroup, szPath, F_TV_SFDF_FILE);
}

static void TvAddSffdNewer(PCWSTR szPath, PWSTR szRelPath)
{
  TvAddPathComplex(g_glb.hSffdNew, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvAddSffdBigger(PCWSTR szPath, PWSTR szRelPath)
{
  TvAddPathComplex(g_glb.hSffdBig, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvAddSffdAbsent(PCWSTR szPath, PWSTR szRelPath)
{
  TvAddPathComplex(g_glb.hSffdAbsent, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvSortLastGroup()
{
  TreeView_SortChildren(g_glb.hTree, g_glb.hLastGroup, 0);
}

static void TvProgressTitle()
{
  static DWORD dwCounter = 0;
  static WCHAR wchProg[] =
  {
    L'-', L'\\', L'|', L'/'
  };
  WCHAR wchBuff[64];
  wsprintf(wchBuff, g_str.pTbl->szProgress, wchProg[++ dwCounter & 3]);
  SetWindowText(g_glb.hTree, wchBuff);
}

static int TvCheckCancel()
{
  return g_glb.st == TVS_CANCEL;
}
///////////////////////////////////////////////////////////////////////////////
//Event Handlers
static void fTvEmpty(HTREEITEM hi)
{
}

static void fTvScanRoot()
{
  WCHAR wchPath[TXT_BUFF_SIZE];
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, 3, g_str.pTbl->szAddDirFile, 0, g_str.pTbl->szCleanAll))
  {
    case 1:
      if(g_fs.FsSelectDirAndFile(g_glb.hTree, g_str.pTbl->szAddDirFile, wchPath))
        TvAddScanPath(wchPath);
      break;
    case 3:
      TvDeleteChildren(g_glb.hScanRoot);
      break;
  }
}

static void fTvScanPath(HTREEITEM hi)
{
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, 5, g_str.pTbl->szOpen, g_str.pTbl->szOpenContainer, g_str.pTbl->szFileProp, 0, g_str.pTbl->szRemoveFromList))
  {
    case 1:
      TvOpenFile(g_glb.hScanRoot, hi);
      break;
    case 2:
      TvOpenPathInExplorer(g_glb.hScanRoot, hi);
      break;
    case 3:
      TvFilePropDlg(g_glb.hScanRoot, hi);
      break;
    case 5:
      TreeView_DeleteItem(g_glb.hTree, hi);
      break;
  }
}

static void fTvSfdfGroup(HTREEITEM hi)
{
  if(1 == g_mnu.MnuPopupMenu(g_glb.hTree, 1, g_str.pTbl->szRemoveGroup))
    TreeView_DeleteItem(g_glb.hTree, hi);
}

static int TvCopyToOther(HTREEITEM hi)
{
  int iResult = 0;
  HTREEITEM hi0, hi1;
  PWSTR p0, p1;
  WCHAR wchPathBuff0[TXT_BUFF_SIZE], wchPathBuff1[TXT_BUFF_SIZE];
  p0 = wchPathBuff0;
  p1 = wchPathBuff1;
  hi1 = hi0 = TreeView_GetParent(g_glb.hTree, hi);
  while(hi0 != g_glb.hSffdNew && hi0 != g_glb.hSffdBig && hi0 != g_glb.hSffdAbsent)
  {
    hi1 = hi0;
    hi0 = TreeView_GetParent(g_glb.hTree, hi0);
  }
  p1 = TvGetItemFullPath(0, hi1, wchPathBuff1);
  p0 = TvGetItemFullPath(TreeView_GetParent(g_glb.hTree, hi1), hi, wchPathBuff1);
  if(*(p0 - 1) == L'\\')
    *(p0 - 1) = 0;
  *++p0 = 0;//extra 0 needed for FsCopy
  //hi1 - source root, wchPathBuff1 - source full path, p1 - end of source root

  if(TreeView_GetNextSibling(g_glb.hTree, hi1))
    hi0 = TreeView_GetNextSibling(g_glb.hTree, hi1);
  else
    hi0 = TreeView_GetPrevSibling(g_glb.hTree, hi1);  
  p0 = TvGetItemFullPath(0, hi0, wchPathBuff0);
  //hi0 - dest. root, wchPathbuff0 - dest. root path, p0 - end of dest. root
  
  //append the rest of source path to the root of dest. path
  while(*p1) *p0 ++ = *p1 ++;
  *p0 ++ = 0;
  *p0 = 0;//extra 0 for FsCopy

  //wchPathBuff0 - destination full path, wchPathBuff1 - source full path
  iResult = g_fs.FsCopy(g_glb.hTree, wchPathBuff0, wchPathBuff1);
//  if(iResult)
//    TreeView_DeleteItem(g_glb.hTree, hi);
  return iResult;
}

static int TvCopyToOtherRecursive(HTREEITEM hi)
{
  int iResult = 1;
  HTREEITEM hci = TreeView_GetChild(g_glb.hTree, hi);
  if(!hci)
  {
    return TvCopyToOther(hi);
  }
  while(hci && iResult)
  {
    iResult = TvCopyToOtherRecursive(hci);
    hci = TreeView_GetNextSibling(g_glb.hTree, hci);
  }
  return iResult;
}

static void TvSffdCopyAllGroup(HTREEITEM hi)
{
  hi = TreeView_GetChild(g_glb.hTree, hi);
  if(TreeView_GetChild(g_glb.hTree, hi))
    TvCopyToOtherRecursive(hi);
  hi = TreeView_GetNextSibling(g_glb.hTree, hi);
  if(TreeView_GetChild(g_glb.hTree, hi))
    TvCopyToOtherRecursive(hi);
}

static void fTvSffdGroup(HTREEITEM hi)
{
  int iCount = TreeView_GetChild(g_glb.hTree, hi) ? 3 : 2;
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, iCount, g_str.pTbl->szOpen, g_str.pTbl->szOpenContainer, g_str.pTbl->szCopyAll))
  {
    case 1:
      TvOpenFile(0, hi);
      break;
    case 2:
      TvOpenPathInExplorer(0, hi);
      break;
    case 3:
      TvCopyToOtherRecursive(hi);
      break;
  }
}

static void fTvSffdFile(HTREEITEM hi)
{
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, 6, g_str.pTbl->szOpen, g_str.pTbl->szOpenContainer, g_str.pTbl->szCopyToOther, g_str.pTbl->szFileProp, 0, g_str.pTbl->szDeleteFile))
  {
    case 1:
      TvOpenFile(TvGetSffdFileRoot(hi), hi);
      break;
    case 2:
      TvOpenPathInExplorer(TvGetSffdFileRoot(hi), hi);
      break;
    case 3:
      TvCopyToOtherRecursive(hi);
      break;
    case 4:
      TvFilePropDlg(TvGetSffdFileRoot(hi), hi);
      break;
    case 6:
      TvAskAndDeleteFile(TvGetSffdFileRoot(hi), hi);
      break;
  }
}

static void fTvSfdfFile(HTREEITEM hi)
{
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, 5, g_str.pTbl->szOpen, g_str.pTbl->szOpenContainer, g_str.pTbl->szFileProp, 0, g_str.pTbl->szDeleteFile))
  {
    case 1:
      TvOpenFile(0, hi);
      break;
    case 2:
      TvOpenPathInExplorer(0, hi);
      break;
    case 3:
      TvFilePropDlg(0, hi);
      break;
    case 5:
      TvAskAndDeleteFile(0, hi);
      break;
  }
}

static void fTvSffdRoot(HTREEITEM hi)
{
  switch(g_mnu.MnuPopupMenu(g_glb.hTree, 3, g_str.pTbl->szCopyAll, 0, g_str.pTbl->szEmptyGroup))
  {
    case 1:
      TvSffdCopyAllGroup(hi);
      break;
    case 3:
      TvDeleteChildren(hi);
      break;
  }
}

static void fTvOpenSelectAbsent(HTREEITEM hi)
{
}

static void fTvOpenSelectBigger(HTREEITEM hi)
{
}

static void fTvOpenSelectNewer(HTREEITEM hi)
{
}

struct TvScanForLeaveCountCheckDir
{
  HTREEITEM hi;
  PWSTR szPath;
  DWORD dwCount;
};

static void TvScanForLeaveCountCheckDir(struct TvScanForLeaveCountCheckDir* psc)
{
  HTREEITEM hci;
  if(psc->dwCount == (DWORD)-1)
    return;
  hci = TreeView_GetChild(g_glb.hTree, psc->hi);
  if(!hci)
  {
    TvGetItemFullPath(g_glb.hScanRoot, psc->hi, psc->szPath);
    if(GetFileAttributes(psc->szPath) & FILE_ATTRIBUTE_DIRECTORY)
      psc->dwCount ++;
    else
      psc->dwCount = (DWORD)-1;
  }
  while(hci)
  {
    psc->hi = hci;
    TvScanForLeaveCountCheckDir(psc);
    hci = TreeView_GetNextSibling(g_glb.hTree, hci);
  }
}

static int TvCheck2DirInputsAndWarn()
{
  struct TvScanForLeaveCountCheckDir sc;
  WCHAR wchPathBuff[TXT_BUFF_SIZE];
  sc.hi = g_glb.hScanRoot;
  sc.szPath = wchPathBuff;
  sc.dwCount = 0;
  TvScanForLeaveCountCheckDir(&sc);
  if(sc.dwCount != 2)
  {
    TvMsgBoxAndLog(g_str.pTbl->szOnly2Dirs, g_str.pTbl->szInvalidInput, MB_OK | MB_ICONWARNING);
    return 1;
  }
  return 0;
}

static int TvCheckIfNoInputAndWarn()
{
  if(!TreeView_GetChild(g_glb.hTree, g_glb.hScanRoot))
  {
    TvAddLogLine(g_str.pTbl->szNoInputLog, g_str.pTbl->szNoInput, g_str.pTbl->szDragDrop);
    TvMsgBox(g_str.pTbl->szDragDrop, g_str.pTbl->szNoInput, MB_OK | MB_ICONWARNING);
    return 1;
  }
  return 0;
}

static void TvCleanup()
{
  TvAddLogLine(g_str.pTbl->szCleanUpTree);
  TvDeleteChildren(g_glb.hScanLog);
  TvDeleteChildren(g_glb.hSfdfScanResult);
  TvDeleteChildren(g_glb.hSffdNew);
  TvDeleteChildren(g_glb.hSffdBig);
  TvDeleteChildren(g_glb.hSffdAbsent);
}

static void TvExpandSffdRes()
{
  TreeView_Expand(g_glb.hTree, g_glb.hSffdStartScan, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hSffdNew, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hSffdBig, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hSffdAbsent, TVE_EXPAND);
  
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdNewRoots[0]);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdNewRoots[1]);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdBigRoots[0]);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdBigRoots[1]);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdAbsentRoots[0]);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdAbsentRoots[1]);
}

static void TvCleanupSffdOutRoots()
{
  if(g_glb.hSffdNewRoots[0])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdNewRoots[0]);
  if(g_glb.hSffdBigRoots[0])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdBigRoots[0]);
  if(g_glb.hSffdAbsentRoots[0])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdAbsentRoots[0]);

  if(g_glb.hSffdNewRoots[1])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdNewRoots[1]);
  if(g_glb.hSffdBigRoots[1])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdBigRoots[1]);
  if(g_glb.hSffdAbsentRoots[1])
    TreeView_DeleteItem(g_glb.hTree, g_glb.hSffdAbsentRoots[1]);

  g_glb.hSffdNewRoots[0] = g_glb.hSffdBigRoots[0] = g_glb.hSffdAbsentRoots[0] = 0;
  g_glb.hSffdNewRoots[1] = g_glb.hSffdBigRoots[1] = g_glb.hSffdAbsentRoots[1] = 0;
}

void fTvStartSfdfScan()
{
  struct TvFsScanLeavesSfdf sc;
  WCHAR wchPathBuff[TXT_BUFF_SIZE];
  DeleteFile(L".files");
  TvDeleteChildren(g_glb.hSfdfScanResult);
  TvDeleteChildren(g_glb.hScanLog);
  if(TvCheckIfNoInputAndWarn())
    return;
  TvSetState(TVS_SCANNING);
  g_fs.FsInitScan();
  sc.hi = g_glb.hScanRoot;
  sc.szPath = wchPathBuff;
  TvFsScanLeavesSfdf(&sc);
  g_fs.FsSfdf();
  TreeView_Expand(g_glb.hTree, g_glb.hSfdfStartScan, TVE_EXPAND);
  TreeView_Expand(g_glb.hTree, g_glb.hSfdfScanResult, TVE_EXPAND);
  TreeView_SelectItem(g_glb.hTree, g_glb.hSfdfStartScan);
  if(TvCheckCancel())
    TvCleanup();
  TvSetState(TVS_IDLE);
}

void fTvSfdfRes(HTREEITEM hi)
{
  if(1 == g_mnu.MnuPopupMenu(g_glb.hTree, 1, g_str.pTbl->szCleanAll))
    TvDeleteChildren(hi);
}

void fTvStartSffdScan()
{
  TvDeleteChildren(g_glb.hSffdNew);
  TvDeleteChildren(g_glb.hSffdBig);
  TvDeleteChildren(g_glb.hSffdAbsent);
  TvDeleteChildren(g_glb.hScanLog);
  TvCleanupSffdOutRoots();
  if(TvCheckIfNoInputAndWarn())
    return;
  if(TvCheck2DirInputsAndWarn())
    return;
  TvSetState(TVS_SCANNING);
  g_fs.FsInitScan();
  TvFsScanLeaveSffd();
  TvExpandSffdRes();
  TreeView_SelectItem(g_glb.hTree, g_glb.hSffdStartScan);
  if(TvCheckCancel())
    TvCleanup();
  TvSetState(TVS_IDLE);
}

void fTvLogs()
{
  UINT uSel = g_mnu.MnuPopupMenu(g_glb.hTree, 1, g_str.pTbl->szCleanupLog);
  if(uSel == 1)
    TvDeleteChildren(g_glb.hScanLog);
}

void TvInitFs()
{
  g_fs.fAddLogLine = TvAddLogLine;
  g_fs.fMsgBoxAndLog = TvMsgBoxAndLog;
  g_fs.fAddSfdfGroup = TvAddSfdfGroup;
  g_fs.fAddSfdfItem = TvAddSfdfItem;
  g_fs.fAddSffdNewer = TvAddSffdNewer;
  g_fs.fAddSffdBigger = TvAddSffdBigger;
  g_fs.fAddSffdAbsent = TvAddSffdAbsent;
  g_fs.fSortLastGroup = TvSortLastGroup;
  g_fs.fProgressTitle = TvProgressTitle;
  g_fs.fCheckCancel = TvCheckCancel;  
}

struct TV_API g_tv =
{
  &g_glb
  , TvInit
  , TvFree
  , TvAddLogLine
};

