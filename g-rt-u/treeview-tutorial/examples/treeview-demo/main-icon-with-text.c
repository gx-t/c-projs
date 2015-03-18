#include <gtk/gtk.h>

enum
{
  COL_ICON = 0,
  COL_TEXT,
  COL_TEXT2,
  NUM_COLS
};

/**************************************************************
 *
 *   create_liststore
 *
 **************************************************************/

GtkListStore *
create_liststore(void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  GdkPixbuf     *icon;
  GError        *error = NULL;

  store = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);

  icon = gdk_pixbuf_new_from_file("icon.png", &error);
  if (error)
  {
    g_warning ("Could not load icon: %s\n", error->message);
    g_error_free(error);
    error = NULL;
  }

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
                     COL_ICON, icon,
                     COL_TEXT, "Smile",
	                   COL_TEXT2, "and another column",
                     -1);

  return store;
}


/**************************************************************
 *
 *   create_treeview
 *
 **************************************************************/

GtkWidget *
create_treeview(void)
{
	GtkTreeModel      *model;
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	GtkWidget         *view;

	model = GTK_TREE_MODEL(create_liststore());

	view = gtk_tree_view_new_with_model(model);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Column #1");

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "pixbuf", COL_ICON,
	                                    NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "text", COL_TEXT,
	                                    NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

  /* 2nd column */

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Column #2");

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "text", COL_TEXT2,
	                                    NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	return view;
}


/**************************************************************
 *
 *   main
 *
 **************************************************************/

gint
main (gint argc, gchar **argv)
{
	GtkWidget *mainwindow, *vbox;

	gtk_init(&argc, &argv);

	mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* quit when window close button is clicked */
  g_signal_connect(mainwindow, "delete_event", gtk_main_quit, NULL); /* dirty */

	gtk_container_add(GTK_CONTAINER(mainwindow), create_treeview());

	gtk_widget_show_all(mainwindow);

	gtk_main();

	return 0;
}
