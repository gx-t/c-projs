#pragma once

//TreeCtrl functions
typedef BOOL (*TREE_ITEM_ENUM_PROC)(HWND, LPTVITEM, PVOID);
BOOL TreeGetCheck(HWND hTree, HTREEITEM hItem);
BOOL TreeSetCheck(HWND hTree, HTREEITEM hItem, BOOL bCheck);
void EnumTreeItems(HWND hTree, TREE_ITEM_ENUM_PROC Proc, PVOID pContext);

