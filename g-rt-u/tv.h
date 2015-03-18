#ifndef __TV_H__
#define __TV_H__

struct TV
{
  GtkTreeView*    tv;
  GtkTreeStore*   ts;
  struct TV_EVT*  evt;
  GtkScrolledWindow* (*CreateTree)();
  void (*AddLogLine)              (const char* s0, const char* s1);
  void (*DeleteChildren)          (const char* tvPath);
  void (*Expand)                  (const char* tvPath, gboolean expand);
  void (*RemovePathIntersection)  (gchar* path, const gchar* tvPath);
  gboolean (*ForEachValue)        (const gchar* tvPath, gpointer data);
  gboolean (*fForEachProc)        (gchar* value, gpointer data);
};
extern struct TV g_tv;

#endif //__TV_H__

