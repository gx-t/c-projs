#include "common.h"

#define DATA_FILE_MAGIC         0xF673B270
#define DATA_FILE_VERSION       1
#define MAX_FILE_COUNT          0x1000000

enum
{
  STATE_OPEN,
    STATE_CLOSE
};

static PCWSTR szTitle           = L"Scan for duplicate files";
static PCWSTR szErrCreateMapIdx = L"Critical error: cannot create index file mapping";
static PCWSTR szErrMapViewIdx   = L"Critical error: cannot map view of index file";
static PCWSTR szErrScanDup      = L"Duplicate file scan error";
static PCWSTR szRegFldLst       = L"Software\\ShahSoft\\MultiDesktop\\Files\\ScanFiles\\ScanDuplicate\\FolderList";
static PCWSTR szRegDupFiles     = L"Software\\ShahSoft\\MultiDesktop\\Files\\ScanFiles\\ScanDuplicate\\DuplicateFiles";

static PCWSTR szErrCreateOutFile= L"Cannot create the output file";

static PCWSTR szInitMsg[]       =
{
  L"Error writing to scan output file."
  , L"Cannot create scan output file."
  , L"Cannot go to the scan data folder."
  , L"Cannot go to user application data folder."
};

#define INIT_MSG(i_)            szInitMsg[i_ + sizeof(szInitMsg) / sizeof(WCHAR)]

static PCWSTR szAddFolder       = L"Choose a folder to add to scan list";
static PCWSTR szErrMove         = L"Cannot enter the selected folder. Check permissions";
static PCWSTR szErrAdd          = L"Error adding folder to list";

static PCWSTR szAddFldrRes[]    =
{
  L"Cannot enter the selected folder. Check permissions"
  , L"The folder could not be added to the scan list"
  , L"You have cancelled folder adding"
  , L"The selected folder is already in scan list"
  , L"The folder has been added to scan list"
};

static PCWSTR szAddTitle        = L"Adding folder to scan list";
static PCWSTR szDelConf         = L"Do you want to remove the selected path from scan list?";
static PCWSTR szErrDelPath[]    =
{
  L"Error removing path from the scan list"
  , L"The path has been removed from the scan list"
};

static PCWSTR szAddFldrLst      = L"Add folders to the list and try again";
static PCWSTR szErrScan         = L"Cannot scan the folders";
static PCWSTR szScanDataDir     = L"File Scan Data";
static PCWSTR szScFilePref      = L"Scan Data - ";
static PCWSTR szScFilePost      = L" - .bin";


#define FDHDR_SIZE              (4 * sizeof(DWORD) + sizeof(WORD))
#define FNAME_SIZE(ns_)         (ns_ * sizeof(WCHAR) + sizeof(WCHAR))
#define PBUFF_LEN               0x10000
#define PBUFF_SIZE              (PBUFF_LEN * sizeof(WCHAR))


typedef struct SCAN_DATA
{
  WIN32_FIND_DATA fd;
  PWSTR   p1, p2;
  HANDLE  hDataFile;
  DWORD   dwBytesReadWritten;
  PDWORD  pdwIdxBuff;
  PDWORD  pdwIdxIter;
  DWORD   dwFileCount;
  DWORD   dwMd5Hash[4];
  WORD    wPathLen;
  PWSTR   szPathBuff;
} SCAN_DATA, *PSCAN_DATA;

#pragma pack(4)
typedef struct FLF_HEADER
{
  DWORD dwMagic;
  DWORD dwVersion;
  DWORD dwState;
  DWORD dwFileCount;
}FLF_HEADER, *PFLF_HEADER;
#pragma pack()

#pragma pack(1)
typedef struct FF_DATA
{
  DWORD dwSizeLow;
  DWORD dwSizeHigh;
  WORD  wPathLen;
}FF_DATA, *PFF_DATA;
#pragma pack()

typedef struct SCAN_CONTEXT
{
  WIN32_FIND_DATA fd;
  HANDLE hDataFile;
  FF_DATA ffd;
  PWSTR szPathBuff;
  DWORD dwFileCount;
  DWORD dwDirCount;
  DWORD dwBytesWritten;
  PWSTR p1, p2;
}SCAN_CONTEXT, *PSCAN_CONTEXT;

typedef struct SORT_CONTEXT
{
  HANDLE hDataFile;
}SORT_CONTEXT, *PSORT_CONTEXT;

union
{
  PSCAN_CONTEXT psc;
  PSORT_CONTEXT pSrtC;
}static g_Cntx;

