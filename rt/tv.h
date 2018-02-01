#ifndef __TV_H__
#define __TV_H__

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
  PVOID pEvtTbl;
};

struct TV_API
{
  struct TV_GLB* pGlb;
  void (*TvInit)();
  void (*TvFree)();
  void (*TvAddLogLine)(PCWSTR szFmt, ...);
};

extern struct TV_API g_tv;

#endif //__TV_H__
