#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <malloc.h>

#define TXT_BUFF_SIZE           4096

struct STR_TBL
{
    PCWSTR szTitle; //L"Report Tree"
    PCWSTR szAdded; //L"Added %s"
    PCWSTR szScanning; //L"Scanning..."
    PCWSTR szErrFatal; //L"Fatal error! Cannot continue"
    PCWSTR szDirScanOp; //L"Directory Scan Operations"
    //5
    PCWSTR szRootToScan; //L"Root to Scan"
    PCWSTR szLogs; //L"Logs"
    PCWSTR szSfdf; //L">> Scan for Duplicate Files"
    PCWSTR szSffd; //L">> Scan for Different Files"
    PCWSTR szDiff; //L"Duplicate Files Found"
    //10
    PCWSTR szNewer; //L"Newer Than in Other Directory"
    PCWSTR szBigger; //L"Bigger Than in Other Directory"
    PCWSTR szAbsent; //L"Absent in Other Directory"
    PCWSTR szScanInProg; //L"The scanning is in progress!"
    PCWSTR szWaitScanEnd; //L"The scanning is in progress. Wait till the end of scannig and try again."
    //15
    PCWSTR szCleanAll; //L"Clean All"
    PCWSTR szAddDirFile; //L"Add Object"
    PCWSTR szOpenInExplorer; //L"Open in explorer"
    PCWSTR szRemoveFromList; //L"Remove from list"
    PCWSTR szCleanupLog; //
    //20
    PCWSTR szCannotOpenDataFile; //"Cannot open data file. Make sure that the media you are running from is not write protected"
    PCWSTR szCollectingNames; //L"Collecting names: %s";
    PCWSTR szCannotScanDir; //L"Cannot scan: %s"
    PCWSTR szFileCount; //L"Found %d files"
    PCWSTR szNeededAtLeast2; //L"At least two file are needed to continue"
    //25
    PCWSTR szCollectSize; //L"Collecting file size information"
    PCWSTR szNoCollectInfo; //L"No collected file information. Giving up.";
    PCWSTR szFatalErr; //L"FATAL ERROR!"
    PCWSTR szErrOpenFilesFile; //L"%s Cannot open file list file!"
    PCWSTR szErrFilesMapping; //L"%s Cannot map file list file!"
    //30
    PCWSTR szErrFileSize; //L"Cannot obtain size of file: %s\n"
    PCWSTR szSortBySize; //L"Sorting files by size (%d files) ...";
    PCWSTR szCalcSameSizeCount; //L"Calculating the number of same size files (%d files total)"
    PCWSTR szFoundSameSize; //L"Found %d files with same size in groups"
    PCWSTR szLogExtractSameSize; //L"Extracting same size file groups"
    //35
    PCWSTR szCalcHash; //L"Calculating file hash codes (%d files)"
    PCWSTR szErrHash; //L"Error calculating hash. Cannot open file %s"
    PCWSTR szSortByHash; //L"Sorting files by hash code (%d files)"
    PCWSTR szExtractSameHash; //L"Extracting same hash file list ..."
    PCWSTR szDuplicateFound; //L"Found %d groups of %d total duplicate files"
    //40
    PCWSTR szNoExt; //L"<No extension>";
    PCWSTR szOpenContainer; //L"&Containing Folder"
    PCWSTR szOpen; //L"&Open";
    PCWSTR szFileProp; //L"&Properties..."
    PCWSTR szDeleteFile; //L"&Delete(!)"
    //45
    PCWSTR szDeleting; //L"Deleting...";
    PCWSTR szCancelled; //L"Canceled."
    PCWSTR szDeleted; //L"Deleted file: %s";
    PCWSTR szErrDel; //L"Cannot delete the file!";
    PCWSTR szFmtErrDel; //L"%s %s";
    //50
    PCWSTR szNoInput; //L"No input"
    PCWSTR szDragDrop; //L"Drag-drop or add objects double clicking the scan root and try again"
    PCWSTR szInvalidInput; //L"Invalid input";
    PCWSTR szOnly2Dirs; //L"The input for this scan must include exactly 2 directory paths";
    PCWSTR szNoInputLog; //L"%s. %s"
    //55
    PCWSTR szCmpDirs; //L"Comparing recursevely %s and %s"
    PCWSTR szErrOpenToCmp; //L"Cannot open file to compare: %s"
    PCWSTR szErrFileTime; //L"Cannot get file time to compare: %s"
    PCWSTR szCopyAll; //L"&Copy All...";
    PCWSTR szEmptyGroup; //L"&Empty Group"
    //60
    PCWSTR szRemoveGroup; //L"&Remove Group"
    PCWSTR szCopyToOther; //L"&Copy to Other Folder"
    PCWSTR szPause; //L"Paused"
    PCWSTR szProgress; //L"Scanning... %c";
    PCWSTR szCleanUpTree; //L"Cleaning up the tree.";
    //65
    PCWSTR szCannotContinue; //L"Cannot continue"
    PCWSTR szErrCopy; //L"Error copying files. Check free space and access rights."
    PCWSTR szCopyAborted; //L"Copying aborted by user."
    PCWSTR szCopyingFromTo; //L"Copy of %s to %s"
    PCWSTR szErrCopyNoSrc; //L"Cannot copy. No such file: %s";
    //70
    PCWSTR szErrCopyDirFile; //L"Replacing directory with file is restricted. Skipping."
    PCWSTR szErrCopyFileDir; //L"Replacing file with directory is restricted. Skipping."
};


#if RT_LANG == EN