static void MergePathList(PWSTR szPathBuff, HKEY hKey)
{
  PWSTR p;
  HKEY hSubKey;
  DWORD dwIdx = 0;
  BOOL bMerge = FALSE;
  while(ERROR_SUCCESS == RegEnumKey(hKey, dwIdx ++, szPathBuff, PBUFF_SIZE))
  {
    p = szPathBuff;
    while(*p)
    {
      if(*p == L'/')
      {
        *p = 0;
        if(ERROR_SUCCESS == RegOpenKey(hKey, szPathBuff, &hSubKey))
        {
          bMerge = TRUE;
          RegCloseKey(hSubKey);
        }
        *p = L'/';
        if(bMerge)
        {
          wprintf(L"Path intersection found. Removing extra path:\n%s\n", szPathBuff);
          RegDeleteKey(hKey, szPathBuff);
          dwIdx --;
          bMerge = FALSE;
          break;
        }
      }
      p ++;
    }
  }
}

static void ScanDir(PWSTR szEnd)
{
  HANDLE hFind;
  *szEnd ++ = L'/';
  *szEnd ++ = L'*';
  *szEnd = 0;
//  wprintf(L"%s\n", g_Cntx.psc->szPathBuff);

  hFind = FindFirstFile(g_Cntx.psc->szPathBuff, &g_Cntx.psc->fd);
  if(hFind == INVALID_HANDLE_VALUE)
  {
    wprintf(L"Unable to scan.\n");
    return;
  }
  if(*(PDWORD)g_Cntx.psc->fd.cFileName == 0x2E)
  {
    FindNextFile(hFind, &g_Cntx.psc->fd);
    if(!FindNextFile(hFind, &g_Cntx.psc->fd))
      return;
  }
  g_Cntx.psc->dwDirCount ++;
  szEnd --;
  do{
    g_Cntx.psc->p1 = g_Cntx.psc->fd.cFileName;
    g_Cntx.psc->p2 = szEnd;
    while(*g_Cntx.psc->p1)
      *g_Cntx.psc->p2 ++ = *g_Cntx.psc->p1 ++;
    *g_Cntx.psc->p2 = 0;

    g_Cntx.psc->ffd.wPathLen = (WORD)(g_Cntx.psc->p2 - g_Cntx.psc->szPathBuff);

    //directory branch
    if(g_Cntx.psc->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      ScanDir(g_Cntx.psc->p2);
    //file branch
    else
    {
      g_Cntx.psc->dwFileCount ++;
      g_Cntx.psc->ffd.dwSizeLow = g_Cntx.psc->fd.nFileSizeLow;
      g_Cntx.psc->ffd.dwSizeHigh = g_Cntx.psc->fd.nFileSizeHigh;
      WriteFile(g_Cntx.psc->hDataFile
        , &g_Cntx.psc->ffd
        , sizeof(FF_DATA)
        , &g_Cntx.psc->dwBytesWritten
        , 0);
//      wprintf(L"+%s\n", g_Cntx.psc->szPathBuff);
      WriteFile(g_Cntx.psc->hDataFile
        , g_Cntx.psc->szPathBuff
        , sizeof(WCHAR) * (g_Cntx.psc->ffd.wPathLen + 1)
        , &g_Cntx.psc->dwBytesWritten
        , 0);
//      wprintf(L"file: %s\n", g_Cntx.psc->szPathBuff);
    }
  }while(FindNextFile(hFind, &g_Cntx.psc->fd));

  FindClose(hFind);
}

static void InitWriteDataFileHeader()
{
  FLF_HEADER Header = {DATA_FILE_MAGIC
    , DATA_FILE_VERSION
    , STATE_OPEN};
  wprintf(L"Writing initial header.\n");
  if(!WriteFile(g_Cntx.psc->hDataFile
    , &Header
    , sizeof(Header)
    , &g_Cntx.psc->dwBytesWritten
    , 0) || g_Cntx.psc->dwBytesWritten != sizeof(Header))
  {
    wprintf(L"Error writing header initial values\n");
  }
}

static void WriteDataFileHeader()
{
  FLF_HEADER Header = {DATA_FILE_MAGIC, DATA_FILE_VERSION, STATE_CLOSE};
  Header.dwFileCount = g_Cntx.psc->dwFileCount;
  wprintf(L"Writing final header.\n");
  if(!WriteFile(g_Cntx.psc->hDataFile
    , &Header
    , sizeof(Header)
    , &g_Cntx.psc->dwBytesWritten
    , 0) || g_Cntx.psc->dwBytesWritten != sizeof(Header))
  {
    wprintf(L"Error writing header initial values\n");
  }
}

static void ReadDataFileHeader(HANDLE hDataFile, PDWORD pdwFileCount)
{
  DWORD dwBytesRead;
  FLF_HEADER dh = {DATA_FILE_MAGIC, DATA_FILE_VERSION, STATE_CLOSE};
  *pdwFileCount = 0;
  if(!ReadFile(hDataFile, &dh, sizeof(dh), &dwBytesRead, 0) || dwBytesRead != sizeof(dh))
    wprintf(L"Cannot read the file list file header.\n");
  if(dh.dwMagic != DATA_FILE_MAGIC)
    wprintf(L"Incorrect magic - not a valid file list file.\n");
  if(dh.dwVersion != DATA_FILE_VERSION)
    wprintf(L"Unsupported file list file version.\n");
  if(dh.dwState != STATE_CLOSE)
    wprintf(L"The file list file has not been closed properly.\n");

  if(dh.dwMagic == DATA_FILE_MAGIC
    || dh.dwVersion == DATA_FILE_VERSION
    || dh.dwState == STATE_CLOSE)
  {
    *pdwFileCount = dh.dwFileCount;
  }
}

static void CollectFileData()
{
  WCHAR wchBuff[0x10000];
  HKEY hKey;
  DWORD dwIdx = 0;
  SCAN_CONTEXT sc = {0};
  sc.szPathBuff = wchBuff;
  g_Cntx.psc = &sc;
  if(!GotoDataFolder())
  {
    wprintf(L"Cannot enter the current users program data directory.\n");
    return;
  }
  CreateDirectory(szScanDataDir, 0);
  if(!SetCurrentDirectory(szScanDataDir))
  {
    wprintf(L"Cannot enter the directory to store the data and index files.\n");
    return;
  }
  sc.hDataFile = CreateFile(L".data"
    , GENERIC_READ | GENERIC_WRITE
    , FILE_SHARE_READ
    , 0
    , CREATE_ALWAYS
    , FILE_ATTRIBUTE_NORMAL
    , 0);
  if(sc.hDataFile == INVALID_HANDLE_VALUE)
  {
    wprintf(L"Cannot create file list file.\n");
    return;
  }
  wprintf(L"Created file list file.\n");
  InitWriteDataFileHeader();
  if(ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, szRegFldLst, &hKey))
  {
    wprintf(L"Openning the list of directories to scan.\n");
    MergePathList(wchBuff, hKey);
    while(ERROR_SUCCESS == RegEnumKey(hKey, dwIdx ++, wchBuff, sizeof(wchBuff) / sizeof(WCHAR)))
    {
      sc.p1 = wchBuff;
      while(*++ sc.p1);
      wprintf(L"Scanning %s and subdirs...\n", wchBuff);
      ScanDir(sc.p1);
    }
    RegCloseKey(hKey);
  }
  else
    wprintf(L"Cannot find the list of folders to scan. Make sure that you enterred any.\n");

  SetFilePointer(g_Cntx.psc->hDataFile, 0, 0, FILE_BEGIN);
  WriteDataFileHeader();

  wprintf(L"Closing file list file.\n");
  CloseHandle(sc.hDataFile);
  wprintf(L"Found %d files in %d directories\n", sc.dwFileCount, sc.dwDirCount);
}

