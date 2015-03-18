
/* a simple test for GuiCellRendererSpin */

#include <gtk/gtk.h>
#include "cellrendererspin.h"

enum
{
  COL_NAME = 0,
  COL_NUMBER,
  NUM_COLS
} ;

static GtkTreeModel *
create_and_fill_model (void)
{
  GtkListStore  *liststore;
  GtkTreeIter    iter;

  liststore = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_FLOAT);

  gtk_list_store_append(liststore, &iter);
  gtk_list_store_set(liststore, &iter, COL_NAME, "M J Smith", COL_NUMBER, 5.0, -1);

  gtk_list_store_append(liststore, &iter);
  gtk_list_store_set(liststore, &iter, COL_NAME, "K F Hemmingwinski", COL_NUMBER, 3.8, -1);

  gtk_list_store_append(liststore, &iter);
  gtk_list_store_set(liststore, &iter, COL_NAME, "P C Juanicini",  COL_NUMBER, 2.5, -1);

  return GTK_TREE_MODEL(liststore);
}


static void
cell_data_func_gpa (GtkTreeViewColumn *col,
                    GtkCellRenderer   *cell,
                    GtkTreeModel      *model,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
	gchar   buf[32];
	GValue  val = {0, };

	gtk_tree_model_get_value(model, iter, COL_NUMBER, &val);

	g_snprintf(buf, sizeof(buf), "%.1f", g_value_get_float(&val));

	g_object_set(cell, "text", buf, NULL);
}


static void
on_gpa_edited (GtkCellRendererText *celltext,
               const gchar         *string_path,
               const gchar         *new_text,
               gpointer             data)
{
	GtkTreeModel *model = GTK_TREE_MODEL(data);
	GtkTreeIter   iter;
	gchar        *name = NULL;
	gfloat        oldval = 0.0;
	gfloat        newval = 0.0;

	gtk_tree_model_get_iter_from_string(model, &iter, string_path);

	gtk_tree_model_get(model, &iter, COL_NAME, &name, COL_NUMBER, &oldval, -1);

	if (sscanf(new_text, "%f", &newval) != 1)
		g_warning("in %s: problem converting string '%s' into float.\n", __FUNCTION__, new_text);

	g_print ("%s:  old GPA = %.1f,  new GPA = %.1f ('%s')\n", name, oldval, newval, new_text);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_NUMBER, newval, -1);

	g_free(name);
}


static GtkWidget *
create_view_and_model (void)
{
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkWidget           *view;
  GtkTreeModel        *model;

  view = gtk_tree_view_new();

  model = create_and_fill_model();

  gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

  g_object_unref(model); /* destroy model automatically with view */

  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);


  /* --- Column #1 --- */
  col = gtk_tree_view_column_new();
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_set_title(col, "Name");
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

  /* --- Column #2 --- */
  col = gtk_tree_view_column_new();
  renderer = gui_cell_renderer_spin_new(0.0, 5.0, 0.1, 1.0, 1.0, 0.1, 1);

	gtk_tree_view_column_set_title(col, "GPA");
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(col, renderer, cell_data_func_gpa, NULL, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	g_object_set(renderer, "editable", TRUE, NULL);

	g_signal_connect(renderer, "edited", G_CALLBACK(on_gpa_edited), model);

  return view;
}


int
main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *view;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "delete_event", gtk_main_quit, NULL); /* dirty */

  view = create_view_and_model();

  gtk_container_add(GTK_CONTAINER(window), view);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
