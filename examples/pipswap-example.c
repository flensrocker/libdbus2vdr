// header

/* swap channels of two vdr instances
 *
 * get object /Setup
 * get primary device (setup.Get "PrimaryDVB")
 * get object /Remote
 * get current channel (remote.SwitchChannel "")
 * subscribe to ChannelSwitch signal
 * set channel (remote.SwitchChannel "channel number")
 * wait for ChannelSwitch signal for the primary device
 * quit
 */

#include <stdio.h>
#include <string.h>
#include <libdbus2vdr/remote.h>
#include <libdbus2vdr/setup.h>

typedef struct _UserData  UserData;

struct _UserData
{
  UserData       *other;
  const char     *busname;
  GMainLoop      *loop;
  DBus2vdrRemote *remote;
  DBus2vdrSetup  *setup;
  gint            primary_device;
  guint           signal_id;
  gchar          *channel;
  gboolean        switch_queued;
  gboolean        switch_done;
};

void  user_data_init(UserData *user_data, UserData *other, const char *busname, GMainLoop *loop);
void  user_data_free(UserData *user_data);

void  get_setup(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  get_device(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  get_channel(GObject *source_object, GAsyncResult *res, gpointer user_data);
void  set_channel(GObject *source_object, GAsyncResult *res, gpointer user_data);

void  on_channel_switch(GDBusConnection *connection,
                        const gchar *sender_name,
                        const gchar *object_path,
                        const gchar *interface_name,
                        const gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data);

gboolean do_quit(gpointer user_data);

// body

void  user_data_init(UserData *user_data, UserData *other, const char *busname, GMainLoop *loop)
{
  user_data->other = other;
  user_data->busname = busname;
  user_data->loop = loop;
  user_data->remote = NULL;
  user_data->setup = NULL;
  user_data->primary_device = 0;
  user_data->signal_id = 0;
  user_data->channel = NULL;
  user_data->switch_queued = FALSE;
  user_data->switch_done = FALSE;
}

void  user_data_free(UserData *user_data)
{
  if (user_data->remote != NULL) {
     g_object_unref(user_data->remote);
     user_data->remote = NULL;
     }
  if (user_data->setup != NULL) {
     g_object_unref(user_data->setup);
     user_data->setup = NULL;
     }
  if (user_data->channel != NULL) {
     g_free(user_data->channel);
     user_data->channel = NULL;
     }
}

void  get_setup(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  data->setup = dbus2vdr_setup_proxy_new_for_bus_finish(res, NULL);
  if (data->setup == NULL) {
     printf("%s: can't get setup\n", data->busname);
     do_quit(data);
     }
  else {
     printf("%s: got setup\n", data->busname);
     dbus2vdr_setup_call_get(data->setup, "PrimaryDVB", NULL, get_device, data);
     }
}

void  get_device(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  GVariant *reply = NULL;
  gint      reply_code = 0;
  gchar    *reply_message = NULL;
  if (!dbus2vdr_setup_call_get_finish(data->setup, &reply, &reply_code, &reply_message, res, NULL)) {
     printf("%s: can't get primary device\n", data->busname);
     do_quit(data);
     }
  else {
     GVariant *device = g_variant_get_child_value(reply, 0);
     g_variant_get(device, "i", &data->primary_device);
     printf("%s: primary device is %d\n", data->busname, data->primary_device);

     dbus2vdr_remote_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                       G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                       data->busname,
                                       "/Remote", NULL,
                                       get_remote, data);
     }
  if (reply != NULL)
     g_variant_unref(reply);
  g_free(reply_message);
}

void  get_remote(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  data->remote = dbus2vdr_remote_proxy_new_for_bus_finish(res, NULL);
  if (data->remote == NULL) {
     printf("%s: can't get remote\n", data->busname);
     do_quit(data);
     }
  else {
     printf("%s: got remote\n", data->busname);
     data->signal_id = g_dbus_connection_signal_subscribe(g_dbus_proxy_get_connection(G_DBUS_PROXY(data->remote)),
                                                          data->busname,
                                                          "de.tvdr.vdr.status",
                                                          "ChannelSwitch",
                                                          "/Status",
                                                          NULL,
                                                          G_DBUS_SIGNAL_FLAGS_NONE,
                                                          on_channel_switch,
                                                          user_data,
                                                          NULL);
     dbus2vdr_remote_call_switch_channel(data->remote, "", NULL, get_channel, data);
     }
}

void  get_channel(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  gint  reply_code = 0;
  if (!dbus2vdr_remote_call_switch_channel_finish(data->remote, &reply_code, &data->channel, res, NULL)
   || (reply_code != 250)
   || (data->channel == NULL)) {
     printf("%s: can't get channel\n", data->busname);
     do_quit(data);
     }
  else {
     printf("%s: got channel %s\n", data->busname, data->channel);
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
  if (!dbus2vdr_remote_call_switch_channel_finish(data->remote, &reply_code, &reply_message, res, NULL)
   ||(reply_code != 250)) {
     printf("%s: can't switch channel\n", data->busname);
     do_quit(data);
     }
  else {
     printf("%s: queued switch to channel %s\n", data->busname, reply_message);
     data->switch_queued = TRUE;
     }
  g_free(reply_message);
}

void  on_channel_switch(GDBusConnection *connection,
                        const gchar *sender_name,
                        const gchar *object_path,
                        const gchar *interface_name,
                        const gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
  if (user_data == NULL)
     return;

  UserData *data = (UserData*)user_data;
  if (!data->switch_queued)
     return;
  gint     device_number;
  gint     channel_number;
  gboolean live_view;
  g_variant_get(parameters, "(iib)", &device_number, &channel_number, &live_view);
  if ((channel_number > 0) && (data->primary_device == device_number)) {
     printf("%s: switched to channel %d\n", data->busname, channel_number);
     data->switch_done = TRUE;
     if (data->other->switch_done) {
        printf("done\n");
        do_quit(data);
        }
     }
}

gboolean do_quit(gpointer user_data)
{
  if (user_data == NULL)
     return FALSE;

  UserData *data = (UserData*)user_data;
  printf("quit\n");
  g_main_loop_quit(data->loop);
  return FALSE;
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

  // quit program after 10 seconds if something hangs
  g_timeout_add_seconds(10, do_quit, &vdr);

  dbus2vdr_setup_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                   G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                   vdr.busname,
                                   "/Setup", NULL,
                                   get_setup, &vdr);
  dbus2vdr_setup_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                   G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                   pip.busname,
                                   "/Setup", NULL,
                                   get_setup, &pip);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  user_data_free(&vdr);
  user_data_free(&pip);

  return 0;
}
