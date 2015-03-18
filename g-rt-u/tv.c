#include "rt.h"

enum
{
  TV_COL0
  , TV_COL1
  , TV_COL2
  , TV_DBLCLICK_PROC
};

static void fTvEmpty(GtkTreeIter* it);
static void fTvScanDupFilesInput(GtkTreeIter* it);
static void fTvScanDupFilesAddDir(GtkTreeIter* it);
static void fTvScanDupFilesAddFile(GtkTreeIter* it);
static void fTvScanDupFilesRun(GtkTreeIter* it);

static void fTvLog(gchar* tvPath);
static void fTvDblClick(GtkTreeView* tv, GtkTreePath* tp, GtkTreeViewColumn* tvc, gpointer ctx);
static void fTvCol0DataFunc(GtkTreeViewColumn* col, GtkCellRenderer* cr, GtkTreeModel* tm, GtkTreeIter* it, gpointer data);

static void TvAddRow(const gchar* tvPath, const char* s0, const char* s1, const char* s2, gpointer dbl_click)
{
  GtkTreeIter il, it;
  if(tvPath)
    gtk_tree_model_get_iter_from_string((GtkTreeModel*)g_tv.ts, &il, tvPath);
  gtk_tree_store_append(g_tv.ts, &it, tvPath ? &il : NULL);
  gtk_tree_store_set(g_tv.ts, &it, TV_COL0, s0, TV_COL1, s1, TV_COL2, s2, TV_DBLCLICK_PROC, dbl_click, -1);
}

static void TvAddLogLine(const char* s0, const char* s1)
{
  TvAddRow("1", s0, s1, 0, fTvEmpty);
}

static void TvDeleteChildren(const char* tvPath)
{
  GtkTreeIter il, it;
  gtk_tree_model_get_iter_from_string((GtkTreeModel*)g_tv.ts, &il, tvPath);
  if(!gtk_tree_model_iter_children((GtkTreeModel*)g_tv.ts, &it, &il))
    return;
  while(gtk_tree_store_remove(g_tv.ts, &it));
}

static void TvExpand(const char* tvPath, gboolean expand)
{
  GtkTreePath* p = 0;
  p = gtk_tree_path_new_from_string(tvPath);
  gtk_tree_view_expand_row(g_tv.tv, p, expand);
  gtk_tree_path_free(p);
}

gboolean TvForEachValue(const gchar* tvPath, gpointer data)
{
  gboolean bcont = TRUE;
  GtkTreeIter itDirRoot, it;
  gtk_tree_model_get_iter_from_string((GtkTreeModel*)g_tv.ts, &itDirRoot, tvPath);
  if(gtk_tree_model_iter_children((GtkTreeModel*)g_tv.ts, &it, &itDirRoot))
  {
    do
    {
      gchar* expath = 0;
      gtk_tree_model_get((GtkTreeModel*)g_tv.ts, &it, 0, &expath, -1);
      bcont = g_tv.fForEachProc(expath, data);
      g_free(expath);
    }while(bcont && gtk_tree_model_iter_next((GtkTreeModel*)g_tv.ts, &it));
  }
  return bcont;
}

static void TvRemovePathIntersection(gchar* path, const gchar* tvPath)
{
  GtkTreeIter itDirRoot, it;
  gtk_tree_model_get_iter_from_string((GtkTreeModel*)g_tv.ts, &itDirRoot, tvPath);
locFirstChild:
  if(gtk_tree_model_iter_children((GtkTreeModel*)g_tv.ts, &it, &itDirRoot))
  {
    do
    {
      gchar* expath = 0;
      gtk_tree_model_get((GtkTreeModel*)g_tv.ts, &it, 0, &expath, -1);
      if(g_fs.IsPathIntersect(path, expath))
      {
        gtk_tree_store_remove(g_tv.ts, &it);
        TvAddLogLine(expath, "Removed intersection");
        g_free(expath);
        goto locFirstChild;
      }
      g_free(expath);
    }while(gtk_tree_model_iter_next((GtkTreeModel*)g_tv.ts, &it));
  }
}

