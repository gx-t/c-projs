#include "common.h"

static LPCWSTR szTextTitle = L"Completed clipboard text processing";
static LPCWSTR szRemove13Msg =
L"%d characters from ASCII buffer and\r\n"\
L"%d characters form unicode buffer have been removed";
static LPCWSTR szWinUniMsg =
L"%d characters total proceeded\r\n"\
L"%d characters from unicode armenian subrange\r\n"\
L"%d characters from unicode russian subrange\r\n"\
L"have been converted from unicode to windows encoding";

static int UniToWinProc(SIZE_T size, PVOID pData, PVOID pContext)
{
  WCHAR szBuff[0x400];
  static UCHAR uchU2WArmTbl[] =
  {
    0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE, 0xC0, 0xC2, 0xC4, 0xC6, 0xC8,
      0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
      0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 0xF0, 0xF2, 0xF4,
      0xF6, 0xF8, 0xFA, 0xFC, '?', '?',  0xA7, 0xA6, 0xAF, 0xB0, 0xAA,
      0xB1, 0xAD, '?', 0xB3, 0xB5, 0xB7, 0xB9, 0xBB, 0xBD, 0xBF, 0xC1,
      0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF, 0xD1, 0xD3, 0xD5, 0xD7,
      0xD9, 0xDB, 0xDD, 0xDF, 0xE1, 0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xED,
      0xEF, 0xF1, 0xF3, 0xF5, 0xF7, 0xF9, 0xFB, 0xFD, 0xA8, '?', 0xA3, 0xA3
  };
  HANDLE hDest;
  PUCHAR pDest;
  LPWSTR pSrc = (LPWSTR)pData;
  int iCountAm = 0;
  int iCountRu = 0;
  int iCountTotal = 0;
  if(!size || size & 1)
    return 0;
  size >>= 1;
  hDest = GlobalAlloc(GMEM_MOVEABLE, size);
  if(!hDest)
    return 0;
  pDest = (PUCHAR)GlobalLock(hDest);
  if(!pDest)
    return 0;
  while(size --)
  {
    if(*pSrc <= 0x589 && *pSrc >= 0x531)//armenian
    {
      *pDest = uchU2WArmTbl[*pSrc - 0x531];
      iCountAm ++;
    }
    else if(*pSrc <= 0x44F && *pSrc >= 0x410)//russian
    {
      *pDest = (UCHAR)(*pSrc - 0x350);
      iCountRu ++;
    }
    else
    {
      *pDest = (UCHAR)(0xFF & *pSrc);
    }
    pSrc ++;
    pDest ++;
    iCountTotal ++;
  }
  GlobalUnlock(hDest);
  EmptyClipboard();
  ClipSetData(CF_TEXT, hDest);
  wsprintf(szBuff, szWinUniMsg, iCountTotal, iCountAm, iCountRu);
  MessageBox(g_hMainWnd, szBuff, szTextTitle, MB_OK | MB_ICONINFORMATION);
  return 0;
}

static int UniToWin()
{
  ClipGetData(UniToWinProc, CF_UNICODETEXT, 0, FALSE);
  return 0;
}

static int ArmToUni()
{
  return 0;
}

static int RusToUniProc(SIZE_T size, PVOID pData, PVOID pContext)
{
  HANDLE hDest;
  LPWSTR pDest;
  PUCHAR pSrc = (PUCHAR)pData;
  if(!size)
    return 0;
  hDest = GlobalAlloc(GMEM_MOVEABLE, 2 * size);
  if(!hDest)
    return 0;
  pDest = (LPWSTR)GlobalLock(hDest);
  if(!pDest)
    return 0;
  while(size --)
  {
    *pDest = (WCHAR)*pSrc;
    if(*pSrc >= 0xC0)
      *pDest += 0x0350;
    pSrc ++;
    pDest ++;
  }
  GlobalUnlock(hDest);
  ClipSetData(CF_UNICODETEXT, hDest);
  return 0;
}

static int RusToUni()
{
  ClipGetData(RusToUniProc, CF_TEXT, 0, FALSE);
  return 0;
}