static struct STR_TBL g_str =
{
    L"Report Tree" //szTitle
        , L"Added %s" //szAdded
        , L"Scanning..." //szScanning
        , L"Fatal error! Cannot continue" //szErrFatal
        , L"Directory Scan Operations" //szDirScanOp
        //5
        , L"Root to Scan" //szRootToScan
        , L"Logs" //szLogs
        , L">> Scan for Duplicate Files" //szSfdf
        , L">> Scan for Different Files" //szSffd
        , L"Duplicate Files Found" //szDiff
        //10
        , L"Newer Than in Other Directory" //szNewer
        , L"Bigger Than in Other Directory" //szBigger
        , L"Absent in Other Directory" //szAbsent
        , L"The scanning is in progress!" //szScanInProg
        , L"The scanning is in progress. Do you want to stop?." //szWaitScanEnd
        //15
        , L"&Clean All" //szCleanAll
        , L"&Add Object" //szAddDirFile
        , L"Open in Explorer" //szOpenInExplorer
        , L"&Remove from List" //szRemoveFromList
        , L"&Cleanup Logs" //szCleanupLog
        //20
        , L"Cannot open data file. Make sure that the media you are running from is not write protected" //szCannotOpenDataFile
        , L"Collecting names: %s" //szCollectingNames
        , L"Cannot scan: %s" //szCannotScanDir
        , L"Found %d files" //szFileCount
        , L"At least two file are needed to continue" //szNeededAtLeast2
        //25
        , L"Collecting file size information" //szCollectSize
        , L"No collected file information. Giving up." //szNoCollectInfo
        , L"FATAL ERROR! " //szFatalErr
        , L"%s Cannot open file list file!" //szErrOpenFilesFile
        , L"%s Cannot map file list file!" //szErrFilesMapping
        //30
        , L"Cannot obtain size of file: %s\n" //szErrFileSize
        , L"Sorting files by size (%d files)" //szSortBySize
        , L"Calculating the number of same size files (%d files total)" //szCalcSameSizeCount
        , L"Found %d files with same size in groups" //szFoundSameSize
        , L"Extracting same size file groups" //szLogExtractSameSize
        //35
        , L"Calculating file hash codes (%d files)" //szCalcHash
        , L"Error calculating hash. Cannot open file %s" //szErrHash
        , L"Sorting files by hash code (%d files)" //szSortByHash
        , L"Extracting same hash file list ..." //szExtractSameHash
        , L"Found %d groups of %d total duplicate files" //szDuplicateFound
        //40
        , L"<No extension>" //szNoExt
        , L"&Containing Folder" //szOpenContainer
        , L"&Open" //szOpen
        , L"&Properties..." //szFileProp
        , L"&Delete(!)" //szDeleteFile
        //45
        , L"Deleting..." //szDeleting
        , L"Canceled." //szCancelled
        , L"Deleted file: %s" //szDeleted
        , L"Cannot delete the file" //szErrDel
        , L"%s %s" //szFmtErrDel
        //50
        , L"No input" //szNoInput
        , L"Drag-drop or add objects double clicking the scan root and try again" //szDragDrop
        , L"Invalid input" //szInvalidInput
        , L"The input for this scan must include exactly 2 directory paths" //szOnly2Dirs
        , L"%s. %s" //szNoInputLog
        //55    
        , L"Comparing recursevely %s and %s" //szCmpDirs
        , L"Cannot open file to compare: %s" //szErrOpenToCmp
        , L"Cannot get file time to compare: %s" //szErrFileTime
        , L"&Copy All" //szCopyAll
        , L"&Empty Group" //szEmptyGroup
        //60
        , L"&Remove Group" //szRemoveGroup
        , L"&Copy to Other Folder" //szCopyToOther
        , L"Paused" //szPause
        , L"Scanning... %c" //szProgress
        , L"Cleaning up the tree." //szCleanUpTree
        //65
        , L"Cannot continue" //szCannotContinue
        , L"Error copying files. Check free space and access rights." //szErrCopy
        , L"Copying aborted by user." //szCopyAborted
        , L"Copy of %s to %s" //szCopyingFromTo
        , L"Cannot copy. No such file: %s" //szErrCopyNoSrc
        //70
        , L"Replacing directory with file is restricted. Skipping." //szErrCopyDirFile
        , L"Replacing file with directory is restricted. Skipping." //szErrCopyFileDir
};


#endif // RT_LANG == EN

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 total[2];
    uint32 state[8];
    uint8 buffer[64];
}sha256_context;

void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, uint8 *input, uint32 length );
void sha256_finish( sha256_context *ctx, uint8 digest[32] );

void sha256_starts( sha256_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x6A09E667;
    ctx->state[1] = 0xBB67AE85;
    ctx->state[2] = 0x3C6EF372;
    ctx->state[3] = 0xA54FF53A;
    ctx->state[4] = 0x510E527F;
    ctx->state[5] = 0x9B05688C;
    ctx->state[6] = 0x1F83D9AB;
    ctx->state[7] = 0x5BE0CD19;
}

#define	Ch(x, y, z)	((z) ^ ((x) & ((y) ^ (z))))
#define	Maj(x, y, z)	(((x) & (y)) ^ ((z) & ((x) ^ (y))))
#define	Rot32(x, s)	(((x) >> s) | ((x) << (32 - s)))
#define	SIGMA0(x)	(Rot32(x, 2) ^ Rot32(x, 13) ^ Rot32(x, 22))
#define	SIGMA1(x)	(Rot32(x, 6) ^ Rot32(x, 11) ^ Rot32(x, 25))
#define	sigma0(x)	(Rot32(x, 7) ^ Rot32(x, 18) ^ ((x) >> 3))
#define	sigma1(x)	(Rot32(x, 17) ^ Rot32(x, 19) ^ ((x) >> 10))

static void sha256_process( sha256_context *ctx, uint8 data[64] )
{
    static const uint32 SHA256_K[64] =
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32 a, b, c, d, e, f, g, h, t, T1, T2, W[64], *H = ctx->state;
    uint8* cp = data;

    for (t = 0; t < 16; t++, cp += 4)
        W[t] = (cp[0] << 24) | (cp[1] << 16) | (cp[2] << 8) | cp[3];

    for (t = 16; t < 64; t++)
        W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

    a = H[0]; b = H[1]; c = H[2]; d = H[3];
    e = H[4]; f = H[5]; g = H[6]; h = H[7];

    for (t = 0; t < 64; t++)
    {
        T1 = h + SIGMA1(e) + Ch(e, f, g) + SHA256_K[t] + W[t];
        T2 = SIGMA0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }
    H[0] += a; H[1] += b; H[2] += c; H[3] += d;
    H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

void sha256_update( sha256_context *ctx, uint8 *input, uint32 length )
{
    uint32 left, fill;

    if( ! length ) return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < length )
        ctx->total[1]++;

    if( left && length >= fill )
    {
        memcpy( (void *) (ctx->buffer + left), (void *) input, fill );
        sha256_process( ctx, ctx->buffer );
        length -= fill;
        input  += fill;
        left = 0;
    }

    while( length >= 64 )
    {
        sha256_process( ctx, input );
        length -= 64;
        input  += 64;
    }

    if( length )
        memcpy( (void *) (ctx->buffer + left), (void *) input, length );
}

static uint8 sha256_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sha256_finish( sha256_context *ctx, uint8 digest[32] )
{
    uint32 last, padn, high, low, t, *p;
    uint8 msglen[8], *cp;

    high = ( ctx->total[0] >> 29 ) | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    msglen[0] = (uint8)(high >> 24);
    msglen[1] = (uint8)(high >> 16);
    msglen[2] = (uint8)(high >> 8);
    msglen[3] = (uint8)high;
    msglen[4] = (uint8)(low >> 24);
    msglen[5] = (uint8)(low >> 16);
    msglen[6] = (uint8)(low >> 8);
    msglen[7] = (uint8)low;

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha256_update( ctx, sha256_padding, padn );
    sha256_update( ctx, msglen, 8 );

    cp = digest;
    p = ctx->state;
    for(t = 0; t < 8; t ++, p ++, cp += 4)
    {
        cp[0] = (uint8)(*p >> 24);
        cp[1] = (uint8)(*p >> 16);
        cp[2] = (uint8)(*p >> 8);
        cp[3] = (uint8)*p;
    }
}

#define FS_MAXFILECOUNT 32768 //limited by stack size

struct FS_GLB
{
    DWORD dwFileCount;
    WIN32_FIND_DATA fd;
} static g_fs;

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

static void   FsNormalizePath(PWSTR szPath);
static PWSTR  FsSkipNetworkBacklashes(PWSTR szPath);
static PCWSTR FsGetExtension(PCWSTR szPath);
static PWSTR  FsSelectDirAndFile(HWND, PCWSTR, PWSTR);
static int    FsCopy(HWND, PCWSTR szDest, PCWSTR szSrc);
static void   FsInitScan();
static void   FsCollectFileNames(PWSTR szScanRoot);
static void   FsSfdf();
static void   FsSffd(PWSTR szScanRoot0, PWSTR szScanRoot1);
static void   FsDelTempFiles();

static void TvAddSfdfGroup(PCWSTR szPath);
static void TvAddSfdfItem(PCWSTR szPath);
static void TvAddSffdNewer(PCWSTR szPath, PWSTR szRelPath);
static void TvAddSffdBigger(PCWSTR szPath, PWSTR szRelPath);
static void TvAddSffdAbsent(PCWSTR szPath, PWSTR szRelPath);
static void TvSortLastGroup();
static void TvProgressTitle();
static int TvCheckCancel();
static void TvAddLogLine(PCWSTR szFmt, ...);