static int CalcFileMd5(PSCAN_DATA psd)
{
  BYTE   btBuff[0x10000];
  struct md5_context ctx;
  HANDLE hFile = CreateFile(psd->szPathBuff
    , GENERIC_READ
    , FILE_SHARE_READ
    , 0
    , OPEN_EXISTING
    , FILE_FLAG_SEQUENTIAL_SCAN
    , 0);

  if(hFile == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  md5_starts(&ctx);
  while(ReadFile(hFile, btBuff, sizeof(btBuff), &psd->dwBytesReadWritten, 0)
    && psd->dwBytesReadWritten)
  {
    md5_update(&ctx, btBuff, psd->dwBytesReadWritten);
  }
  CloseHandle(hFile);
  md5_finish(&ctx, psd->dwMd5Hash);
  return 0;
}

static int AddFolderForScanning(PWSTR szPathBuff)
{
  static const UINT uMsgBoxFlag[] =
  {
    MB_OK | MB_ICONEXCLAMATION
    , MB_OK | MB_ICONEXCLAMATION
    , MB_OK | MB_ICONINFORMATION
    , MB_OK | MB_ICONINFORMATION
    , MB_OK | MB_ICONINFORMATION
  };
  DWORD dwErrCode;
  HKEY hKey, hSubKey = 0;
  PWSTR p = szPathBuff;
  int iResult = 0;
  if(Location(szAddFolder, &dwErrCode, BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON))
  {
    GetCurrentDirectory(PBUFF_LEN, szPathBuff);
    for(; *p; *p ++)
    {
      if(*p == L'\\')
        *p = L'/';
    }
    p --;
    if(*p == L'/')
      *p = 0;
    if(ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, szRegFldLst, &hKey))
    {
      if(ERROR_SUCCESS == RegOpenKey(hKey, szPathBuff, &hSubKey))
        iResult = 3;
      else if(ERROR_SUCCESS == RegCreateKey(hKey, szPathBuff, &hSubKey))
        iResult = 4;
      if(hSubKey)
        RegCloseKey(hSubKey);
      RegCloseKey(hKey);
    }
  }
  else
    iResult = (int)dwErrCode;
  //0 - cannot enter
  //1 - error adding
  //2 - cancelled
  //3 - already in list
  //4 - success
  MessageBox(g_hMainWnd, szAddFldrRes[iResult], szAddTitle, uMsgBoxFlag[iResult]);
  return 1;
}

