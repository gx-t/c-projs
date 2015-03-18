#ifndef __RT_H__
#define __RT_H__
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "tv.h"
#include "fs.h"
#include "sha256.h"

struct RtApp
{
  const gchar* strTitle;
  GtkWindow*    mw;
  void (*PassEvent)();
  gint (*MsgBoxYesNo)(const gchar* msg);
};
extern struct RtApp g_rt;

#endif //__RT_H__

