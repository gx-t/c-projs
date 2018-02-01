 #include "common.h"

BOOL TreeGetCheck(
                  HWND hTree,
                  HTREEITEM hItem
                  )
{
	TVITEM item;
	item.mask       = TVIF_HANDLE | TVIF_STATE;
	item.hItem      = hItem;
	item.stateMask  = TVIS_STATEIMAGEMASK;
	SendMessage(hTree, TVM_GETITEM, 0, (LPARAM)&item);
	return ((BOOL)(item.state >> 12) -1);
}

BOOL TreeSetCheck(
                  HWND hTree,
                  HTREEITEM  hItem,
                  BOOL bCheck
                  )
{
	TVITEM item;
	item.mask       = TVIF_HANDLE | TVIF_STATE;
	item.hItem      = hItem;
	item.stateMask  = TVIS_STATEIMAGEMASK;
 	item.state      = INDEXTOSTATEIMAGEMASK((bCheck ? 2 : 1));
	return (BOOL)SendMessage(hTree, TVM_SETITEM, 0, (LPARAM)&item);
}

void EnumTreeItems(
                   HWND hTree,
                   TREE_ITEM_ENUM_PROC Proc,
                   PVOID pContext
                   )
{
  WCHAR szBuff[MAX_PATH];
  TVITEM TreeItem = {TVIF_TEXT};
  TreeItem.hItem = TreeView_GetRoot(hTree);
  TreeItem.cchTextMax = MAX_PATH;
  TreeItem.pszText = szBuff;
  while(TreeItem.hItem)
  {
    TreeView_GetItem(hTree, &TreeItem);
    if(!Proc(hTree, &TreeItem, pContext))
      return;
    TreeItem.hItem = TreeView_GetNextSibling(hTree, TreeItem.hItem);
  }
}

