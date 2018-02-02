#pragma once

typedef enum
{
  MT_TEXT_MENU,
    MT_REG_MENU
} MENU_TYPE, *PMENU_TYPE;

typedef struct TEXT_MENU
{
  MENU_TYPE     mt;
  UINT          uCount;
  PPCWSTR       pszItemArr;
  PUINT         puFlags;
  int (**ProcTable)();
  int (*SelProc)();
  DWORD         dwItemTextBuffLen;
  PWSTR         szItemTextBuff;
} TEXT_MENU, *PTEXT_MENU;

typedef struct REG_MENU
{
  TEXT_MENU mb;
  HKEY hKeyRoot;
  UINT (*InitCheckProc)(struct REG_MENU*);
} REG_MENU, *PREG_MENU;

void AppendTextMenu(HMENU, PCWSTR, PUINT);
UINT PopupTextMenu(HMENU);
void GetMenuItemText(HMENU, UINT, PWSTR, UINT);
void PopupMenu(PMENU_TYPE);
