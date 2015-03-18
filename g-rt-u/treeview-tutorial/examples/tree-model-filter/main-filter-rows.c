#include <gtk/gtk.h>
#include "guitreemodelfilter.h"

/**************************************************************
 *                                                            *
 *   Short demonstration of how to use GtkTreeModelFilter     *
 *    to filter out certain rows of a tree model/tree view    *
 *                                                            *
 *   Note: we changed the namespace from GtkTreeModelFilter   *
 *         to GuiTreeModelFilter here, in order to make this  *
 *         demo work with Gtk+ versions < 2.3                 *
 *                                                            *
 *   GtkTreeModelFilter will be in Gtk+-2.4 and is already    *
 *         in the (unstable) Gtk+-2.3 development series      *
 *                                                            *
 *   part of the GtkTreeView tutorial                         *
 *   written by Tim Mueller <tim @at@ centricular .dot. net>  *
 *                                                            *
 **************************************************************/

enum
{
	COL_NAME = 0,
	COL_YEAR,
	NUM_COLS
};

static GtkTreeModel      *filter;  /* NULL */
static guint              minyear; /* 0 */


/**************************************************************
 *
 *   filter_visible_function
 *
 *   This is where the action is. Returns TRUE if
 *    the row should be visible, and FALSE if the
 *    row should be hidden
 *
 **************************************************************/

static gboolean
filter_visible_function (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	gint   year;

	gtk_tree_model_get(model, iter, COL_YEAR, &year, -1);

	if (year < minyear)
		return FALSE;

	return TRUE;
}


/**************************************************************
 *
 *   fill_list_store
 *
 **************************************************************/

static void
fill_list_store (GtkListStore *store)
{
	const gchar *names[] = { "John B.", "Jane Z.", "Carl O.", "Owen P.", "Jeremy F.",
	                         "Michael M.", "Ute D.", "Akira T.", "Thomas F.", "Matthew J."};
	guint        i;

	for (i = 0;  i < G_N_ELEMENTS(names);  ++i)
	{
		GtkTreeIter  iter;
		guint        randomyear;

		randomyear = 1900 + (g_random_int() % 103);

		gtk_list_store_append(store, &iter);

		gtk_list_store_set(store, &iter,
		                   COL_NAME, names[i],
		                   COL_YEAR, randomyear,
		                   -1);
	}
}

/**************************************************************
 *
 *   create_models_and_view
 *
 **************************************************************/

GtkWidget *
create_models_and_view (void)
{
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *col;
	GtkListStore      *store;
	GtkWidget         *view;

	store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_INT);

	filter = gui_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);

	gui_tree_model_filter_set_visible_func(GUI_TREE_MODEL_FILTER(filter),
	                                       filter_visible_function,
	                                       NULL,
	                                       NULL);

	fill_list_store(store);

	/* set up the view */
	view = gtk_tree_view_new_with_model(filter);

	/* column 1: name */
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_NAME);
	gtk_tree_view_column_set_title(col, "Name");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	/* column 2: year of birth - note: COL_YEAR is of G_TYPE_INT,
	 *           but it will automatically converted to a string
	 *           (the "text" property/attribute requires a string) */
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_YEAR);
	gtk_tree_view_column_set_title(col, "Year Born");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	return view;
}


/**************************************************************
 *
 *   onSpinButtonValueChanged
 *
 **************************************************************/

static void
onSpinButtonValueChanged (GtkSpinButton *spin, gpointer data)
{
	g_assert(filter != NULL);

	minyear = gtk_spin_button_get_value_as_int(spin);

	gui_tree_model_filter_refilter(GUI_TREE_MODEL_FILTER(filter));

	g_print("%s: %d\n", __FUNCTION__, minyear);
}

/**************************************************************
 *
 *   create_spin_button_row
 *
 **************************************************************/

static GtkWidget *
create_spin_button_row (void)
{
	GtkWidget *hbox, *spinbutton, *label;

	hbox = gtk_hbox_new(FALSE, 2);

	spinbutton = gtk_spin_button_new_with_range(1900.0, 2004.0, 1.0);

	g_signal_connect(spinbutton, "value-changed", G_CALLBACK(onSpinButtonValueChanged), NULL);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton), 1900.0);

	label = gtk_label_new("Show all persons born after ");

	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spinbutton, FALSE, FALSE, 0);

	return hbox;
}

/**************************************************************
 *
 *   fill_window
 *
 **************************************************************/

static void
fill_window(GtkWidget *mainwindow)
{
	GtkWidget *vbox, *view, *spinrowhbox;

	spinrowhbox = create_spin_button_row();

	view = create_models_and_view();

	vbox = gtk_vbox_new(FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox), spinrowhbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(mainwindow), vbox);
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

	fill_window(mainwindow);

	gtk_widget_show_all(mainwindow);

	gtk_main();

	return 0;
}
