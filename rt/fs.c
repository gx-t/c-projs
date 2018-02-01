#include "rt.h"
#include "sha256.h"

#define FS_MAXFILECOUNT 32768 //limited by stack size

struct FileEntry
{
  DWORD dwSizeHigh, dwSizeLow;
  WCHAR wchPath[1];
};

struct IdxSha256
{
  struct FileEntry* pfe;
  union
  {
    BYTE bt[32];
    DWORD dw[8];
  }sh;
};

static struct FS_GLB g_glb;

static void FsSlowOpMsgPass()
{
  static DWORD dwOldTicks;
  DWORD dwTicks = GetTickCount();
  if(dwTicks - dwOldTicks > 100)
  {
    g_fs.fProgressTitle();
    g_rt.RtPassMsg();
    dwOldTicks = dwTicks;
  }
}

static void FsNormalizePath(PWSTR szPath)
{
  if(GetFileAttributes(szPath) & FILE_ATTRIBUTE_DIRECTORY)
  {
    while(*szPath) szPath ++;
    if(*(szPath - 1) != L'\\')
    {
      *szPath ++ = L'\\';
      *szPath = 0;
    }
  }
}

static PWSTR FsSkipNetworkBacklashes(PWSTR szPath)
{
  while(*szPath == L'\\') szPath ++;
  return szPath;
}

static PCWSTR FsGetExtension(PCWSTR szPath)
{
	PCWSTR p = szPath;
	while(*p++);
	while(--p != szPath && *p != L'\\' && *p != L'.');
	if(*p == L'.' && *++p)
		return p;
	return 0;
}

static PWSTR FsSelectDirAndFile(HWND hWnd, PCWSTR szTitle, PWSTR szPathBuff)
{
  *szPathBuff = 0;
  LPITEMIDLIST lpid;
  BROWSEINFO bi = {0};
  LPMALLOC pMalloc = 0;
  bi.hwndOwner = hWnd;
  bi.lpszTitle = szTitle;
  bi.ulFlags = 0x0200 | BIF_NEWDIALOGSTYLE | BIF_BROWSEINCLUDEFILES;
  lpid = SHBrowseForFolder(&bi);
  if(lpid)
  {
    if(
      SHGetPathFromIDList(lpid, szPathBuff) &&
      NOERROR == SHGetMalloc(&pMalloc)
      )
    {
      pMalloc->lpVtbl->Free(pMalloc, lpid);
      pMalloc->lpVtbl->Release(pMalloc);
      return szPathBuff;
    }
  }
  return 0;
}

static int FsCopy(HWND hWnd, PCWSTR szDest, PCWSTR szSrc)
{
  SHFILEOPSTRUCT sh = {hWnd, FO_COPY, szSrc, szDest, FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI};
  DWORD dwAttrDest = GetFileAttributes(szDest);
  DWORD dwAttrSrc = GetFileAttributes(szSrc);
  if(dwAttrSrc == 0xFFFFFFFF)
  {
    g_fs.fAddLogLine(g_str.pTbl->szErrCopyNoSrc, szSrc);
    return 0;
  }
  if(dwAttrDest != 0xFFFFFFFF)
  {
    if((dwAttrDest & FILE_ATTRIBUTE_DIRECTORY) && !(dwAttrSrc & FILE_ATTRIBUTE_DIRECTORY))
    {
      g_fs.fAddLogLine(g_str.pTbl->szErrCopyDirFile);
      return 0;
    }
    if(!(dwAttrDest & FILE_ATTRIBUTE_DIRECTORY) && (dwAttrSrc & FILE_ATTRIBUTE_DIRECTORY))
    {
      g_fs.fAddLogLine(g_str.pTbl->szErrCopyFileDir);
      return 0;
    }
  }
  g_fs.fAddLogLine(g_str.pTbl->szCopyingFromTo, szSrc, szDest);
  int iRet = SHFileOperation(&sh);
  if(iRet)
  {
    g_fs.fAddLogLine(g_str.pTbl->szErrCopy, g_str.pTbl->szCannotContinue, MB_OK | MB_ICONERROR);
    return 0;
  }
  if(sh.fAnyOperationsAborted)
  {
    g_fs.fAddLogLine(g_str.pTbl->szCopyAborted);
    return 0;
  }
  return 1;
}

struct FsScanCollectNames
{
  HANDLE hFile;
  PWSTR szScanRoot;
};