static void RemoveNLUniProc(SIZE_T size, PVOID pData, PVOID pContext)
{
  LPWSTR pSrc = (LPWSTR)pData;
  LPWSTR pDest = pSrc;
  for(; *pSrc; pSrc ++, pDest ++)
  {
    if(*pSrc == '\r' && *(pSrc + 1) == '\n')
    {
      if(*(pSrc + 2) == ' ' || *(pSrc + 2) == '\t')
      {
        *pDest = *pSrc;
        pSrc ++;
        pDest ++;
        *pDest = *pSrc;
      }
      else
      {
        pSrc ++;
        *pDest = ' ';
      }
      continue;
    }
    *pDest = *pSrc;
  }
  *pDest = 0;
}
/*
static void RemoveNLAsciiProc(SIZE_T size, PVOID pData, PVOID pContext)
{
  LPSTR pSrc = (LPSTR)pData;
  LPSTR pDest = pSrc;
  for(; *pSrc; pSrc ++, pDest ++)
  {
    if(*pSrc == '\r' && *(pSrc + 1) == '\n')
    {
      if(*(pSrc + 2) == ' ' || *(pSrc + 2) == '\t')
      {
        *pDest = *pSrc;
        pSrc ++;
        pDest ++;
        *pDest = *pSrc;
      }
      else
      {
        pSrc ++;
        *pDest = ' ';
      }
      continue;
    }
    *pDest = *pSrc;
  }
  *pDest = 0;
}
*/
static int RemoveNL()
{
  ClipGetData(RemoveNLUniProc, CF_UNICODETEXT, 0, TRUE);
//  ClipGetData(RemoveNLAsciiProc, CF_TEXT, 0, TRUE);
  return 0;
}

static int Remove13()
{
  HANDLE hClip;
  WCHAR wchText[0x400];
  LPWSTR pUniSrc, pUniDest;
  LPSTR pAscSrc, pAscDest;
  int iUniCount = 0;
  int iAscCount = 0;
  if(!OpenClipboard(g_hMainWnd))
    return 0;
  hClip = GetClipboardData(CF_UNICODETEXT);
  if(hClip)
  {
    pUniDest = pUniSrc = (LPWSTR)GlobalLock(hClip);
    for(; *pUniSrc; pUniSrc ++, pUniDest ++)
    {
      if(*pUniSrc == L'\r')
        pUniSrc ++;
      *pUniDest = *pUniSrc;
    }
    *pUniDest = 0;
    GlobalUnlock(hClip);
    SetClipboardData(CF_UNICODETEXT, hClip);
    iUniCount = ((int)pUniSrc - (int)pUniDest) >> 1;
  }

  hClip = GetClipboardData(CF_TEXT);
  if(hClip)
  {
    pAscDest = pAscSrc = (LPSTR)GlobalLock(hClip);
    for(; *pAscSrc; pAscSrc ++, pAscDest ++)
    {
      if(*pAscSrc == '\r')
        pAscSrc ++;
      *pAscDest = *pAscSrc;
    }
    *pAscDest = 0;
    GlobalUnlock(hClip);
    SetClipboardData(CF_TEXT, hClip);
    iAscCount = (int)pAscSrc - (int)pAscDest;
  }
  CloseClipboard();
  wsprintf(wchText, szRemove13Msg, iUniCount, iAscCount);
  MessageBox(g_hMainWnd, wchText, szTextTitle, MB_OK | MB_ICONINFORMATION);
  return 0;
}

int Text()
{
  static LPCWSTR szTextArr[] =
  {
    0,
      L"&1. Unicode to ASCII",
      L"&2. Armenian to unicode",
      L"&3. Russian to unicode",
      0,
      L"&4. Remove extra NLs",
      0,
      L"&5. Remove 13",
      0,
      L"&6. Format..."
  };

  static int (*DispatchTable[])() =
  {
    EmptyProc,
      UniToWin,
      ArmToUni,
      RusToUni,
      0,
      RemoveNL,
      0,
      Remove13,
      0,
      TxtFormat
  };

  static UINT uFlags[] =
  {
    0,
      MF_STRING,
      MF_STRING,
      MF_STRING,
      0,
      MF_STRING,
      0,
      MF_STRING,
      0,
      MF_STRING
  };

  static TEXT_MENU mt =
  {
    MT_TEXT_MENU
      , sizeof(szTextArr) / sizeof(szTextArr[0])
      , szTextArr
      , uFlags
      , DispatchTable
  };

  BOOL bUniText = IsClipboardFormatAvailable(CF_UNICODETEXT);
  BOOL bText = IsClipboardFormatAvailable(CF_TEXT);
  uFlags[1] |= (bUniText ? 0 : MF_GRAYED);
  uFlags[3] = uFlags[2] |= (bText ? 0 : MF_GRAYED);
  uFlags[5] |= (bUniText ? 0 : MF_GRAYED);
  uFlags[6] |= (bText ? 0 : MF_GRAYED);

  PopupMenu((PMENU_TYPE)&mt);
  mt.SelProc();
  return 0;
}

