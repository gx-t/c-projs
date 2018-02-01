#pragma once

#define TVM_SETLINECOLOR            (TV_FIRST + 40)
#define TreeView_SetLineColor(hwnd, clr) \
    (COLORREF)SNDMSG((hwnd), TVM_SETLINECOLOR, 0, (LPARAM)(clr))

#define TVM_GETLINECOLOR            (TV_FIRST + 41)
#define TreeView_GetLineColor(hwnd) \
    (COLORREF)SNDMSG((hwnd), TVM_GETLINECOLOR, 0, 0)


//TreeCtrl functions
typedef BOOL (*TREE_ITEM_ENUM_PROC)(HWND, LPTVITEM, PVOID);
BOOL TreeGetCheck(HWND hTree, HTREEITEM hItem);
BOOL TreeSetCheck(HWND hTree, HTREEITEM hItem, BOOL bCheck);
void EnumTreeItems(HWND hTree, TREE_ITEM_ENUM_PROC Proc, PVOID pContext);