static void RtPassMsg()
{
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } 
}

static void FsSlowOpMsgPass()
{
    static DWORD dwOldTicks;
    DWORD dwTicks = GetTickCount();
    if(dwTicks - dwOldTicks > 100)
    {
        TvProgressTitle();
        RtPassMsg();
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
        TvAddLogLine(g_str.szErrCopyNoSrc, szSrc);
        return 0;
    }
    if(dwAttrDest != 0xFFFFFFFF)
    {
        if((dwAttrDest & FILE_ATTRIBUTE_DIRECTORY) && !(dwAttrSrc & FILE_ATTRIBUTE_DIRECTORY))
        {
            TvAddLogLine(g_str.szErrCopyDirFile);
            return 0;
        }
        if(!(dwAttrDest & FILE_ATTRIBUTE_DIRECTORY) && (dwAttrSrc & FILE_ATTRIBUTE_DIRECTORY))
        {
            TvAddLogLine(g_str.szErrCopyFileDir);
            return 0;
        }
    }
    TvAddLogLine(g_str.szCopyingFromTo, szSrc, szDest);
    int iRet = SHFileOperation(&sh);
    if(iRet)
    {
        TvAddLogLine(g_str.szErrCopy, g_str.szCannotContinue, MB_OK | MB_ICONERROR);
        return 0;
    }
    if(sh.fAnyOperationsAborted)
    {
        TvAddLogLine(g_str.szCopyAborted);
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
    if(TvCheckCancel())
        return;
    if(GetFileAttributes(psc->szScanRoot) & FILE_ATTRIBUTE_DIRECTORY)
    {
        *szEnd ++ = L'*';
        *szEnd = 0;
        hFind = FindFirstFile(psc->szScanRoot, &g_fs.fd);
        if(hFind == INVALID_HANDLE_VALUE)
        {
            TvAddLogLine(g_str.szCannotScanDir, psc->szScanRoot);
            return;
        }
        if(*g_fs.fd.cFileName == L'.')//0x2E)
        {
            FindNextFile(hFind, &g_fs.fd);
            if(!FindNextFile(hFind, &g_fs.fd))
            {
                FindClose(hFind);
                return;
            }
        }
        szEnd --;
        do
        {
            p0 = szEnd;
            p1 = g_fs.fd.cFileName;
            while(*p1)
                *p0 ++ = *p1 ++;
            *p0 = 0;
            if(g_fs.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                *p0 ++ = L'\\';
                *p0 = 0;
            }
            FsScanCollectNames(psc, p0);
            FsSlowOpMsgPass();
        }while(FindNextFile(hFind, &g_fs.fd));
        FindClose(hFind);
    }
    else
    {
        WriteFile(psc->hFile, &g_fs.fd.nFileSizeHigh, 2 * sizeof(DWORD), &dwBytesWritten, 0);
        dwBytes = szEnd - psc->szScanRoot;
        dwBytes *= sizeof(WCHAR);
        dwBytes += sizeof(WCHAR);
        WriteFile(psc->hFile, psc->szScanRoot, dwBytes, &dwBytesWritten, 0);
        g_fs.dwFileCount ++;
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
        TvAddLogLine(g_str.szCannotOpenDataFile);
        return;
    }
    TvAddLogLine(g_str.szCollectingNames, szScanRoot);
    SetFilePointer(sc.hFile, 0, 0, FILE_END);
    while(*p) p++;
    sc.szScanRoot = szScanRoot;
    FsScanCollectNames(&sc, p);
    CloseHandle(sc.hFile);
}

static void FsCollectIdx(struct FileEntry* pfe, struct FileEntry** pIdxFile)
{
    PCWSTR szPath;
    DWORD dwCount = g_fs.dwFileCount;
    if(TvCheckCancel())
        return;
    TvAddLogLine(g_str.szCollectSize);
    while(!TvCheckCancel() && dwCount --)
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
    DWORD dwCount = g_fs.dwFileCount;
    *pdwHashCount = 0;
    if(TvCheckCancel())
        return;
    TvAddLogLine(g_str.szSortBySize, g_fs.dwFileCount);
    qsort(pIdxFile, g_fs.dwFileCount, sizeof(struct FileEntry*), (int (*)(const void*, const void*))FsSizeCmp);
    TvAddLogLine(g_str.szCalcSameSizeCount, g_fs.dwFileCount);
    pFfd2 = pFfd1 = pIdxFile;
    while(!TvCheckCancel() && -- dwCount)
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
    TvAddLogLine(g_str.szFoundSameSize, *pdwHashCount);
}

static void FsExtractSameSize(struct FileEntry** pIdxFile, struct IdxSha256* pIdxHash)
{
    struct FileEntry **pFfd1, **pFfd2;
    int iLoop = 1;
    DWORD dwCount = g_fs.dwFileCount;
    if(TvCheckCancel())
        return;
    pFfd2 = pFfd1 = pIdxFile;
    TvAddLogLine(g_str.szLogExtractSameSize);
    while(!TvCheckCancel() && -- dwCount)
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
    if(TvCheckCancel())
        return;
    TvAddLogLine(g_str.szCalcHash, dwHashCount);
    while(!TvCheckCancel() && dwHashCount --)
    {
        hFile = CreateFile(pIdxHash->pfe->wchPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            TvAddLogLine(g_str.szErrHash, pIdxHash->pfe->wchPath);
            memset(pIdxHash->sh.bt, 0, 8);
            pIdxHash->pfe = 0;
            pIdxHash ++;
            continue;
        }
        sha256_starts(&ctx);
        while(!TvCheckCancel() && ReadFile(hFile, btBuff, sizeof(btBuff), &dwBytesR, 0))
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
    if(TvCheckCancel())
        return;
    TvAddLogLine(g_str.szSortByHash, dwHashCount);
    qsort(pIdxHash, dwHashCount, sizeof(struct IdxSha256), (int (*)(const void*, const void*))FsSha256Cmp);
    TvAddLogLine(g_str.szExtractSameHash);
    struct IdxSha256 *pSh1, *pSh2;
    int iLoop = 1;
    DWORD dwGroups = 0, dwTotalDuplicate = 0;
    while(!pIdxHash->pfe) ++pIdxHash, --dwHashCount;
    pSh2 = pSh1 = pIdxHash;
    while(!TvCheckCancel() && -- dwHashCount)
    {
        pSh2 ++;
        if(!FsSha256Cmp(pSh1, pSh2))
        {
            if(iLoop)
            {
                iLoop = 0;
                TvAddSfdfGroup(pSh1->pfe->wchPath);
                TvAddSfdfItem(pSh1->pfe->wchPath);
                dwGroups ++;
                dwTotalDuplicate ++;
            }
            TvAddSfdfItem(pSh2->pfe->wchPath);
            dwTotalDuplicate ++;
        }
        else
        {
            if(!iLoop)
            {
                TvSortLastGroup();
            }
            iLoop = 1;
        }
        pSh1 = pSh2;
        FsSlowOpMsgPass();
    }
    TvAddLogLine(g_str.szDuplicateFound, dwGroups, dwTotalDuplicate);
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
    TvAddLogLine(g_str.szFileCount, g_fs.dwFileCount);
    if(g_fs.dwFileCount >= 2)
    {
        hFile = CreateFile(L".files", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
        if(hFile != INVALID_HANDLE_VALUE)
        {
            hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
            CloseHandle(hFile);
            if(hMap)
            {
                pfe = (struct FileEntry*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
                if(pfe)
                {
                    dwBytes = g_fs.dwFileCount * sizeof(struct FileEntry*);
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
                    UnmapViewOfFile((PVOID)pfe);
                }
                else
                {
                    TvAddLogLine(g_str.szErrFilesMapping, g_str.szFatalErr);
                }
                CloseHandle(hMap);
            }
            else
                TvAddLogLine(g_str.szNoCollectInfo);
        }
        else
            TvAddLogLine(g_str.szErrOpenFilesFile, g_str.szFatalErr);
    }
    else
        TvAddLogLine(g_str.szNeededAtLeast2);
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
        TvAddLogLine(g_str.szErrOpenToCmp, pfst->szFile);
        return 0;
    }
    pfst->dwSizeLow = GetFileSize(hFile, &pfst->dwSizeHigh);
    if(!GetFileTime(hFile, 0, 0, &pfst->ftWrite))
    {
        TvAddLogLine(g_str.szErrFileTime, pfst->szFile);
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
        TvAddSffdAbsent(psc->szScanRoot0, psc->szRelPath0);
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
            TvAddSffdBigger(psc->szScanRoot0, psc->szRelPath0);

        if(fst0.ftWrite.dwHighDateTime > fst1.ftWrite.dwHighDateTime
                || (fst0.ftWrite.dwHighDateTime == fst1.ftWrite.dwHighDateTime
                    && fst0.ftWrite.dwLowDateTime > fst1.ftWrite.dwLowDateTime))
            TvAddSffdNewer(psc->szScanRoot0, psc->szRelPath0);
        return 0;
    }
    return 0;
}

static void FsScanDiffFiles(struct FsScanDiffFiles* psc, PWSTR szEnd0, PWSTR szEnd1)
{
    static PWSTR p0, p1, p2;
    HANDLE hFind;
    DWORD dwAttr0;
    if(TvCheckCancel())
        return;
    dwAttr0 = GetFileAttributes(psc->szScanRoot0);
    if(FsProcessFileDiff(psc, dwAttr0))
        return;
    ++ g_fs.dwFileCount;
    if(dwAttr0 & FILE_ATTRIBUTE_DIRECTORY)
    {
        *szEnd0 ++ = L'*';
        *szEnd0 = 0;
        hFind = FindFirstFile(psc->szScanRoot0, &g_fs.fd);
        if(hFind == INVALID_HANDLE_VALUE)
        {
            TvAddLogLine(g_str.szCannotScanDir, psc->szScanRoot0);
            return;
        }
        if(*g_fs.fd.cFileName == '.')//0x2E)
        {
            FindNextFile(hFind, &g_fs.fd);
            if(!FindNextFile(hFind, &g_fs.fd))
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
            p1 = g_fs.fd.cFileName;
            while(*p1)
                *p2 ++ = *p0 ++ = *p1 ++;

            *p2 = *p0 = 0;
            if(g_fs.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                *p2 ++ = *p0 ++ = L'\\';
                *p2 = *p0 = 0;
            }
            FsScanDiffFiles(psc, p0, p2);
            FsSlowOpMsgPass();
        }while(!TvCheckCancel() && FindNextFile(hFind, &g_fs.fd));
        FindClose(hFind);
    }
}

static void FsSffd(PWSTR szScanRoot0, PWSTR szScanRoot1)
{
    struct FsScanDiffFiles sc;
    g_fs.dwFileCount = 0;
    sc.szRelPath0 = sc.szScanRoot0 = szScanRoot0;
    sc.szRelPath1 = sc.szScanRoot1 = szScanRoot1;
    while(*++sc.szRelPath0);
    while(*++sc.szRelPath1);
    FsScanDiffFiles(&sc, sc.szRelPath0, sc.szRelPath1);
}

static void FsInitScan()
{
    g_fs.dwFileCount = 0;
}


static UINT MnuPopupMenu(HWND hWnd, UINT uCount, ...)
{
    va_list pArgList;
    POINT pt;
    RECT rc;
    HMENU hMenu;
    UINT uCmd = 0;
    HTREEITEM hSelItem = TreeView_GetSelection(hWnd);
    if(!hSelItem)
        return 0;
    hMenu = CreatePopupMenu();
    if(!hMenu)
        return 0;
    va_start(pArgList, uCount);
    for(uCmd = 0; uCmd < uCount; uCmd ++)
    {
        PCWSTR szText = va_arg(pArgList, PCWSTR);
        AppendMenu(hMenu, szText ? MF_STRING : MF_SEPARATOR, uCmd + 1, szText);
    }
    va_end (pArgList);
    if(TreeView_GetItemRect(hWnd, hSelItem, &rc, TRUE))
    {
        pt.x = (rc.left + rc.right) >> 1;
        pt.y = (rc.top + rc.bottom) >> 1;
        ClientToScreen(hWnd, &pt);
        uCmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, 0);
    }
    DestroyMenu(hMenu);
    return uCmd;
}

enum TV_STATE
{
    TVS_IDLE,
    TVS_SCANNING,
    TVS_CANCEL,
    TVS_LAST
};

struct TV_GLB
{
    HWND hTree;
    WNDPROC pOldProc;
    HTREEITEM hScanRoot;
    HTREEITEM hSfdfStartScan;
    HTREEITEM hSfdfScanResult;
    HTREEITEM hSffdStartScan;
    HTREEITEM hSffdNew;
    HTREEITEM hSffdBig;
    HTREEITEM hSffdAbsent;
    HTREEITEM hSffdNewRoots[2];
    HTREEITEM hSffdBigRoots[2];
    HTREEITEM hSffdAbsentRoots[2];
    HTREEITEM hScanLog;
    HTREEITEM hLastGroup;
    enum TV_STATE st;
};

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

static struct TVI_CLASS g_EvtTbl[F_TV_LAST] =
{
    [F_TV_EMPTY].fTvDblClick                 = fTvEmpty,
    [F_TV_EMPTY].fTvKbdEnter                 = fTvEmpty,

    [F_TV_SCANROOT].fTvDblClick              = fTvScanRoot,
    [F_TV_SCANROOT].fTvKbdEnter              = fTvScanRoot,

    [F_TV_SCANPATH].fTvDblClick              = fTvScanPath,
    [F_TV_SCANPATH].fTvKbdEnter              = fTvScanPath,

    [F_TV_SFDF_GROUP].fTvDblClick            = fTvSfdfGroup,
    [F_TV_SFDF_GROUP].fTvKbdEnter            = fTvSfdfGroup,

    [F_TV_SFFD_GROUP].fTvDblClick            = fTvSffdGroup,
    [F_TV_SFFD_GROUP].fTvKbdEnter            = fTvSffdGroup,

    [F_TV_SFFD_FILE].fTvDblClick             = fTvSffdFile,
    [F_TV_SFFD_FILE].fTvKbdEnter             = fTvSffdFile,

    [F_TV_SFDF_FILE].fTvDblClick             = fTvSfdfFile,
    [F_TV_SFDF_FILE].fTvKbdEnter             = fTvSfdfFile,

    [F_TV_SFFD_ROOT].fTvDblClick             = fTvSffdRoot,
    [F_TV_SFFD_ROOT].fTvKbdEnter             = fTvSffdRoot,

    [F_TV_OPENSELECT_ABSENT].fTvDblClick     = fTvOpenSelectAbsent,
    [F_TV_OPENSELECT_ABSENT].fTvKbdEnter     = fTvOpenSelectAbsent,

    [F_TV_OPENSELECT_BIGGER].fTvDblClick     = fTvOpenSelectBigger,
    [F_TV_OPENSELECT_BIGGER].fTvKbdEnter     = fTvOpenSelectBigger,

    [F_TV_OPENSELECT_NEWER].fTvDblClick      = fTvOpenSelectNewer,
    [F_TV_OPENSELECT_NEWER].fTvKbdEnter      = fTvOpenSelectNewer,

    [F_TV_STARTSFDFSCAN].fTvDblClick         = fTvStartSfdfScan,
    [F_TV_STARTSFDFSCAN].fTvKbdEnter         = fTvStartSfdfScan,

    [F_TV_SFDF_RES].fTvDblClick              = fTvSfdfRes,
    [F_TV_SFDF_RES].fTvKbdEnter              = fTvSfdfRes,

    [F_TV_STARTSFFDSCAN].fTvDblClick         = fTvStartSffdScan,
    [F_TV_STARTSFFDSCAN].fTvKbdEnter         = fTvStartSffdScan,

    [F_TV_LOGS].fTvDblClick                  = fTvLogs,
    [F_TV_LOGS].fTvKbdEnter                  = fTvLogs,
};

static struct TV_GLB g_tv;

static UINT TvMsgBox(PCWSTR szText, PCWSTR szTitle, UINT uType)
{
    return MessageBox(g_tv.hTree, szText, szTitle, uType);
}

static HTREEITEM TvAddChildItemByName(HTREEITEM hi, PCWSTR szName, LPARAM lTviData)
{
    TVINSERTSTRUCT tvis = {0};

    tvis.item.mask    = TVIF_PARAM | TVIF_TEXT;
    tvis.item.pszText = (PWSTR)szName;
    tvis.item.lParam  = lTviData;
    tvis.hParent      = hi;
    return TreeView_InsertItem(g_tv.hTree, &tvis);
}

static void TvAddLogLine(PCWSTR szFmt, ...)
{
    HTREEITEM hLineItem;
    WCHAR wchBuff[TXT_BUFF_SIZE];
    va_list pArgList;
    va_start(pArgList, szFmt);
    wvsprintf(wchBuff, szFmt, pArgList);
    va_end(pArgList);
    hLineItem = TvAddChildItemByName(g_tv.hScanLog, wchBuff, F_TV_EMPTY);
    TreeView_SelectItem(g_tv.hTree, hLineItem);
}

static UINT TvMsgBoxAndLog(PCWSTR szText, PCWSTR szTitle, UINT uType)
{
    TvAddLogLine(g_str.szNoInputLog, szTitle, szText);
    return TvMsgBox(szText, szTitle, uType);
}

static void TvSetState(enum TV_STATE st)
{
    g_tv.st = st;
    PCWSTR szTreeTitle;
    switch(st)
    {
        case TVS_IDLE:
            szTreeTitle = g_str.szTitle;
            break;
        case TVS_SCANNING:
            szTreeTitle = g_str.szScanning;
            break;
        case TVS_CANCEL:
            szTreeTitle = g_str.szCancelled;
            break;
        default:
            return;
    }
    SetWindowText(g_tv.hTree, szTreeTitle);
}

static PWSTR TvGetItemText(HTREEITEM hi, WCHAR wchBuff[TXT_BUFF_SIZE])
{
    TVITEM tvi = {TVIF_TEXT};
    tvi.pszText = wchBuff;
    tvi.cchTextMax = TXT_BUFF_SIZE;
    tvi.hItem = hi;
    TreeView_GetItem(g_tv.hTree, &tvi);
    return wchBuff;
}

static HTREEITEM TvFindChildItemByName(HTREEITEM hi, PCWSTR szName)
{
    WCHAR wchName[TXT_BUFF_SIZE];
    hi = TreeView_GetChild(g_tv.hTree, hi);
    while(hi)
    {
        TvGetItemText(hi, wchName);
        if(!lstrcmpW(wchName, szName))
            break;
        hi = TreeView_GetNextSibling(g_tv.hTree, hi);
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
            TreeView_Expand(g_tv.hTree, hi, TVE_EXPAND);
    }
    return hci;
}

static void TvDeleteChildren(HTREEITEM hItem)
{
    HTREEITEM hChild;
    while((hChild = TreeView_GetChild(g_tv.hTree, hItem)))
        TreeView_DeleteItem(g_tv.hTree, hChild);
}

static void TvAddScanPath(PWSTR szRoot)
{
    FsNormalizePath(szRoot);
    HTREEITEM hti = g_tv.hScanRoot;
    PWSTR p = FsSkipNetworkBacklashes(szRoot);
    PCWSTR szPath = p;
    TvAddLogLine(g_str.szAdded, szRoot);
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
    if(g_tv.st == TVS_IDLE)
        return 0;
    SetWindowText(g_tv.hTree, g_str.szPause);
    if(IDYES == TvMsgBox(g_str.szWaitScanEnd, g_str.szScanInProg, MB_OK | MB_YESNO | MB_ICONQUESTION))
    {
        TvSetState(TVS_CANCEL);
        TvAddLogLine(g_str.szCancelled);
    }
    else
        TvSetState(g_tv.st);
    return 1;
}

static void TvExpandInput()
{
    TreeView_Expand(g_tv.hTree, TreeView_GetRoot(g_tv.hTree), TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hScanRoot, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hScanLog, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hSfdfStartScan, TVE_COLLAPSE);
    TreeView_Expand(g_tv.hTree, g_tv.hSffdStartScan, TVE_COLLAPSE);
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
        hRoot = TreeView_GetParent(g_tv.hTree, hi);
    while(hi != hRoot)
    {
        *phti ++ = hi;
        hi = TreeView_GetParent(g_tv.hTree, hi);
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
    ShellExecute(g_tv.hTree, L"open", L"explorer", wchPath, 0, SW_SHOWNORMAL);  
}

static void TvOpenFile(HTREEITEM hRoot, HTREEITEM hi)
{
    WCHAR wchPath[TXT_BUFF_SIZE];
    TvGetItemFullPath(hRoot, hi, wchPath);
    ShellExecute(g_tv.hTree, L"open", wchPath, 0, 0, SW_SHOWNORMAL);
}

static int TvIsItemDeleteException(HTREEITEM hi)
{
    return
        hi == g_tv.hSffdNewRoots[0]
        || hi == g_tv.hSffdNewRoots[1]
        || hi == g_tv.hSffdBigRoots[0]
        || hi == g_tv.hSffdBigRoots[1]
        || hi == g_tv.hSffdAbsentRoots[0]
        || hi == g_tv.hSffdAbsentRoots[1];
}

static void TvAskAndDeleteFile(HTREEITEM hRoot, HTREEITEM hi)
{
    WCHAR wchPath[TXT_BUFF_SIZE];
    SHFILEOPSTRUCT op;
    HTREEITEM hParent = TreeView_GetParent(g_tv.hTree, hi);
    PWSTR p = TvGetItemFullPath(hRoot, hi, wchPath);
    *(p + (*(p - 1) == L'\\' ? -1 : 1)) = 0;
    //  if(IDYES != TvMsgBox(wchPath, szAskDelete, MB_YESNO | MB_ICONQUESTION))
    //    return;
    op.hwnd = g_tv.hTree;
    op.wFunc = FO_DELETE;
    op.pFrom = wchPath;
    op.pTo = 0;
    op.fFlags = 0;
    op.fAnyOperationsAborted = 0;
    op.hNameMappings = 0;
    op.lpszProgressTitle = g_str.szDeleting;
    if(!SHFileOperation(&op))
    {
        if(!op.fAnyOperationsAborted)
        {
            TreeView_DeleteItem(g_tv.hTree, hi);
            TvAddLogLine(g_str.szDeleted, wchPath);
        }
    }
    else
    {
        if(GetFileAttributes(wchPath) != (DWORD)-1)
            TvMsgBoxAndLog(wchPath, g_str.szErrDel, MB_OK | MB_ICONERROR);
        else
            TreeView_DeleteItem(g_tv.hTree, hi);
    }
    if(!TvIsItemDeleteException(hParent)
            && !TreeView_GetChild(g_tv.hTree, hParent)
            && !op.fAnyOperationsAborted)
        TreeView_DeleteItem(g_tv.hTree, hParent);
}

static void TvFilePropDlg(HTREEITEM hRoot, HTREEITEM hi)
{
    WCHAR wchPath[TXT_BUFF_SIZE];
    SHELLEXECUTEINFO si = {sizeof(SHELLEXECUTEINFO), SEE_MASK_INVOKEIDLIST, g_tv.hTree, L"properties", wchPath};
    TvGetItemFullPath(hRoot, hi, wchPath);
    ShellExecuteEx(&si);
}

static HTREEITEM TvGetSffdFileRoot(HTREEITEM hi)
{
    do{
        hi = TreeView_GetParent(g_tv.hTree, hi);
    }while(hi != g_tv.hSffdNew && hi != g_tv.hSffdBig && hi != g_tv.hSffdAbsent);
    return hi;
}

struct TvFsScanLeavesSfdf
{
    HTREEITEM hi;
    PWSTR szPath;
};

static void TvFsScanLeavesSfdf(struct TvFsScanLeavesSfdf* psc)
{
    HTREEITEM hci = TreeView_GetChild(g_tv.hTree, psc->hi);
    if(!hci)
    {
        TvGetItemFullPath(g_tv.hScanRoot, psc->hi, psc->szPath);
        FsCollectFileNames(psc->szPath);
        return;
    }
    while(hci)
    {
        psc->hi = hci;
        TvFsScanLeavesSfdf(psc);
        hci = TreeView_GetNextSibling(g_tv.hTree, hci);
    }
}

static void TvGetDiffDataAndLog(HTREEITEM h0, PWSTR szPathBuff0, HTREEITEM h1, PWSTR szPathBuff1)
{
    TvGetItemFullPath(g_tv.hScanRoot, h0, szPathBuff0);
    TvGetItemFullPath(g_tv.hScanRoot, h1, szPathBuff1);
    TvAddLogLine(g_str.szCmpDirs, szPathBuff0, szPathBuff1);
}

static void TvAddSffdOutRoots(PCWSTR szRoot0, PCWSTR szRoot1)
{
    g_tv.hSffdNewRoots[0] = TvAddChildItemByName(g_tv.hSffdNew, szRoot0, F_TV_SFFD_GROUP);
    g_tv.hSffdNewRoots[1] = TvAddChildItemByName(g_tv.hSffdNew, szRoot1, F_TV_SFFD_GROUP);
    g_tv.hSffdBigRoots[0] = TvAddChildItemByName(g_tv.hSffdBig, szRoot0, F_TV_SFFD_GROUP);
    g_tv.hSffdBigRoots[1] = TvAddChildItemByName(g_tv.hSffdBig, szRoot1, F_TV_SFFD_GROUP);
    g_tv.hSffdAbsentRoots[0] = TvAddChildItemByName(g_tv.hSffdAbsent, szRoot0, F_TV_SFFD_GROUP);
    g_tv.hSffdAbsentRoots[1] = TvAddChildItemByName(g_tv.hSffdAbsent, szRoot1, F_TV_SFFD_GROUP);
}

static void TvFsScanLeaveSffd()
{
    WCHAR wchPath0[TXT_BUFF_SIZE];
    WCHAR wchPath1[TXT_BUFF_SIZE];
    HTREEITEM hl = 0;
    HTREEITEM hi = g_tv.hScanRoot;
    while(TreeView_GetChild(g_tv.hTree, hi))
    {
        hi = TreeView_GetChild(g_tv.hTree, hi);
        hl = TreeView_GetNextSibling(g_tv.hTree, hi);
        if(hl)
        {
            while(TreeView_GetChild(g_tv.hTree, hl))
                hl = TreeView_GetChild(g_tv.hTree, hl);
            while(TreeView_GetChild(g_tv.hTree, hi))
                hi = TreeView_GetChild(g_tv.hTree, hi);
        }
    }
    TvGetDiffDataAndLog(hi, wchPath0, hl, wchPath1);
    TvAddSffdOutRoots(wchPath0, wchPath1);
    FsSffd(wchPath0, wchPath1);
    TvGetDiffDataAndLog(hi, wchPath0, hl, wchPath1);
    FsSffd(wchPath1, wchPath0);
}

static void TvDblClick()
{
    POINT pt;
    TVHITTESTINFO ht = {{0}};
    TVITEM tvi = {TVIF_PARAM};
    GetCursorPos(&pt);
    ScreenToClient(g_tv.hTree, &pt);
    ht.pt = pt;
    tvi.hItem = TreeView_HitTest(g_tv.hTree, &ht);
    if(tvi.hItem && (ht.flags & TVHT_ONITEM) && !TvInProgressWarning())
    {
        TreeView_GetItem(g_tv.hTree, &tvi);
        g_EvtTbl[tvi.lParam & F_TV_LAST].fTvDblClick(tvi.hItem);
    }
}

static void TvKbdEnter()
{
    TVITEM tvi = {TVIF_PARAM};
    tvi.hItem = TreeView_GetSelection(g_tv.hTree);
    if(tvi.hItem && !TvInProgressWarning())
    {
        TreeView_GetItem(g_tv.hTree, &tvi);
        g_EvtTbl[tvi.lParam & F_TV_LAST].fTvKbdEnter(tvi.hItem);
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
    return CallWindowProc(g_tv.pOldProc, hWnd, msg, wParam, lParam);
}

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
    g_tv.st = TVS_IDLE;
    g_tv.hTree = CreateWindowEx(WS_EX_ACCEPTFILES
            , WC_TREEVIEWW
            , g_str.szTitle
            , WS_OVERLAPPEDWINDOW | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT
            , CW_USEDEFAULT
            , CW_USEDEFAULT
            , CW_USEDEFAULT
            , CW_USEDEFAULT
            , 0
            , 0
            , hExeInst
            , 0);
    TreeView_SetBkColor(g_tv.hTree, 0x00FFFFFF);
    TreeView_SetLineColor(g_tv.hTree, 0x00FF0000);
    SendMessage(g_tv.hTree, WM_SETICON, 0, (LPARAM)hIcon);
    g_tv.pOldProc = (WNDPROC)SetWindowLongPtr(g_tv.hTree , GWLP_WNDPROC, (LONG_PTR)TvProc);

    HTREEITEM hDirOp;
    TVINSERTSTRUCT tvis = {0};
    tvis.item.mask = TVIF_PARAM | TVIF_TEXT;

    //the root item - directory scan operations:
    tvis.item.pszText = (PWSTR)g_str.szDirScanOp;
    tvis.item.lParam = F_TV_EMPTY;
    hDirOp = tvis.hParent = TreeView_InsertItem(g_tv.hTree, &tvis);

    //root for the input directory/file list:
    tvis.item.pszText = (PWSTR)g_str.szRootToScan;
    tvis.item.lParam = F_TV_SCANROOT;
    g_tv.hScanRoot = TreeView_InsertItem(g_tv.hTree, &tvis);

    //start scan for duplicate files:
    tvis.item.pszText = (PWSTR)g_str.szSfdf;
    tvis.item.lParam = F_TV_STARTSFDFSCAN;
    tvis.hParent = g_tv.hSfdfStartScan = TreeView_InsertItem(g_tv.hTree, &tvis);

    //root for found different file lists:
    tvis.item.lParam = F_TV_SFDF_RES;
    tvis.item.pszText = (PWSTR)g_str.szDiff;
    g_tv.hSfdfScanResult = TreeView_InsertItem(g_tv.hTree, &tvis);

    //start scan for different files:
    tvis.hParent = hDirOp;
    tvis.item.pszText = (PWSTR)g_str.szSffd;
    tvis.item.lParam = F_TV_STARTSFFDSCAN;
    tvis.hParent = g_tv.hSffdStartScan = TreeView_InsertItem(g_tv.hTree, &tvis);

    //scan results - files modified later:
    tvis.item.lParam = F_TV_SFFD_ROOT;
    tvis.item.pszText = (PWSTR)g_str.szNewer;
    g_tv.hSffdNew = TreeView_InsertItem(g_tv.hTree, &tvis);

    //scan results - files that are bigger:
    tvis.item.pszText = (PWSTR)g_str.szBigger;
    g_tv.hSffdBig = TreeView_InsertItem(g_tv.hTree, &tvis);

    //scan results - files that are absent in other scan dir:
    tvis.item.pszText = (PWSTR)g_str.szAbsent;
    g_tv.hSffdAbsent = TreeView_InsertItem(g_tv.hTree, &tvis);


    //logs root:
    tvis.hParent = 0;
    tvis.item.pszText = (PWSTR)g_str.szLogs;
    tvis.item.lParam = F_TV_LOGS;
    g_tv.hScanLog = TreeView_InsertItem(g_tv.hTree, &tvis);

    g_tv.hSffdNewRoots[0] = g_tv.hSffdBigRoots[0] = g_tv.hSffdAbsentRoots[0] = 0;
    g_tv.hSffdNewRoots[1] = g_tv.hSffdBigRoots[1] = g_tv.hSffdAbsentRoots[1] = 0;

    TreeView_Expand(g_tv.hTree, hDirOp, TVE_EXPAND);

    ShowWindow(g_tv.hTree, SW_SHOWNORMAL);
    UpdateWindow(g_tv.hTree);

    g_tv.hLastGroup = 0;
}

static void TvFree()
{
}

static void TvAddSfdfGroup(PCWSTR szPath)
{
    PCWSTR szExt = FsGetExtension(szPath);
    if(!szExt)
        szExt = g_str.szNoExt;
    HTREEITEM hExtItem = TvFindChildItemByNameOrAdd(g_tv.hSfdfScanResult, szExt, F_TV_SFDF_GROUP, 0);
    g_tv.hLastGroup = TvAddChildItemByName(hExtItem, szPath, F_TV_SFDF_GROUP);
}

static void TvAddSfdfItem(PCWSTR szPath)
{
    TvAddChildItemByName(g_tv.hLastGroup, szPath, F_TV_SFDF_FILE);
}

static void TvAddSffdNewer(PCWSTR szPath, PWSTR szRelPath)
{
    TvAddPathComplex(g_tv.hSffdNew, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvAddSffdBigger(PCWSTR szPath, PWSTR szRelPath)
{
    TvAddPathComplex(g_tv.hSffdBig, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvAddSffdAbsent(PCWSTR szPath, PWSTR szRelPath)
{
    TvAddPathComplex(g_tv.hSffdAbsent, szPath, szRelPath, F_TV_SFFD_GROUP, F_TV_SFFD_FILE);
}

static void TvSortLastGroup()
{
    TreeView_SortChildren(g_tv.hTree, g_tv.hLastGroup, 0);
}

static void TvProgressTitle()
{
    static DWORD dwCounter = 0;
    static WCHAR wchProg[] =
    {
        L'-', L'\\', L'|', L'/'
    };
    WCHAR wchBuff[64];
    wsprintf(wchBuff, g_str.szProgress, wchProg[++ dwCounter & 3]);
    SetWindowText(g_tv.hTree, wchBuff);
}

static int TvCheckCancel()
{
    return g_tv.st == TVS_CANCEL;
}

static void fTvEmpty(HTREEITEM hi)
{
}

static void fTvScanRoot()
{
    WCHAR wchPath[TXT_BUFF_SIZE];
    switch(MnuPopupMenu(g_tv.hTree, 3, g_str.szAddDirFile, 0, g_str.szCleanAll))
    {
        case 1:
            if(FsSelectDirAndFile(g_tv.hTree, g_str.szAddDirFile, wchPath))
                TvAddScanPath(wchPath);
            break;
        case 3:
            TvDeleteChildren(g_tv.hScanRoot);
            break;
    }
}

static void fTvScanPath(HTREEITEM hi)
{
    switch(MnuPopupMenu(g_tv.hTree, 5, g_str.szOpen, g_str.szOpenContainer, g_str.szFileProp, 0, g_str.szRemoveFromList))
    {
        case 1:
            TvOpenFile(g_tv.hScanRoot, hi);
            break;
        case 2:
            TvOpenPathInExplorer(g_tv.hScanRoot, hi);
            break;
        case 3:
            TvFilePropDlg(g_tv.hScanRoot, hi);
            break;
        case 5:
            TreeView_DeleteItem(g_tv.hTree, hi);
            break;
    }
}

static void fTvSfdfGroup(HTREEITEM hi)
{
    if(1 == MnuPopupMenu(g_tv.hTree, 1, g_str.szRemoveGroup))
        TreeView_DeleteItem(g_tv.hTree, hi);
}

static int TvCopyToOther(HTREEITEM hi)
{
    int iResult = 0;
    HTREEITEM hi0, hi1;
    PWSTR p0, p1;
    WCHAR wchPathBuff0[TXT_BUFF_SIZE], wchPathBuff1[TXT_BUFF_SIZE];
    p0 = wchPathBuff0;
    p1 = wchPathBuff1;
    hi1 = hi0 = TreeView_GetParent(g_tv.hTree, hi);
    while(hi0 != g_tv.hSffdNew && hi0 != g_tv.hSffdBig && hi0 != g_tv.hSffdAbsent)
    {
        hi1 = hi0;
        hi0 = TreeView_GetParent(g_tv.hTree, hi0);
    }
    p1 = TvGetItemFullPath(0, hi1, wchPathBuff1);
    p0 = TvGetItemFullPath(TreeView_GetParent(g_tv.hTree, hi1), hi, wchPathBuff1);
    if(*(p0 - 1) == L'\\')
        *(p0 - 1) = 0;
    *++p0 = 0;//extra 0 needed for FsCopy
    //hi1 - source root, wchPathBuff1 - source full path, p1 - end of source root

    if(TreeView_GetNextSibling(g_tv.hTree, hi1))
        hi0 = TreeView_GetNextSibling(g_tv.hTree, hi1);
    else
        hi0 = TreeView_GetPrevSibling(g_tv.hTree, hi1);  
    p0 = TvGetItemFullPath(0, hi0, wchPathBuff0);
    //hi0 - dest. root, wchPathbuff0 - dest. root path, p0 - end of dest. root

    //append the rest of source path to the root of dest. path
    while(*p1) *p0 ++ = *p1 ++;
    *p0 ++ = 0;
    *p0 = 0;//extra 0 for FsCopy

    //wchPathBuff0 - destination full path, wchPathBuff1 - source full path
    iResult = FsCopy(g_tv.hTree, wchPathBuff0, wchPathBuff1);
    //  if(iResult)
    //    TreeView_DeleteItem(g_tv.hTree, hi);
    return iResult;
}

static int TvCopyToOtherRecursive(HTREEITEM hi)
{
    int iResult = 1;
    HTREEITEM hci = TreeView_GetChild(g_tv.hTree, hi);
    if(!hci)
    {
        return TvCopyToOther(hi);
    }
    while(hci && iResult)
    {
        iResult = TvCopyToOtherRecursive(hci);
        hci = TreeView_GetNextSibling(g_tv.hTree, hci);
    }
    return iResult;
}

static void TvSffdCopyAllGroup(HTREEITEM hi)
{
    hi = TreeView_GetChild(g_tv.hTree, hi);
    if(TreeView_GetChild(g_tv.hTree, hi))
        TvCopyToOtherRecursive(hi);
    hi = TreeView_GetNextSibling(g_tv.hTree, hi);
    if(TreeView_GetChild(g_tv.hTree, hi))
        TvCopyToOtherRecursive(hi);
}

static void fTvSffdGroup(HTREEITEM hi)
{
    int iCount = TreeView_GetChild(g_tv.hTree, hi) ? 3 : 2;
    switch(MnuPopupMenu(g_tv.hTree, iCount, g_str.szOpen, g_str.szOpenContainer, g_str.szCopyAll))
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
    switch(MnuPopupMenu(g_tv.hTree, 6, g_str.szOpen, g_str.szOpenContainer, g_str.szCopyToOther, g_str.szFileProp, 0, g_str.szDeleteFile))
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
    switch(MnuPopupMenu(g_tv.hTree, 5, g_str.szOpen, g_str.szOpenContainer, g_str.szFileProp, 0, g_str.szDeleteFile))
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
    switch(MnuPopupMenu(g_tv.hTree, 3, g_str.szCopyAll, 0, g_str.szEmptyGroup))
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
    hci = TreeView_GetChild(g_tv.hTree, psc->hi);
    if(!hci)
    {
        TvGetItemFullPath(g_tv.hScanRoot, psc->hi, psc->szPath);
        if(GetFileAttributes(psc->szPath) & FILE_ATTRIBUTE_DIRECTORY)
            psc->dwCount ++;
        else
            psc->dwCount = (DWORD)-1;
    }
    while(hci)
    {
        psc->hi = hci;
        TvScanForLeaveCountCheckDir(psc);
        hci = TreeView_GetNextSibling(g_tv.hTree, hci);
    }
}

static int TvCheck2DirInputsAndWarn()
{
    struct TvScanForLeaveCountCheckDir sc;
    WCHAR wchPathBuff[TXT_BUFF_SIZE];
    sc.hi = g_tv.hScanRoot;
    sc.szPath = wchPathBuff;
    sc.dwCount = 0;
    TvScanForLeaveCountCheckDir(&sc);
    if(sc.dwCount != 2)
    {
        TvMsgBoxAndLog(g_str.szOnly2Dirs, g_str.szInvalidInput, MB_OK | MB_ICONWARNING);
        return 1;
    }
    return 0;
}

static int TvCheckIfNoInputAndWarn()
{
    if(!TreeView_GetChild(g_tv.hTree, g_tv.hScanRoot))
    {
        TvAddLogLine(g_str.szNoInputLog, g_str.szNoInput, g_str.szDragDrop);
        TvMsgBox(g_str.szDragDrop, g_str.szNoInput, MB_OK | MB_ICONWARNING);
        return 1;
    }
    return 0;
}

static void TvCleanup()
{
    TvAddLogLine(g_str.szCleanUpTree);
    TvDeleteChildren(g_tv.hScanLog);
    TvDeleteChildren(g_tv.hSfdfScanResult);
    TvDeleteChildren(g_tv.hSffdNew);
    TvDeleteChildren(g_tv.hSffdBig);
    TvDeleteChildren(g_tv.hSffdAbsent);
}

static void TvExpandSffdRes()
{
    TreeView_Expand(g_tv.hTree, g_tv.hSffdStartScan, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hSffdNew, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hSffdBig, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hSffdAbsent, TVE_EXPAND);

    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdNewRoots[0]);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdNewRoots[1]);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdBigRoots[0]);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdBigRoots[1]);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdAbsentRoots[0]);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdAbsentRoots[1]);
}

static void TvCleanupSffdOutRoots()
{
    if(g_tv.hSffdNewRoots[0])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdNewRoots[0]);
    if(g_tv.hSffdBigRoots[0])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdBigRoots[0]);
    if(g_tv.hSffdAbsentRoots[0])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdAbsentRoots[0]);

    if(g_tv.hSffdNewRoots[1])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdNewRoots[1]);
    if(g_tv.hSffdBigRoots[1])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdBigRoots[1]);
    if(g_tv.hSffdAbsentRoots[1])
        TreeView_DeleteItem(g_tv.hTree, g_tv.hSffdAbsentRoots[1]);

    g_tv.hSffdNewRoots[0] = g_tv.hSffdBigRoots[0] = g_tv.hSffdAbsentRoots[0] = 0;
    g_tv.hSffdNewRoots[1] = g_tv.hSffdBigRoots[1] = g_tv.hSffdAbsentRoots[1] = 0;
}