static void TvDlgAddPathToTree(const gchar* tvPath
                              , GtkFileChooserAction act
                              , const gchar* log
                              , const char* tvpath1
                              , const char* tvpath2
                              , void (*proc)(GtkTreeIter*))
{
  GtkFileChooserDialog* dlg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new("Select File to Scan", g_rt.mw, act, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  if(gtk_dialog_run((GtkDialog*)dlg) == GTK_RESPONSE_ACCEPT)
  {
    gchar* path = gtk_file_chooser_get_filename((GtkFileChooser*)dlg);
    if(tvpath1)
      TvRemovePathIntersection(path, tvpath1);
    if(tvpath2)
      TvRemovePathIntersection(path, tvpath2);
    TvAddRow(tvPath, path, 0, 0, proc);
    TvAddLogLine(path, log);
    g_free(path);
  }
  gtk_widget_destroy((GtkWidget*)dlg);
  TvExpand(tvPath, TRUE);
}

static GtkTreeModel* TvCreateModel()
{
  g_tv.ts = gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
  TvAddRow(0, "Directory Scan Operations", 0, 0, fTvEmpty);
//==============================================================================
  TvAddRow("0", "Scan for Duplicate Files", 0, 0, fTvEmpty);
  TvAddRow("0:0", "Inputs", 0, 0, fTvScanDupFilesInput);
  TvAddRow("0:0:0", "Add Directory...", 0, 0, fTvScanDupFilesAddDir);
  TvAddRow("0:0:0", "Add File...", 0, 0, fTvScanDupFilesAddFile);
  TvAddRow("0:0", ">>Run...", 0, 0, fTvScanDupFilesRun);
  TvAddRow("0:0", "Outputs", 0, 0, fTvEmpty);

//==============================================================================
  TvAddRow("0", "Scan for Different Files", 0, 0, fTvEmpty);
  TvAddRow("0:1", "Inputs", 0, 0, fTvEmpty);
  TvAddRow("0:1:0", "Add Directory...", 0, 0, fTvEmpty);
  TvAddRow("0:1", ">>Run...", 0, 0, fTvEmpty);
  TvAddRow("0:1", "Outputs", 0, 0, fTvEmpty);
  TvAddRow("0:1:2", "Newer Then in Other Directory", 0, 0, fTvEmpty);
  TvAddRow("0:1:2", "Bigger Then in Other Directory", 0, 0, fTvEmpty);
  TvAddRow("0:1:2", "Absent in Other Directory", 0, 0, fTvEmpty);

//==============================================================================
  TvAddRow(0, "Logs", 0, 0, fTvLog);
  return (GtkTreeModel*)g_tv.ts;
}

static GtkScrolledWindow* TvCreateTree()
{
  static const char* tx = "text";
  GtkScrolledWindow* sw = (GtkScrolledWindow*)gtk_scrolled_window_new(0, 0);
  g_tv.tv = (GtkTreeView*)gtk_tree_view_new_with_model(TvCreateModel());//gtk_tree_view_new();
  GtkCellRenderer* cr = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(g_tv.tv, -1, "Col0", cr, tx, 0, NULL);
  gtk_tree_view_insert_column_with_attributes(g_tv.tv, -1, "Col1", cr, tx, 1, NULL);
  gtk_tree_view_insert_column_with_attributes(g_tv.tv, -1, "Col2", cr, tx, 2, NULL);
//  gtk_tree_view_insert_column_with_attributes(g_tv.tv, -1, "Col3", cr, tx, 3, NULL);
/*  gtk_tree_view_column_set_attributes(gtk_tree_view_get_column(rtApp.tv, 0)
                                      , cr
                                      , "markup"
                                      , 0
                                      , NULL, NULL);*/
  gtk_tree_view_column_set_cell_data_func(gtk_tree_view_get_column(g_tv.tv, 0), cr, fTvCol0DataFunc, NULL, NULL);

  gtk_scrolled_window_add_with_viewport(sw, (GtkWidget*)g_tv.tv);
  gtk_tree_view_set_model(g_tv.tv, (GtkTreeModel*)g_tv.ts);
  g_object_unref(g_tv.ts);
  
  gtk_tree_view_set_grid_lines(g_tv.tv, GTK_TREE_VIEW_GRID_LINES_BOTH);
  gtk_tree_view_set_enable_tree_lines(g_tv.tv, TRUE);
//  gtk_tree_view_set_headers_clickable(g_tv.tv, TRUE);
  gtk_tree_view_column_set_resizable(gtk_tree_view_get_column(g_tv.tv, 0), TRUE);
  gtk_tree_view_column_set_resizable(gtk_tree_view_get_column(g_tv.tv, 1), TRUE);
  gtk_tree_view_column_set_resizable(gtk_tree_view_get_column(g_tv.tv, 2), TRUE);
  g_signal_connect((GtkObject*)g_tv.tv, "row-activated", GTK_SIGNAL_FUNC(fTvDblClick), NULL);
  gtk_tree_view_expand_all(g_tv.tv);
  return sw;
}

static void TvAskDeleteRow(GtkTreeIter* it)
{
  static const gchar* msg = "Are you sure you want to the selected item from the list?";
  gchar* pch;
  if(GTK_RESPONSE_YES != g_rt.MsgBoxYesNo(msg))
    return;
  gtk_tree_model_get((GtkTreeModel*)g_tv.ts, it, 0, &pch, -1);
  gtk_tree_store_remove(g_tv.ts, it);
  TvAddLogLine(pch, "Removed");
  g_free(pch);
}




/*
GtkWindow *         gtk_widget_get_tooltip_window       (GtkWidget *widget);
void                gtk_tree_view_set_tooltip_cell      (GtkTreeView *tree_view,
                                                         GtkTooltip *tooltip,
                                                         GtkTreePath *path,
                                                         GtkTreeViewColumn *column,
                                                         GtkCellRenderer *cell);
*/

void fTvEmpty(GtkTreeIter* it)
{
  g_print(">>fTvEmpty\n");
}

void fTvScanDupFilesInput(GtkTreeIter* it)
{
  static const char* msg = "Are you sure you want to clean the list of input files and directories?";
  if(GTK_RESPONSE_YES == g_rt.MsgBoxYesNo(msg))
  {
    TvDeleteChildren("0:0:0:0");
    TvDeleteChildren("0:0:0:1");
    TvAddLogLine("*****", "Deleted all input paths");
  }
}

void fTvScanDupFilesAddDir(GtkTreeIter* it)
{
  TvDlgAddPathToTree("0:0:0:0", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Added directory", "0:0:0:0", "0:0:0:1", TvAskDeleteRow);
}

void fTvScanDupFilesAddFile(GtkTreeIter* it)
{
  TvDlgAddPathToTree("0:0:0:1", GTK_FILE_CHOOSER_ACTION_OPEN, "Added file", "0:0:0:0", "0:0:0:1", TvAskDeleteRow);
}

void fTvScanDupFilesRun(GtkTreeIter* it)
{
  g_fs.ScanForDupFiles();
}

void fTvCol0DataFunc(GtkTreeViewColumn* col, GtkCellRenderer* cr, GtkTreeModel* tm, GtkTreeIter* it, gpointer data)
{
//  g_object_set(cr, "foreground", "red", NULL);
}

void fTvDblClick(GtkTreeView* tv, GtkTreePath* tp, GtkTreeViewColumn* tvc, gpointer ctx)
{
//  void (*proc)(GtkTreePath*) = 0;
  GtkTreeIter it;
  void (*proc)(GtkTreeIter* it);
  if(gtk_tree_model_get_iter((GtkTreeModel*)g_tv.ts, &it, tp))
  {
    gtk_tree_model_get((GtkTreeModel*)g_tv.ts, &it, TV_DBLCLICK_PROC, &proc, -1);
    if(proc)
      proc(&it);
  }
}

void fTvLog(gchar* tvPath)
{
  if(g_strcmp0(tvPath, "1"))
    return;
  static const char* msg = "Are you sure you want to clean the collected log information?";
  if(GTK_RESPONSE_YES == g_rt.MsgBoxYesNo(msg))
    TvDeleteChildren("1");
}

struct TV g_tv =
{
  .CreateTree               = TvCreateTree
  , .AddLogLine             = TvAddLogLine
  , .DeleteChildren         = TvDeleteChildren
  , .Expand                 = TvExpand
  , .RemovePathIntersection = TvRemovePathIntersection
  , .ForEachValue           = TvForEachValue
};

