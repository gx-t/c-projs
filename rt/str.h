#ifndef __STR_H__
#define __STR_H__

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

struct STR_API
{
  struct STR_TBL* pTbl;
};

extern struct STR_API g_str;

#endif //__STR_H__
