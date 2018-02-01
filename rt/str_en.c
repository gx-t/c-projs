#include "rt.h"
#if RT_LANG == EN

static struct STR_TBL g_tbl =
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

struct STR_API g_str =
{
  &g_tbl
};

#endif //RT_LANG == EN
