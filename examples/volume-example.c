#include <stdio.h>

#include <libdbus2vdr/remote.h>
#include <libdbus2vdr/status.h>


typedef struct _WorkData  WorkData;

struct _WorkData
{
  GMainLoop         *loop;
  DBus2vdrRemote    *remote;
  guint              set_volume_id;
};

gboolean  do_connect(gpointer user_data);

void      get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data);

void      on_set_volume(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data);


gboolean  do_connect(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  dbus2vdr_remote_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                    "de.tvdr.vdr",
                                    "/Remote",
                                    NULL,
                                    get_remote,
                                    user_data);
  return FALSE;
}

void get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  work_data->remote = dbus2vdr_remote_proxy_new_for_bus_finish(res, NULL);
  if (work_data->remote != NULL) {
     GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(work_data->remote));
     if (conn == NULL)
        return;
     work_data->set_volume_id = g_dbus_connection_signal_subscribe(conn,
                                                                   "de.tvdr.vdr",
                                                                   "de.tvdr.vdr.status",
                                                                   "SetVolume",
                                                                   "/Status",
                                                                   NULL,
                                                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                                                   on_set_volume,
                                                                   user_data,
                                                                   NULL);
     }
}

void      on_set_volume(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  gint     volume = -1;
  gboolean absolute = FALSE;
  g_variant_get(parameters, "(ib)", &volume, &absolute);
  printf("volume changed: %d, %s\n", volume, absolute ? "(ABS)" : "(REL)");

  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  if (work_data->remote != NULL) {
     gint abs_volume = -1;
     gboolean mute = FALSE;
     dbus2vdr_remote_call_get_volume_sync(work_data->remote, &abs_volume, &mute, NULL, NULL);
     printf("now: %d (%s)\n", abs_volume, mute ? "muted" : "not muted");
     }
}


int main(int argc, char *argv[])
{
  g_type_init();

  GMainContext *context = NULL;

  WorkData work_data;
  work_data.remote = NULL;
  work_data.set_volume_id = 0;

  GSource *source = g_idle_source_new();
  g_source_set_callback(source, do_connect, &work_data, NULL);
  g_source_attach(source, context);

  work_data.loop = g_main_loop_new(context, FALSE);
  g_main_loop_run(work_data.loop);
  g_main_loop_unref(work_data.loop);

  g_object_unref(work_data.remote);

  return 0;
}
