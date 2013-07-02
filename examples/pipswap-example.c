// header
#include <stdio.h>
#include <string.h>
#include <libdbus2vdr/remote.h>

typedef struct _UserData  UserData;

struct _UserData
{
  UserData       *other;
  const char     *busname;
  GMainLoop      *loop;
  DBus2vdrRemote *remote;
  gchar          *channel;
  gboolean        switched;
};

void  user_data_init(UserData *user_data, UserData *other, const char *busname, GMainLoop *loop);
void  user_data_free(UserData *user_data);

void  get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  get_channel(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  set_channel(GObject *source_object, GAsyncResult *res, gpointer user_data);


// body

void  user_data_init(UserData *user_data, UserData *other, const char *busname, GMainLoop *loop)
{
  user_data->other = other;
  user_data->busname = busname;
  user_data->loop = loop;
  user_data->remote = NULL;
  user_data->channel = NULL;
  user_data->switched = FALSE;
}

void  user_data_free(UserData *user_data)
{
  if (user_data->remote != NULL) {
     g_object_unref(user_data->remote);
     user_data->remote = NULL;
     }
  if (user_data->channel != NULL) {
     g_free(user_data->channel);
     user_data->channel = NULL;
     }
}

void  get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  data->remote = dbus2vdr_remote_proxy_new_for_bus_finish(res, NULL);
  if (data->remote == NULL) {
     printf("can't get remote for %s\n", data->busname);
     g_main_loop_quit(data->loop);
     }
  else {
     printf("got remote for %s\n", data->busname);
     dbus2vdr_remote_call_switch_channel(data->remote, "", NULL, get_channel, data);
     }
}

void  get_channel(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  gint  reply_code = 0;
  dbus2vdr_remote_call_switch_channel_finish(data->remote, &reply_code, &data->channel, res, NULL);
  if ((reply_code != 250) || (data->channel == NULL)) {
     printf("can't get channel for %s\n", data->busname);
     g_main_loop_quit(data->loop);
     }
  else {
     printf("got channel %s for %s\n", data->channel, data->busname);
     int len = strlen(data->channel);
     int i = 0;
     for (i = 0; i < len; i++) {
         if (data->channel[i] == ' ') {
            data->channel[i] = '\0';
            break;
            }
         }
     if (data->other->channel != NULL) {
        dbus2vdr_remote_call_switch_channel(data->remote, data->other->channel, NULL, set_channel, data);
        dbus2vdr_remote_call_switch_channel(data->other->remote, data->channel, NULL, set_channel, data->other);
        }
     }
}

void  set_channel(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  gint   reply_code = 0;
  gchar *reply_message = NULL;
  dbus2vdr_remote_call_switch_channel_finish(data->remote, &reply_code, &reply_message, res, NULL);
  if (reply_code != 250) {
     printf("can't switch channel for %s\n", data->busname);
     g_main_loop_quit(data->loop);
     }
  else {
     printf("switched channel to %s for %s\n", reply_message, data->busname);
     data->switched = TRUE;
     if (data->other->switched == TRUE) {
        printf("done\n");
        g_main_loop_quit(data->loop);
        }
     }
  g_free(reply_message);
}


int main(int argc, char *argv[])
{
  GMainLoop *loop = NULL;
  UserData   vdr;
  UserData   pip;

  g_type_init();
  loop = g_main_loop_new(NULL, FALSE);

  user_data_init(&vdr, &pip, "de.tvdr.vdr", loop);
  user_data_init(&pip, &vdr, "de.tvdr.vdr1", loop);

  dbus2vdr_remote_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                    vdr.busname,
                                    "/Remote", NULL,
                                    get_remote, &vdr);
  dbus2vdr_remote_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                    pip.busname,
                                    "/Remote", NULL,
                                    get_remote, &pip);

  g_main_loop_run(loop);
  printf("quit\n");
  g_main_loop_unref(loop);

  user_data_free(&vdr);
  user_data_free(&pip);

  return 0;
}