static void ReadFileData(PSCAN_DATA psd)
{
  SetFilePointer(psd->hDataFile, *psd->pdwIdxIter, 0 , FILE_BEGIN);
  ReadFile(psd->hDataFile, psd->dwMd5Hash, FDHDR_SIZE, &psd->dwBytesReadWritten, 0);
  ReadFile(psd->hDataFile, psd->szPathBuff, FNAME_SIZE(psd->wPathLen), &psd->dwBytesReadWritten, 0);
}

static void DumpData()
{
  HANDLE hDataFile, hIdxFile;
  DWORD dwBytesRead;
  DWORD dwOffset;
  WCHAR wchBuff[0x10000];
  FF_DATA ffd;
  DWORD dwFileCount, dwCountResult = 0;
  hDataFile = CreateFile(L".data", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
  if(hDataFile == INVALID_HANDLE_VALUE)
  {
    wprintf(L"Cannot open file list file to dump\n");
    return;
  }
  hIdxFile = CreateFile(L".index", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
  if(hIdxFile != INVALID_HANDLE_VALUE)
  {
    ReadDataFileHeader(hDataFile, &dwFileCount);
    if(GetFileSize(hIdxFile, 0) / sizeof(DWORD) <= dwFileCount)
    {
      dwFileCount = GetFileSize(hIdxFile, 0) / sizeof(DWORD);
      while(dwFileCount --)
      {
        if(!ReadFile(hIdxFile, &dwOffset, sizeof(DWORD), &dwBytesRead, 0)
          || dwBytesRead != sizeof(DWORD))

        {
          wprintf(L"Error reading index file.\n");
          break;
        }
        if(dwOffset != SetFilePointer(hDataFile, dwOffset, 0, FILE_BEGIN))
        {
          wprintf(L"Error seeking file list file.\n");
          break;
        }
        if(!ReadFile(hDataFile, &ffd, sizeof(ffd), &dwBytesRead, 0) ||
          dwBytesRead != sizeof(ffd) ||
          !ReadFile(hDataFile, wchBuff, (ffd.wPathLen + 1) * sizeof(WCHAR), &dwBytesRead, 0) ||
          dwBytesRead != (ffd.wPathLen + 1) * sizeof(WCHAR))
        {
          wprintf(L"Error scanning file list file.\n");
          break;
        }
        wprintf(L"%p%p->%s\n", ffd.dwSizeHigh, ffd.dwSizeLow, wchBuff);
        dwCountResult ++;
      }
    }
    else
      wprintf(L"Index and data files do not match.\n");
    CloseHandle(hIdxFile);
  }
  else
    wprintf(L"Cannot open index file.\n");
  CloseHandle(hDataFile);
}

//To be changed!
static HANDLE g_hDataFile;

static int Md5SortProc(const PDWORD pdwIdx1, const PDWORD pdwIdx2)
{
  DWORD dwBytes;
  DWORD dwMd5Hash1[4], dwMd5Hash2[4];
  SetFilePointer(g_hDataFile, *pdwIdx1, 0, FILE_BEGIN);
  ReadFile(g_hDataFile, dwMd5Hash1, sizeof(dwMd5Hash1), &dwBytes, 0);
  SetFilePointer(g_hDataFile, *pdwIdx2, 0, FILE_BEGIN);
  ReadFile(g_hDataFile, dwMd5Hash2, sizeof(dwMd5Hash2), &dwBytes, 0);
  if(dwMd5Hash1[0] > dwMd5Hash2[0])
    return 1;
  if(dwMd5Hash1[0] < dwMd5Hash2[0])
    return -1;
  if(dwMd5Hash1[1] > dwMd5Hash2[1])
    return 1;
  if(dwMd5Hash1[1] < dwMd5Hash2[1])
    return -1;
  if(dwMd5Hash1[2] > dwMd5Hash2[2])
    return 1;
  if(dwMd5Hash1[2] < dwMd5Hash2[2])
    return -1;
  if(dwMd5Hash1[3] > dwMd5Hash2[3])
    return 1;
  if(dwMd5Hash1[3] < dwMd5Hash2[3])
    return -1;
  return 0;
}

static int ScanForDuplicateFile(PSCAN_DATA psd)
{
  FILE* pHtmlFile;
  PWSTR p;
  WCHAR wchPathBuff[PBUFF_LEN];
  BOOL bLoop = FALSE;
  DWORD dwMd5Hash1[4] = {0};
  PDWORD pdwEnd = psd->pdwIdxIter = psd->pdwIdxBuff;
  pdwEnd += psd->dwFileCount;
  wchPathBuff[0] = 0;
  pHtmlFile = _wfopen(L".html", L"wt");
  fwprintf(pHtmlFile, L"<html>\r\n<body>\r\n<table border=1>\r\n");
  for(; psd->pdwIdxIter < pdwEnd; psd->pdwIdxIter ++)
  {
    ReadFileData(psd);
    if(dwMd5Hash1[0] == psd->dwMd5Hash[0]
      || dwMd5Hash1[1] == psd->dwMd5Hash[1]
      || dwMd5Hash1[2] == psd->dwMd5Hash[2]
      || dwMd5Hash1[3] == psd->dwMd5Hash[3])
      
    {
      if(!bLoop)
      {
//        wprintf(L"---%s\n", wchPathBuff);
        fwprintf(pHtmlFile, L"<tr><td><a href=\"%s\">%s</a></td>", wchPathBuff, wchPathBuff);
        p = wchPathBuff;
        while(*p++);
        while(*--p != L'/');
        *p = 0;
        fwprintf(pHtmlFile, L"<td><a href=\"%s\">***</a></td></tr>\r\n", wchPathBuff);
        bLoop = TRUE;
      }
      if(bLoop)
      {
//        wprintf(L"---%s\n", psd->szPathBuff);
        fwprintf(pHtmlFile, L"<tr><td><a href=\"%s\">%s</a></td>", psd->szPathBuff, psd->szPathBuff);
        p = psd->szPathBuff;
        while(*p++);
        while(*--p != L'/');
        *p = 0;
        fwprintf(pHtmlFile, L"<td><a href=\"%s\">***</a></td></tr>\r\n", psd->szPathBuff);
      }
    }
    else
    {
      if(bLoop)
      {
//        wprintf(L"*********************\n");
        fwprintf(pHtmlFile, L"</table>\r\n<table border=1>\r\n");
        bLoop = FALSE;
      }
    }

    memcpy(dwMd5Hash1, psd->dwMd5Hash, sizeof(dwMd5Hash1));
    lstrcpy(wchPathBuff, psd->szPathBuff);
  }
  fwprintf(pHtmlFile, L"</table>\r\n</body>\r\n</html>\r\n");
  fclose(pHtmlFile);
  ShellExecute(0, L"Open", L".html", L".", 0, SW_SHOWNORMAL);
  return 0;
}

static void GenIndexFile()
{
  HANDLE hDataFile, hIdxFile;
  DWORD dwFileCount, dwBytesRW, dwOffset;
  FF_DATA ffd;
  wprintf(L"Indexing file list file...\n");

  hDataFile = CreateFile(L".data", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
  if(hDataFile == INVALID_HANDLE_VALUE)
  {
    wprintf(L"Cannot open file list file for indexing\n");
    return;
  }
  ReadDataFileHeader(hDataFile, &dwFileCount);
  if(dwFileCount)
  {
    hIdxFile
      = CreateFile(L".index"
      , GENERIC_READ | GENERIC_WRITE
      , FILE_SHARE_READ
      , 0
      , CREATE_ALWAYS
      , FILE_ATTRIBUTE_NORMAL
      , 0);
    if(hIdxFile != INVALID_HANDLE_VALUE
      && dwFileCount * sizeof(DWORD) == SetFilePointer(hIdxFile
      , dwFileCount * sizeof(DWORD)
      , 0
      , FILE_BEGIN))
    {
      SetEndOfFile(hIdxFile);
      SetFilePointer(hIdxFile, 0, 0, FILE_BEGIN);
      while(dwFileCount --)
      {
        dwOffset = SetFilePointer(hDataFile, 0, 0, FILE_CURRENT);
        ReadFile(hDataFile, &ffd, sizeof(ffd), &dwBytesRW, 0);
        SetFilePointer(hDataFile, (ffd.wPathLen + 1) * sizeof(WCHAR), 0, FILE_CURRENT);
        WriteFile(hIdxFile, &dwOffset, sizeof(DWORD), &dwBytesRW, 0);
      }
      CloseHandle(hIdxFile);
    }
    else
      wprintf(L"Error initializing the index file.\n");
  }
  else
    wprintf(L"The file list is empty.\n");
  CloseHandle(hDataFile);
}

static PDWORD OpenIdxAsMapRW(PHANDLE phIdxFileMap)
{
  PDWORD pdwIdxFileView = 0;
  HANDLE hIdxFile
    = CreateFile(L".index"
    , GENERIC_READ | GENERIC_WRITE
    , FILE_SHARE_READ
    , 0
    , OPEN_EXISTING
    , FILE_FLAG_RANDOM_ACCESS
    , 0);
  *phIdxFileMap = 0;
  if(hIdxFile == INVALID_HANDLE_VALUE)
    return 0;

  *phIdxFileMap = CreateFileMapping(hIdxFile, 0, PAGE_READWRITE, 0, 0, 0);
  CloseHandle(hIdxFile);
  if(!*phIdxFileMap)
    return 0;

  pdwIdxFileView = (PDWORD)MapViewOfFile(*phIdxFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if(!pdwIdxFileView)
  {
    CloseHandle(*phIdxFileMap);
    *phIdxFileMap = 0;
  }
  return pdwIdxFileView;
}

static int IdxSortBySizeProc(const PDWORD pdwIdx1, const PDWORD pdwIdx2)
{
  DWORD dwBytesRead;
  FF_DATA fdd1, fdd2;
  SetFilePointer(g_Cntx.pSrtC->hDataFile, *pdwIdx1, 0, FILE_BEGIN);
  ReadFile(g_Cntx.pSrtC->hDataFile, &fdd1, sizeof(fdd1), &dwBytesRead, 0);
  SetFilePointer(g_Cntx.pSrtC->hDataFile, *pdwIdx2, 0, FILE_BEGIN);
  ReadFile(g_Cntx.pSrtC->hDataFile, &fdd2, sizeof(fdd2), &dwBytesRead, 0);

  if(fdd1.dwSizeHigh > fdd2.dwSizeHigh)
    return 1;
  if(fdd1.dwSizeHigh < fdd2.dwSizeHigh)
    return -1;
  if(fdd1.dwSizeLow > fdd2.dwSizeLow)
    return 1;
  if(fdd1.dwSizeLow < fdd2.dwSizeLow)
    return -1;
  return 0;
}

static void SortBySize()
{
  DWORD dwFileCount;
  HANDLE hIdxFileMap;
  SORT_CONTEXT SrtC = {0};
  PDWORD pdwIdxArr = OpenIdxAsMapRW(&hIdxFileMap);
  g_Cntx.pSrtC = &SrtC;
  if(pdwIdxArr)
  {
    SrtC.hDataFile
      = CreateFile(L".data"
      , GENERIC_READ
      , FILE_SHARE_READ
      , 0
      , OPEN_EXISTING
      , FILE_FLAG_RANDOM_ACCESS
      , 0);
    if(SrtC.hDataFile != INVALID_HANDLE_VALUE)
    {
      wprintf(L"Sorting indexes by size...\n");
      ReadDataFileHeader(SrtC.hDataFile, &dwFileCount);
      if(dwFileCount)
        qsort(pdwIdxArr, dwFileCount, sizeof(DWORD), (int (*)(const void*, const void*))IdxSortBySizeProc);
      else
        wprintf(L"The data file is empty.\n");
      CloseHandle(SrtC.hDataFile);
    }
    UnmapViewOfFile(pdwIdxArr);
    CloseHandle(hIdxFileMap);
  }
  else
  {
    wprintf(L"Cannot map the index file.\n");
    DeleteFile(L".index");
  }
}

static void ReadFdd(HANDLE hIdxFile, HANDLE hDataFile, PFF_DATA pFdd, PDWORD pdwIdx, PBOOL pbResult)
{
  WCHAR wchBuff[0x10000];
  DWORD dwBytesRead;
  
  if(!*pbResult)
    return;
  
  *pbResult =
    ReadFile(hIdxFile, pdwIdx, sizeof(DWORD), &dwBytesRead, 0)
    && dwBytesRead == sizeof(DWORD)
    && *pdwIdx == SetFilePointer(hDataFile, *pdwIdx, 0, FILE_BEGIN)
    && ReadFile(hDataFile, pFdd, sizeof(FF_DATA), &dwBytesRead, 0)
    && dwBytesRead == sizeof(FF_DATA);

  ReadFile(hDataFile, wchBuff, (pFdd->wPathLen + 1) * sizeof(WCHAR), &dwBytesRead, 0);
  wprintf(L"+++%s\n", wchBuff);
}

static void WriteIdx(HANDLE hIdxFile, DWORD dwIdx, PBOOL pbResult)
{
  DWORD dwBytesWritten;

  if(!*pbResult)
    return;
  
  *pbResult = WriteFile(hIdxFile, &dwIdx, sizeof(dwIdx), &dwBytesWritten, 0)
    && dwBytesWritten == sizeof(dwIdx);
}

static void SetIdxPos(HANDLE hIdxFile, DWORD dwPos, PBOOL pbResult)
{
  if(!*pbResult)
    return;

  *pbResult = dwPos == SetFilePointer(hIdxFile, dwPos, 0, FILE_BEGIN);
}

static void ExtractDuplicateSize()
{
  HANDLE hIdxFile, hDataFile;
  DWORD dwFileCount;
  DWORD dwIdx1 = 0, dwIdx2 = 0;
  DWORD dwWritePos = 0;
  DWORD dwReadPos = 0;
  BOOL bLoop = FALSE;
  BOOL bSuccess = TRUE;
  FF_DATA fdd1, fdd2;
  wprintf(L"Scanning for same size files.\n");
  hDataFile = CreateFile(L".data"
    , GENERIC_READ
    , FILE_SHARE_READ
    , 0
    , OPEN_EXISTING
    , FILE_FLAG_RANDOM_ACCESS
    , 0);
  if(INVALID_HANDLE_VALUE != hDataFile)
  {
    ReadDataFileHeader(hDataFile, &dwFileCount);
    if(dwFileCount)
    {
      hIdxFile = CreateFile(L".index"
        , GENERIC_READ | GENERIC_WRITE
        , FILE_SHARE_READ
        , 0
        , OPEN_EXISTING
        , FILE_FLAG_RANDOM_ACCESS
        , 0);
      if(hIdxFile != INVALID_HANDLE_VALUE)
      {
        ReadFdd(hIdxFile, hDataFile, &fdd1, &dwIdx1, &bSuccess);
        while(bSuccess && -- dwFileCount)
        {
          ReadFdd(hIdxFile, hDataFile, &fdd2, &dwIdx2, &bSuccess);
          if(bSuccess && fdd1.dwSizeHigh == fdd2.dwSizeHigh && fdd1.dwSizeLow == fdd2.dwSizeLow)
          {
            dwReadPos = SetFilePointer(hIdxFile, 0, 0, FILE_CURRENT);
            SetIdxPos(hIdxFile, dwWritePos, &bSuccess);
            if(bLoop)
            {
              WriteIdx(hIdxFile, dwIdx2, &bSuccess);
            }
            else
            {
              WriteIdx(hIdxFile, dwIdx1, &bSuccess);
              WriteIdx(hIdxFile, dwIdx2, &bSuccess);
              bLoop = TRUE;
            }
            dwWritePos = SetFilePointer(hIdxFile, 0, 0, FILE_CURRENT);
            SetIdxPos(hIdxFile, dwReadPos, &bSuccess);
          }
          else
          {
            bLoop = FALSE;
          }
          fdd1.dwSizeHigh = fdd2.dwSizeHigh;
          fdd1.dwSizeLow = fdd2.dwSizeLow;
        }
        SetIdxPos(hIdxFile, dwWritePos, &bSuccess);
        SetEndOfFile(hIdxFile);
        CloseHandle(hIdxFile);
      }
    }
    CloseHandle(hDataFile);
  }
  if(bSuccess)
    wprintf(L"Found %d files with same size\n", dwWritePos / sizeof(DWORD));
  else
  {
    wprintf(L"Error scanning for duplicate size files.\n");
    DeleteFile(L".data");
    DeleteFile(L".index");
  }
}

static int StartScanning(PWSTR szPathBuff)
{
  CollectFileData();
  GenIndexFile();
  SortBySize();
  ExtractDuplicateSize();
  DumpData();
  ShellExecute(0, L"open", L".", 0, 0, SW_NORMAL);
  return 0;
}

static int PathSel(PWSTR szPathBuff)
{
  HKEY hKey;
  int iResult = 0;
  const static UINT uFlagArr[] =
  {
    MB_OK | MB_ICONERROR, MB_OK | MB_ICONINFORMATION
  };
  if(IDYES != MessageBox(g_hMainWnd, szPathBuff, szDelConf, MB_YESNO | MB_ICONQUESTION))
    return 0;
  if(ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, szRegFldLst, &hKey))
  {
    iResult = ERROR_SUCCESS == RegDeleteKey(hKey, szPathBuff);
    RegCloseKey(hKey);
  }
  MessageBox(g_hMainWnd, szPathBuff, szErrDelPath[iResult], uFlagArr[iResult]);
  return 0;
}

static UINT PathExistsCheckProc(PREG_MENU prm)
{
  DWORD dwAttr = GetFileAttributes(prm->mb.szItemTextBuff);
  return dwAttr == (~0) ? MF_GRAYED : (dwAttr & FILE_ATTRIBUTE_DIRECTORY ? 0 : MF_GRAYED);
}

static UINT FileExistsCheckProc(PREG_MENU prm)
{
  DWORD dwAttr = GetFileAttributes(prm->mb.szItemTextBuff);
  return dwAttr == (~0) ? MF_GRAYED : (dwAttr & FILE_ATTRIBUTE_DIRECTORY ? MF_GRAYED : 0);
}

static int FileSelProc(PREG_MENU prm)
{
  wprintf(L"+++%s\n", prm->mb.szItemTextBuff);
  return 1;
}

static int Back()
{
  return 2;
}

static int DupGroupSel(PWSTR szPath)
{
/*  static LPCWSTR szTextArr[] =
  {
    0,
      L"&<< Back"
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      Back
  };

  static REG_MENU rm =
  {
    MT_REG_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
      , FileSelProc
      , PBUFF_LEN
      , 0
      , 0
      , FileExistsCheckProc
  };

  wprintf(L"---%s\n", szPath);
  
  if(ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, szRegDupFiles, &rm.hKeyRoot))
    return 0;
  rm.mb.szItemTextBuff = szPath;
  do
  {
    lstrcpy(wchPathBuff, pfs->rm.mb.szItemTextBuff);
    fs.bLoop = TRUE;
    PopupMenu((PMENU_TYPE)&fs);
  }while(fs.rm.mb.SelProc(&fs));
  RegCloseKey(fs.rm.hKeyRoot);*/
  return 0;
}

static int ScanResults(PWSTR szPathBuff)
{
  /*
  static LPCWSTR szTextArr[] =
  {
    0,
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc
  };
  
  static FILE_SEL fs =
  {
    MT_REG_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
      , DupGroupSel
      , PBUFF_LEN
      , 0
      , HKEY_CURRENT_USER
      , FileExistsCheckProc
  };

  fs.rm.mb.szItemTextBuff = szPathBuff;
  do
  {
    lstrcpy(szPathBuff, szRegDupFiles);
    fs.bLoop = FALSE;
    PopupMenu((PMENU_TYPE)&fs);
  }while(fs.rm.mb.SelProc(&fs));*/
  return 1;
}

static int ScanDuplicate()
{
  WCHAR wchPathBuff[PBUFF_LEN];

  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Add folder for scanning...",
      L"&2. Start scanning...",
      0,
      L"&3. Scan results...",
      0
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
    AddFolderForScanning,
    StartScanning,
    0,
    ScanResults,
    0
  };

  static REG_MENU rm =
  {
    MT_REG_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , 0
      , DispatchTable
      , PathSel
      , PBUFF_LEN
      , 0
      , HKEY_CURRENT_USER
      , PathExistsCheckProc
    };

  rm.mb.szItemTextBuff = wchPathBuff;
  do
  {
    lstrcpy(wchPathBuff, szRegFldLst);
    rm.mb.SelProc = PathSel;
    PopupMenu((PMENU_TYPE)&rm);
  }while(rm.mb.SelProc(wchPathBuff));
  return 0;
}

static int LocalSynch()
{
  return 0;
}

static int RemoteSynch()
{
  return 0;
}

int ScanFiles()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Find duplicate files...",
      L"&2. Local synchronization...",
      L"&3. Remote synchronization..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      ScanDuplicate,
      LocalSynch,
      RemoteSynch
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
