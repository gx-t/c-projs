
#include <gtk/gtk.h>

enum
{
  COL_TEXT = 0,
  NUM_COLS
};


static GtkWidget *treeview;
static GtkWidget *delbutton;


/**************************************************************************
 *
 *  onAddButtonPress
 *
 *  Button has been clicked, or <enter> has been hit in entry
 *
 **************************************************************************/

static void
onAddButtonPress (GtkWidget *entry, gpointer data)
{
	const gchar *txt;

	g_assert(GTK_IS_ENTRY(entry));

	txt = gtk_entry_get_text(GTK_ENTRY(entry));

	/* ignore if entry is empty */
	if (txt && *txt)
	{
		GtkTreeModel *model;
		GtkTreeIter   newrow;

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

		gtk_list_store_append(GTK_LIST_STORE(model), &newrow);

		gtk_list_store_set(GTK_LIST_STORE(model), &newrow, COL_TEXT, txt, -1);

		gtk_entry_set_text(GTK_ENTRY(entry), ""); /* clear entry */
	}
}


/**************************************************************************
 *
 *  onDelButtonPress
 *
 **************************************************************************/

static void
onDelButtonPress (GtkWidget *button, gpointer data)
{
	GtkTreeSelection *sel;
	GtkTreeModel     *model;
	GtkTreeIter       selected_row;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	g_assert(gtk_tree_selection_get_mode(sel) == GTK_SELECTION_SINGLE);

	if (gtk_tree_selection_get_selected(sel, &model, &selected_row))
	{
		gtk_list_store_remove(GTK_LIST_STORE(model), &selected_row);
	}
	else
	{
		/* If no row is selected, the button should
		 *  not be clickable in the first place */
		g_assert_not_reached();
	}
}


/**************************************************************************
 *
 *  onSelectionChanged
 *
 *  Sets 'delete selected row' button active or inactive, depending
 *   on whether a row is selected or not. This function is called
 *   whenever the selection changes.
 *
 **************************************************************************/

static void
onSelectionChanged (GtkTreeSelection *sel, GtkListStore *liststore)
{
	GtkTreeIter  selected_row;

	/* Check if a row is selected or not             */
	/* This will only work in SINGLE or BROWSE mode! */
	if (gtk_tree_selection_get_selected(sel, NULL, &selected_row))
	{
		gtk_widget_set_sensitive(delbutton, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(delbutton, FALSE);
	}
}

/**************************************************************************
 *
 *  create_list_view
 *
 *  sets up the tree view and returns a GtkTreeView widget
 *
 **************************************************************************/

static GtkWidget *
create_list_view (void)
{
	GtkCellRenderer    *renderer;
	GtkTreeViewColumn  *col;
	GtkTreeSelection   *sel;
	GtkListStore       *liststore;
	GtkWidget          *view;

	liststore = gtk_list_store_new(NUM_COLS, G_TYPE_STRING); /* NUM_COLS = 1 */

	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststore));

	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_TEXT);
	gtk_tree_view_column_set_title(col, " Your items here ");

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
	g_signal_connect(sel, "changed", G_CALLBACK(onSelectionChanged), liststore);

	return view;
}


/**************************************************************************
 *
 *  create_entry_hbox
 *
 *  Returns GtkHBox widget with entry and button inside
 *
 **************************************************************************/

static GtkWidget *
create_entry_hbox (void)
{
	GtkWidget *addbutton;
	GtkWidget *entry;
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 2);

	/* when 'return' is pressed, add text to list
	 *  just as if the button had been pressed */
	entry = gtk_entry_new();
	g_signal_connect(entry, "activate", G_CALLBACK(onAddButtonPress), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

	addbutton = gtk_button_new_with_label(" Add text to list ");
	/* using _swapped, so that the first argument to the callback
	 *  is what we specify as 'data' argument here, ie. the entry,
	 *  and data argument to callback is what is the object to
	 *  connect to here, ie. the button */
	g_signal_connect_swapped(addbutton, "clicked", G_CALLBACK(onAddButtonPress), entry);
	gtk_box_pack_start(GTK_BOX(hbox), addbutton, FALSE, FALSE, 0);

	delbutton = gtk_button_new_with_label(" Remove selected row ");
	g_signal_connect(delbutton, "clicked", G_CALLBACK(onDelButtonPress), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), delbutton, FALSE, FALSE, 0);

	/* at beginning no rows, so nothing to delete */
	gtk_widget_set_sensitive(delbutton, FALSE);

	return hbox;
}


/**************************************************************************
 *
 *  main
 *
 **************************************************************************/

int
main (int argc, char **argv)
{
  GtkWidget *window;
	GtkWidget *scrollwin;
	GtkWidget *tophbox;
	GtkWidget *vbox;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  g_signal_connect(window, "delete_event", gtk_main_quit, NULL); /* dirty */

	vbox = gtk_vbox_new(FALSE, 2);

	tophbox = create_entry_hbox();
  gtk_box_pack_start(GTK_BOX(vbox), tophbox, FALSE, FALSE, 0);

	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
	                               GTK_POLICY_AUTOMATIC,
	                               GTK_POLICY_AUTOMATIC);

  treeview = create_list_view();

  gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

  gtk_box_pack_start(GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
