#include <libdbus2vdr/remote.h>

int main(int argc, char *argv[])
{
  int rc = 0;
  DBus2vdrRemote *remoteVDR = NULL;
  DBus2vdrRemote *remotePIP = NULL;
  gint replyCodeVDR = 0;
  gint replyCodePIP = 0;
  gchar *channelVDR = NULL;
  gchar *channelPIP = NULL;
  gchar *channel = NULL;

  g_type_init();

  remoteVDR = dbus2vdr_remote_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                     G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                                     "de.tvdr.vdr",
                                                     "/Remote",
                                                     NULL, NULL);
  if (remoteVDR == NULL) {
     rc++;
     goto unref;
     }

  remotePIP = dbus2vdr_remote_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                     G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                                     "de.tvdr.vdr1",
                                                     "/Remote",
                                                     NULL, NULL);
  if (remotePIP == NULL) {
     rc++;
     goto unref;
     }

  if (!dbus2vdr_remote_call_switch_channel_sync(remoteVDR, "", &replyCodeVDR, &channelVDR, NULL, NULL)) {
     rc++;
     goto unref;
     }

  if (!dbus2vdr_remote_call_switch_channel_sync(remotePIP, "", &replyCodePIP, &channelPIP, NULL, NULL)) {
     rc++;
     goto unref;
     }

  dbus2vdr_remote_call_switch_channel_sync(remotePIP, channelVDR, &replyCodePIP, &channel, NULL, NULL);
  g_free(channel);
  dbus2vdr_remote_call_switch_channel_sync(remoteVDR, channelPIP, &replyCodeVDR, &channel, NULL, NULL);
  g_free(channel);

unref:
  g_object_unref(remoteVDR);
  g_object_unref(remotePIP);
  g_free(channelVDR);
  g_free(channelPIP);

  return rc;
}