static void FsScanCollectNames(struct FsScanCollectNames* psc, PWSTR szEnd)
{
  static DWORD dwBytesWritten;
  static DWORD dwBytes;
  static PWSTR p0, p1;
  HANDLE hFind;
  if(g_fs.fCheckCancel())
    return;
  if(GetFileAttributes(psc->szScanRoot) & FILE_ATTRIBUTE_DIRECTORY)
  {
    *szEnd ++ = L'*';
    *szEnd = 0;
    hFind = FindFirstFile(psc->szScanRoot, &g_glb.fd);
    if(hFind == INVALID_HANDLE_VALUE)
    {
      g_fs.fAddLogLine(g_str.pTbl->szCannotScanDir, psc->szScanRoot);
      return;
    }
    if(*(PDWORD)g_glb.fd.cFileName == '.')//0x2E)
    {
      FindNextFile(hFind, &g_glb.fd);
      if(!FindNextFile(hFind, &g_glb.fd))
      {
        FindClose(hFind);
        return;
      }
    }
    szEnd --;
    do
    {
      p0 = szEnd;
      p1 = g_glb.fd.cFileName;
      while(*p1)
        *p0 ++ = *p1 ++;
      *p0 = 0;
      if(g_glb.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        *p0 ++ = L'\\';
        *p0 = 0;
      }
      FsScanCollectNames(psc, p0);
      FsSlowOpMsgPass();
    }while(FindNextFile(hFind, &g_glb.fd));
    FindClose(hFind);
  }
  else
  {
    WriteFile(psc->hFile, &g_glb.fd.nFileSizeHigh, 2 * sizeof(DWORD), &dwBytesWritten, 0);
    dwBytes = (unsigned)szEnd - (unsigned)psc->szScanRoot;
    dwBytes += sizeof(WCHAR);
    WriteFile(psc->hFile, psc->szScanRoot, dwBytes, &dwBytesWritten, 0);
    g_glb.dwFileCount ++;
  }
}

static void FsDelTempFiles()
{
  DeleteFile(L".files");
}

