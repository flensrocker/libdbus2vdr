#include <stdio.h>

#include <libdbus2vdr/connection.h>
#include <libdbus2vdr/remote.h>


typedef struct _WorkData  WorkData;

struct _WorkData
{
  GMainLoop         *loop;
  DBus2vdrConnection connection;
  DBus2vdrRemote     remote;
};

gboolean  do_work(gpointer user_data);
void      done_connect(gpointer user_data);
void      done_enable(int reply_code, gpointer user_data);
void      done_disable(int reply_code, gpointer user_data);
void      done_status(gboolean enabled, gpointer user_data);

gboolean  do_work(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  WorkData *work_data = (WorkData*)user_data;
  dbus2vdr_connection_new(&work_data->connection, 0, user_data);
  dbus2vdr_connection_connect(&work_data->connection, G_BUS_TYPE_SYSTEM, done_connect);
  return FALSE;
}

void done_connect(gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  if (dbus2vdr_connection_is_connected(&work_data->connection)) {
     printf("connected\n");
     dbus2vdr_remote_new(&work_data->remote, &work_data->connection, user_data);
     printf("disable remote...\n");
     dbus2vdr_remote_disable(&work_data->remote, done_disable);
     }
}

void done_enable(int reply_code, gpointer user_data)
{
  if (user_data == NULL)
     return;

  WorkData *work_data = (WorkData*)user_data;
  if (reply_code == 250)
     printf("remote is enabled\n");
  else
     printf("remote could not be enabled\n");
  dbus2vdr_remote_delete(&work_data->remote);
  dbus2vdr_connection_delete(&work_data->connection);
  g_main_loop_quit(work_data->loop);
}

void done_disable(int reply_code, gpointer user_data)
{
  if (user_data == NULL)
     return;

  if (reply_code == 250)
     printf("remote is disabled\n");
  else
     printf("remote could not be disabled\n");

  WorkData *work_data = (WorkData*)user_data;
  dbus2vdr_remote_status(&work_data->remote, done_status);
}

void done_status(gboolean enabled, gpointer user_data)
{
  if (user_data == NULL)
     return;

  printf("status of remote: %s\n", enabled ? "on" : "off");

  WorkData *work_data = (WorkData*)user_data;
  dbus2vdr_remote_enable(&work_data->remote, done_enable);
}

int main(int argc, char *argv[])
{
  g_type_init();

  GMainContext *context = NULL;

  WorkData data;

  GSource *source = g_idle_source_new();
  g_source_set_callback(source, do_work, &data, NULL);
  g_source_attach(source, context);

  data.loop = g_main_loop_new(context, FALSE);
  g_main_loop_run(data.loop);
  g_main_loop_unref(data.loop);

  return 0;
}
