#include <stdio.h>
#include <glib-unix.h>
#include <libdbus2vdr/remote.h>


typedef struct _WorkData  WorkData;

struct _WorkData
{
  GMainLoop         *loop;
  GDBusConnection   *connection;
  DBus2vdrRemote    *remote;
  guint              set_volume_id;
};

gboolean on_hangup(gpointer user_data);

void on_connect(GObject *source_object, GAsyncResult *res, gpointer user_data);
void get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data);
void on_set_volume(GDBusConnection *connection,
                   const gchar *sender_name,
                   const gchar *object_path,
                   const gchar *interface_name,
                   const gchar *signal_name,
                   GVariant *parameters,
                   gpointer user_data);


gboolean on_hangup(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  WorkData *work_data = (WorkData*)user_data;
  printf("hangup\n");
  g_main_loop_quit(work_data->loop);
  return FALSE;
}

void on_connect(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  work_data->connection = g_bus_get_finish(res, NULL);
  if (work_data->connection == NULL) {
     printf("can't connect to system bus\n");
     g_main_loop_quit(work_data->loop);
     return;
     }

  dbus2vdr_remote_proxy_new(work_data->connection,
                            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                            "de.tvdr.vdr",
                            "/Remote",
                            NULL,
                            get_remote,
                            user_data);
}

void get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  work_data->remote = dbus2vdr_remote_proxy_new_finish(res, NULL);
  if (work_data->remote == NULL) {
     printf("can't get remote object\n");
     g_main_loop_quit(work_data->loop);
     return;
     }

  work_data->set_volume_id = g_dbus_connection_signal_subscribe(work_data->connection,
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

void on_set_volume(GDBusConnection *connection,
                   const gchar *sender_name,
                   const gchar *object_path,
                   const gchar *interface_name,
                   const gchar *signal_name,
                   GVariant *parameters,
                   gpointer user_data)
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
  work_data.connection = NULL;
  work_data.remote = NULL;
  work_data.set_volume_id = 0;

  g_unix_signal_add(SIGHUP, on_hangup, &work_data);
  g_bus_get(G_BUS_TYPE_SYSTEM, NULL, on_connect, &work_data);

  work_data.loop = g_main_loop_new(context, FALSE);
  g_main_loop_run(work_data.loop);
  g_main_loop_unref(work_data.loop);

  g_object_unref(work_data.remote);

  printf("exit\n");
  return 0;
}