static void FsCollectFileNames(PWSTR szScanRoot)
{
  PWSTR p = szScanRoot;
  struct FsScanCollectNames sc;
  sc.hFile = CreateFile(L".files", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if(sc.hFile == INVALID_HANDLE_VALUE)
  {
    g_fs.fAddLogLine(g_str.pTbl->szCannotOpenDataFile);
    return;
  }
  g_fs.fAddLogLine(g_str.pTbl->szCollectingNames, szScanRoot);
  SetFilePointer(sc.hFile, 0, 0, FILE_END);
  while(*p) p++;
  sc.szScanRoot = szScanRoot;
  FsScanCollectNames(&sc, p);
  CloseHandle(sc.hFile);
}

static void FsCollectIdx(struct FileEntry* pfe, struct FileEntry** pIdxFile)
{
  PCWSTR szPath;
  DWORD dwCount = g_glb.dwFileCount;
  if(g_fs.fCheckCancel())
    return;
  g_fs.fAddLogLine(g_str.pTbl->szCollectSize);
  while(!g_fs.fCheckCancel() && dwCount --)
  {
    *pIdxFile = pfe;
    szPath = pfe->wchPath;
    while(*szPath ++);
    pfe = (struct FileEntry*)szPath;
    pIdxFile ++;
    FsSlowOpMsgPass();
  }
}

static int FsSizeCmp(struct FileEntry** pIdx1, struct FileEntry** pIdx2)
{
  if((*pIdx1)->dwSizeHigh > (*pIdx2)->dwSizeHigh)
    return 1;
  if((*pIdx1)->dwSizeHigh < (*pIdx2)->dwSizeHigh)
    return -1;
  if((*pIdx1)->dwSizeLow > (*pIdx2)->dwSizeLow)
    return 1;
  if((*pIdx1)->dwSizeLow < (*pIdx2)->dwSizeLow)
    return -1;
  return 0;
}

static void FsSortSizeGetHashCount(struct FileEntry** pIdxFile, PDWORD pdwHashCount)
{
  struct FileEntry **pFfd1, **pFfd2;
  int iLoop = 1;
  DWORD dwCount = g_glb.dwFileCount;
  *pdwHashCount = 0;
  if(g_fs.fCheckCancel())
    return;
  g_fs.fAddLogLine(g_str.pTbl->szSortBySize, g_glb.dwFileCount);
  qsort(pIdxFile, g_glb.dwFileCount, sizeof(struct FileEntry*), (int (*)(const void*, const void*))FsSizeCmp);
  g_fs.fAddLogLine(g_str.pTbl->szCalcSameSizeCount, g_glb.dwFileCount);
  pFfd2 = pFfd1 = pIdxFile;
  while(!g_fs.fCheckCancel() && -- dwCount)
  {
    pFfd2 ++;
    if(!FsSizeCmp(pFfd1, pFfd2))
    {
      if(iLoop)
      {
        iLoop = 0;
        (*pdwHashCount) ++;
      }
      (*pdwHashCount) ++;
    }
    else
      iLoop = 1;
    pFfd1 = pFfd2;
    FsSlowOpMsgPass();
  }
  g_fs.fAddLogLine(g_str.pTbl->szFoundSameSize, *pdwHashCount);
}

static void FsExtractSameSize(struct FileEntry** pIdxFile, struct IdxSha256* pIdxHash)
{
  struct FileEntry **pFfd1, **pFfd2;
  int iLoop = 1;
  DWORD dwCount = g_glb.dwFileCount;
  if(g_fs.fCheckCancel())
    return;
  pFfd2 = pFfd1 = pIdxFile;
  g_fs.fAddLogLine(g_str.pTbl->szLogExtractSameSize);
  while(!g_fs.fCheckCancel() && -- dwCount)
  {
    pFfd2 ++;
    if(!FsSizeCmp(pFfd1, pFfd2))
    {
      if(iLoop)
      {
        iLoop = 0;
        pIdxHash->pfe = *pFfd1;
        pIdxHash ++;
      }
      pIdxHash->pfe = *pFfd2;
      pIdxHash ++;
    }
    else
      iLoop = 1;
    pFfd1 = pFfd2;
    FsSlowOpMsgPass();
  }
}

static void FsCalcHash(struct IdxSha256* pIdxHash, DWORD dwHashCount)
{
  HANDLE hFile;
  DWORD dwBytesR;
  BYTE btBuff[0x10000];
  sha256_context ctx;
  if(g_fs.fCheckCancel())
    return;
  g_fs.fAddLogLine(g_str.pTbl->szCalcHash, dwHashCount);
  while(!g_fs.fCheckCancel() && dwHashCount --)
  {
    hFile = CreateFile(pIdxHash->pfe->wchPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if(hFile == INVALID_HANDLE_VALUE)
    {
      g_fs.fAddLogLine(g_str.pTbl->szErrHash, pIdxHash->pfe->wchPath);
      memset(pIdxHash->sh.bt, 0, 8);
      pIdxHash->pfe = 0;
      pIdxHash ++;
      continue;
    }
    sha256_starts(&ctx);
    while(!g_fs.fCheckCancel() && ReadFile(hFile, btBuff, sizeof(btBuff), &dwBytesR, 0))
    {
      FsSlowOpMsgPass();
      if(!dwBytesR)
        break;
      sha256_update(&ctx, btBuff, dwBytesR);
    }
    sha256_finish(&ctx, pIdxHash->sh.bt);
    CloseHandle(hFile);
    pIdxHash ++;
  }
}

static int FsSha256Cmp(struct IdxSha256* pSh1, struct IdxSha256* pSh2)
{
  return memcmp(pSh1->sh.bt, pSh2->sh.bt, 32);
}

static void FsSortHashExtractEquals(struct IdxSha256* pIdxHash, DWORD dwHashCount)
{
  if(g_fs.fCheckCancel())
    return;
  g_fs.fAddLogLine(g_str.pTbl->szSortByHash, dwHashCount);
  qsort(pIdxHash, dwHashCount, sizeof(struct IdxSha256), (int (*)(const void*, const void*))FsSha256Cmp);
  g_fs.fAddLogLine(g_str.pTbl->szExtractSameHash);
  struct IdxSha256 *pSh1, *pSh2;
  int iLoop = 1;
  DWORD dwGroups = 0, dwTotalDuplicate = 0;
  while(!pIdxHash->pfe) ++pIdxHash, --dwHashCount;
  pSh2 = pSh1 = pIdxHash;
  while(!g_fs.fCheckCancel() && -- dwHashCount)
  {
    pSh2 ++;
    if(!FsSha256Cmp(pSh1, pSh2))
    {
      if(iLoop)
      {
        iLoop = 0;
        g_fs.fAddSfdfGroup(pSh1->pfe->wchPath);
        g_fs.fAddSfdfItem(pSh1->pfe->wchPath);
        dwGroups ++;
        dwTotalDuplicate ++;
      }
      g_fs.fAddSfdfItem(pSh2->pfe->wchPath);
      dwTotalDuplicate ++;
    }
    else
    {
      if(!iLoop)
      {
        g_fs.fSortLastGroup();
      }
      iLoop = 1;
    }
    pSh1 = pSh2;
    FsSlowOpMsgPass();
  }
  g_fs.fAddLogLine(g_str.pTbl->szDuplicateFound, dwGroups, dwTotalDuplicate);
}

static void FsSfdf()
{
  HANDLE hFile;
  HANDLE hMap;
  struct FileEntry* pfe;
  struct FileEntry** pIdxFile;
  struct IdxSha256* pIdxHash;
  DWORD dwHashCount;
  DWORD dwBytes;
  HANDLE hp = GetProcessHeap();
  g_fs.fAddLogLine(g_str.pTbl->szFileCount, g_glb.dwFileCount);
  if(g_glb.dwFileCount >= 2)
  {
    hFile = CreateFile(L".files", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if(hFile != INVALID_HANDLE_VALUE)
    {
      hMap = g_sys.pSysProcTbl->CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
      CloseHandle(hFile);
      if(hMap)
      {
        pfe = (struct FileEntry*)g_sys.pSysProcTbl->MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if(pfe)
        {
          dwBytes = g_glb.dwFileCount * sizeof(struct FileEntry*);
          pIdxFile = (struct FileEntry**)HeapAlloc(hp, 0, dwBytes);
          if(pIdxFile)
          {
            FsCollectIdx(pfe, pIdxFile);
            FsSortSizeGetHashCount(pIdxFile, &dwHashCount);
            if(dwHashCount)
            {
              dwBytes = dwHashCount * sizeof(struct IdxSha256);
              pIdxHash = (struct IdxSha256*)HeapAlloc(hp, 0, dwBytes);
              if(pIdxHash)
              {
                FsExtractSameSize(pIdxFile, pIdxHash);
                FsCalcHash(pIdxHash, dwHashCount);
                FsSortHashExtractEquals(pIdxHash, dwHashCount);
                HeapFree(hp, 0, pIdxHash);
              }
            }
            HeapFree(hp, 0, pIdxFile);
          }
          g_sys.pSysProcTbl->UnmapViewOfFile((PVOID)pfe);
        }
        else
        {
          g_fs.fAddLogLine(g_str.pTbl->szErrFilesMapping, g_str.pTbl->szFatalErr);
        }
        CloseHandle(hMap);
      }
      else
        g_fs.fAddLogLine(g_str.pTbl->szNoCollectInfo);
    }
    else
      g_fs.fAddLogLine(g_str.pTbl->szErrOpenFilesFile, g_str.pTbl->szFatalErr);
  }
  else
    g_fs.fAddLogLine(g_str.pTbl->szNeededAtLeast2);
  FsDelTempFiles();
}

struct FsScanDiffFiles
{
  PWSTR szScanRoot0, szScanRoot1, szRelPath0, szRelPath1;
};

struct FsGetFileSizeTime
{
  PCWSTR szFile;
  DWORD dwSizeHigh;
  DWORD dwSizeLow;
  FILETIME ftWrite;
};

static int FsGetFileSizeTime(struct FsGetFileSizeTime* pfst)
{
  HANDLE hFile = CreateFile(pfst->szFile
                            , 0
                            , FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
                            , 0, OPEN_EXISTING
                            , 0
                            , 0);
  if(hFile == INVALID_HANDLE_VALUE)
  {
    pfst->dwSizeHigh = pfst->dwSizeLow
      = pfst->ftWrite.dwLowDateTime
      = pfst->ftWrite.dwHighDateTime = 0;
    g_fs.fAddLogLine(g_str.pTbl->szErrOpenToCmp, pfst->szFile);
    return 0;
  }
  pfst->dwSizeLow = GetFileSize(hFile, &pfst->dwSizeHigh);
  if(!GetFileTime(hFile, 0, 0, &pfst->ftWrite))
  {
    g_fs.fAddLogLine(g_str.pTbl->szErrFileTime, pfst->szFile);
    pfst->ftWrite.dwLowDateTime = pfst->ftWrite.dwHighDateTime = 0;
  }
  CloseHandle(hFile);
  return 1;
}

static int FsProcessFileDiff(struct FsScanDiffFiles* psc, DWORD dwAttr0)
{
  struct FsGetFileSizeTime fst0, fst1;
  DWORD dwAttr1 = GetFileAttributes(psc->szScanRoot1);
  if(dwAttr1 == (DWORD)-1
    || ((dwAttr1 & FILE_ATTRIBUTE_DIRECTORY) && !(dwAttr0 & FILE_ATTRIBUTE_DIRECTORY))
    || ((dwAttr0 & FILE_ATTRIBUTE_DIRECTORY) && !(dwAttr1 & FILE_ATTRIBUTE_DIRECTORY)))
  {
    g_fs.fAddSffdAbsent(psc->szScanRoot0, psc->szRelPath0);
    return 1;
  }
  if((dwAttr0 & FILE_ATTRIBUTE_DIRECTORY) && (dwAttr1 & FILE_ATTRIBUTE_DIRECTORY))
    return 0;
  fst0.szFile = psc->szScanRoot0;
  fst1.szFile = psc->szScanRoot1;
  if(FsGetFileSizeTime(&fst0) && FsGetFileSizeTime(&fst1))
  {
    if(fst0.dwSizeHigh > fst1.dwSizeHigh
      || (fst0.dwSizeHigh == fst1.dwSizeHigh && fst0.dwSizeLow > fst1.dwSizeLow))
      g_fs.fAddSffdBigger(psc->szScanRoot0, psc->szRelPath0);

    if(fst0.ftWrite.dwHighDateTime > fst1.ftWrite.dwHighDateTime
      || (fst0.ftWrite.dwHighDateTime == fst1.ftWrite.dwHighDateTime
          && fst0.ftWrite.dwLowDateTime > fst1.ftWrite.dwLowDateTime))
      g_fs.fAddSffdNewer(psc->szScanRoot0, psc->szRelPath0);
    return 0;
  }
  return 0;
}

static void FsScanDiffFiles(struct FsScanDiffFiles* psc, PWSTR szEnd0, PWSTR szEnd1)
{
  static PWSTR p0, p1, p2;
  HANDLE hFind;
  DWORD dwAttr0;
  if(g_fs.fCheckCancel())
    return;
  dwAttr0 = GetFileAttributes(psc->szScanRoot0);
  if(FsProcessFileDiff(psc, dwAttr0))
    return;
  ++ g_glb.dwFileCount;
  if(dwAttr0 & FILE_ATTRIBUTE_DIRECTORY)
  {
    *szEnd0 ++ = L'*';
    *szEnd0 = 0;
    hFind = FindFirstFile(psc->szScanRoot0, &g_glb.fd);
    if(hFind == INVALID_HANDLE_VALUE)
    {
      g_fs.fAddLogLine(g_str.pTbl->szCannotScanDir, psc->szScanRoot0);
      return;
    }
    if(*(PDWORD)g_glb.fd.cFileName == '.')//0x2E)
    {
      FindNextFile(hFind, &g_glb.fd);
      if(!FindNextFile(hFind, &g_glb.fd))
      {
        FindClose(hFind);
        return;
      }
    }
    szEnd0 --;
    do
    {
      p0 = szEnd0;
      p2 = szEnd1;
      p1 = g_glb.fd.cFileName;
      while(*p1)
        *p2 ++ = *p0 ++ = *p1 ++;

      *p2 = *p0 = 0;
      if(g_glb.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        *p2 ++ = *p0 ++ = L'\\';
        *p2 = *p0 = 0;
      }
      FsScanDiffFiles(psc, p0, p2);
      FsSlowOpMsgPass();
    }while(!g_fs.fCheckCancel() && FindNextFile(hFind, &g_glb.fd));
    FindClose(hFind);
  }
}

static void FsSffd(PWSTR szScanRoot0, PWSTR szScanRoot1)
{
  struct FsScanDiffFiles sc;
  g_glb.dwFileCount = 0;
  sc.szRelPath0 = sc.szScanRoot0 = szScanRoot0;
  sc.szRelPath1 = sc.szScanRoot1 = szScanRoot1;
  while(*++sc.szRelPath0);
  while(*++sc.szRelPath1);
  FsScanDiffFiles(&sc, sc.szRelPath0, sc.szRelPath1);
}

static void FsInitScan()
{
  g_glb.dwFileCount = 0;
}

struct FS_API g_fs = 
{
  //global data:
  &g_glb
  //services:
    , FsNormalizePath
    , FsSkipNetworkBacklashes
    , FsGetExtension
    , FsSelectDirAndFile
    , FsCopy
    , FsInitScan
    , FsCollectFileNames
    , FsSfdf
    , FsSffd
    , FsDelTempFiles
    
    //callbacks... set by client
};