void fTvStartSfdfScan()
{
    struct TvFsScanLeavesSfdf sc;
    WCHAR wchPathBuff[TXT_BUFF_SIZE];
    FsDelTempFiles();
    TvDeleteChildren(g_tv.hSfdfScanResult);
    TvDeleteChildren(g_tv.hScanLog);
    if(TvCheckIfNoInputAndWarn())
        return;

    TvSetState(TVS_SCANNING);
    FsInitScan();
    sc.hi = g_tv.hScanRoot;
    sc.szPath = wchPathBuff;
    TvFsScanLeavesSfdf(&sc);
    FsSfdf();
    TreeView_Expand(g_tv.hTree, g_tv.hSfdfStartScan, TVE_EXPAND);
    TreeView_Expand(g_tv.hTree, g_tv.hSfdfScanResult, TVE_EXPAND);
    TreeView_SelectItem(g_tv.hTree, g_tv.hSfdfStartScan);
    if(TvCheckCancel())
        TvCleanup();

    TvSetState(TVS_IDLE);
}

void fTvSfdfRes(HTREEITEM hi)
{
    if(1 == MnuPopupMenu(g_tv.hTree, 1, g_str.szCleanAll))
        TvDeleteChildren(hi);
}

void fTvStartSffdScan()
{
    TvDeleteChildren(g_tv.hSffdNew);
    TvDeleteChildren(g_tv.hSffdBig);
    TvDeleteChildren(g_tv.hSffdAbsent);
    TvDeleteChildren(g_tv.hScanLog);
    TvCleanupSffdOutRoots();
    if(TvCheckIfNoInputAndWarn())
        return;
    if(TvCheck2DirInputsAndWarn())
        return;
    TvSetState(TVS_SCANNING);
    FsInitScan();
    TvFsScanLeaveSffd();
    TvExpandSffdRes();
    TreeView_SelectItem(g_tv.hTree, g_tv.hSffdStartScan);
    if(TvCheckCancel())
        TvCleanup();
    TvSetState(TVS_IDLE);
}

void fTvLogs()
{
    UINT uSel = MnuPopupMenu(g_tv.hTree, 1, g_str.szCleanupLog);
    if(uSel == 1)
        TvDeleteChildren(g_tv.hScanLog);
}

static void RtMessageLoop()
{
    MSG msg;
    while(GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
    TvInit();
    RtMessageLoop();
    TvFree();
    return 0;
}

