#include <stdio.h>
#include <glib-unix.h>
#include <libdbus2vdr/remote.h>

#define RECONNECT_SECONDS 5

typedef struct _WorkData  WorkData;

struct _WorkData
{
  gchar             *bus_address;
  GMainLoop         *loop;
  gboolean           reconnect;
  GDBusConnection   *connection;
  gboolean           retrieve_remote;
  DBus2vdrRemote    *remote;
  guint              set_volume_id;
};

void work_data_create(WorkData *work_data)
{
  work_data->bus_address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
  work_data->loop = NULL;
  work_data->reconnect = FALSE;
  work_data->connection = NULL;
  work_data->retrieve_remote = FALSE;
  work_data->remote = NULL;
  work_data->set_volume_id = 0;
}

void work_data_destroy(WorkData *work_data)
{
  if (work_data->connection != NULL) {
     if (work_data->set_volume_id > 0) {
         g_dbus_connection_signal_unsubscribe(work_data->connection, work_data->set_volume_id);
         work_data->set_volume_id = 0;
         }
     if (work_data->remote != NULL) {
        g_object_unref(work_data->remote);
        work_data->remote = NULL;
        }
     g_object_unref(work_data->connection);
     work_data->connection = NULL;
     }
  if (work_data->loop != NULL) {
     g_main_loop_unref(work_data->loop);
     work_data->loop = NULL;
     }
  g_free(work_data->bus_address);
  work_data->bus_address = NULL;
}


gboolean on_hangup(gpointer user_data);

void     on_connect(GObject *source_object, GAsyncResult *res, gpointer user_data);
gboolean do_reconnect(gpointer user_data);
void     on_close(GDBusConnection *connection,
                  gboolean         remote_peer_vanished,
                  GError          *error,
                  gpointer         user_data);

gboolean retrieve_remote(gpointer user_data);
void     get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data);

void     on_set_volume(GDBusConnection *connection,
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
  work_data->connection = g_dbus_connection_new_for_address_finish(res, NULL);
  if (work_data->connection == NULL) {
     if (!work_data->reconnect) {
        work_data->reconnect = TRUE;
        printf("can't connect to system bus, register reconnect handler\n");
        }
     g_timeout_add_seconds(RECONNECT_SECONDS, do_reconnect, user_data);
     return;
     }

  printf("connected to system bus\n");
  work_data->reconnect = FALSE;
  g_dbus_connection_set_exit_on_close(work_data->connection, FALSE);
  g_signal_connect(work_data->connection, "closed", G_CALLBACK(on_close), user_data);

  printf("retrieve remote\n");
  retrieve_remote(user_data);
}

gboolean do_reconnect(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  WorkData *work_data = (WorkData*)user_data;
  g_dbus_connection_new_for_address(work_data->bus_address,
                                    G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                    NULL, NULL, on_connect, user_data);
  return FALSE;
}

void on_close(GDBusConnection *connection,
              gboolean         remote_peer_vanished,
              GError          *error,
              gpointer         user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  printf("connection closed\n");
  work_data->set_volume_id = 0;
  if (work_data->remote != NULL) {
     g_object_unref(work_data->remote);
     work_data->remote = NULL;
     }
  if (work_data->connection != NULL) {
     g_object_unref(work_data->connection);
     work_data->connection = NULL;
     }
  do_reconnect(user_data);
}

gboolean retrieve_remote(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  WorkData *work_data = (WorkData*)user_data;
  dbus2vdr_remote_proxy_new(work_data->connection,
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
  work_data->remote = dbus2vdr_remote_proxy_new_finish(res, NULL);
  if (work_data->remote == NULL) {
     if (!work_data->retrieve_remote) {
        work_data->retrieve_remote = TRUE;
        printf("can't get remote object\n");
        }
     g_timeout_add_seconds(RECONNECT_SECONDS, retrieve_remote, user_data);
     return;
     }

  printf("register SetVolume signal\n");
  work_data->retrieve_remote = FALSE;
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
  printf("volume changed: %d (%s)\n", volume, absolute ? "absolute" : "relative");

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

  WorkData work_data;
  work_data_create(&work_data);

  g_unix_signal_add(SIGHUP, on_hangup, &work_data);
  g_unix_signal_add(SIGINT, on_hangup, &work_data);

  do_reconnect(&work_data);

  work_data.loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(work_data.loop);

  work_data_destroy(&work_data);

  printf("exit\n");
  return 0;
}
