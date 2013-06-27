#include <stdio.h>

#include <libdbus2vdr/remote.h>


typedef struct _WorkData  WorkData;

struct _WorkData
{
  GMainLoop         *loop;
  DBus2vdrRemote    *remote;
};

gboolean  do_work(gpointer user_data);

void      done_connect(GObject *source_object, GAsyncResult *res, gpointer user_data);
void      done_enable(GObject *source_object, GAsyncResult *res, gpointer user_data);
void      done_disable(GObject *source_object, GAsyncResult *res, gpointer user_data);
void      done_status(GObject *source_object, GAsyncResult *res, gpointer user_data);

gboolean  do_work(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  WorkData *work_data = (WorkData*)user_data;
  dbus2vdr_remote_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                    "de.tvdr.vdr",
                                    "/Remote",
                                    NULL,
                                    done_connect,
                                    user_data);
  return FALSE;
}

void done_connect(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  work_data->remote = dbus2vdr_remote_proxy_new_for_bus_finish(res, NULL);
  if (work_data->remote != NULL) {
     printf("connected\n");
     printf("disable remote...\n");
     dbus2vdr_remote_call_disable(work_data->remote, NULL, done_disable, user_data);
     }
}

void done_enable(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  gint reply_code = 0;
  gchar *reply_message = NULL;
  dbus2vdr_remote_call_enable_finish(work_data->remote, &reply_code, &reply_message, res, NULL);
  if (reply_code == 250)
     printf("remote is enabled\n");
  else
     printf("remote could not be enabled\n");
  g_free(reply_message);

  g_main_loop_quit(work_data->loop);
}

void done_disable(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  gint reply_code = 0;
  gchar *reply_message = NULL;
  dbus2vdr_remote_call_disable_finish(work_data->remote, &reply_code, &reply_message, res, NULL);
  if (reply_code == 250)
     printf("remote is disabled\n");
  else
     printf("remote could not be disabled\n");
  g_free(reply_message);

  dbus2vdr_remote_call_status(work_data->remote, NULL, done_status, user_data);
}

void done_status(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  gboolean enabled = TRUE;
  dbus2vdr_remote_call_status_finish(work_data->remote, &enabled, res, NULL);
  printf("status of remote: %s\n", enabled ? "on" : "off");

  dbus2vdr_remote_call_enable(work_data->remote, NULL, done_enable, user_data);
}

int main(int argc, char *argv[])
{
  g_type_init();

  GMainContext *context = NULL;

  WorkData work_data;

  GSource *source = g_idle_source_new();
  g_source_set_callback(source, do_work, &work_data, NULL);
  g_source_attach(source, context);

  work_data.loop = g_main_loop_new(context, FALSE);
  g_main_loop_run(work_data.loop);
  g_main_loop_unref(work_data.loop);

  g_object_unref(work_data.remote);

  return 0;
}
