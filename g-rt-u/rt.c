//gcc `pkg-config --cflags --libs gtk+-2.0` -g tree_test.c -o tree_test
//gcc `pkg-config --cflags --libs gtk+-2.0` -O2 -s tree_test.c -o tree_test

#include "rt.h"

static void RtPassEvent()
{
  while (gtk_events_pending())
    gtk_main_iteration();
}

static gint RtMsgBoxYesNo(const gchar* msg)
{
  gint res;
  GtkMessageDialog* mb = (GtkMessageDialog*)gtk_message_dialog_new(g_rt.mw, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, msg, NULL);
  gtk_window_set_title((GtkWindow*)mb, g_rt.strTitle);
  res = gtk_dialog_run((GtkDialog*)mb);
  gtk_widget_destroy((GtkWidget*)mb);
  return res;
}

//******************************************************************************
//main
int main(int argc, char* argv[])
{
  gtk_init(&argc, &argv);
  g_rt.mw = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(g_rt.mw, g_rt.strTitle);
  gtk_window_set_default_size((GtkWindow*)g_rt.mw, 600, 400);
  g_signal_connect((GtkObject*)g_rt.mw, "delete_event", GTK_SIGNAL_FUNC(gtk_main_quit), 0);
  gtk_container_add((GtkContainer*)g_rt.mw, (GtkWidget*)g_tv.CreateTree());
  gtk_widget_show_all((GtkWidget*)g_rt.mw);
  gtk_main();
  return 0;
}

struct RtApp g_rt =
{
  .strTitle = "Report Tree"
  , .PassEvent = RtPassEvent
  , .MsgBoxYesNo = RtMsgBoxYesNo
};

