#include "remote.h"

#include "connection.h"

#define DBUS2VDR_REMOTE_INTERFACE  DBUS2VDR_NAMESPACE".remote"
#define DBUS2VDR_REMOTE_OBJECT     "/Remote"

void dbus2vdr_remote_enable_finish(GObject *source_object, GAsyncResult *res, gpointer user_data);
void dbus2vdr_remote_disable_finish(GObject *source_object, GAsyncResult *res, gpointer user_data);
void dbus2vdr_remote_status_finish(GObject *source_object, GAsyncResult *res, gpointer user_data);

void dbus2vdr_remote_new(DBus2vdrRemote *remote, DBus2vdrConnection *connection, gpointer user_data)
{
  if (remote == NULL)
     return;

  remote->user_data = user_data;
  remote->connection = connection;
  remote->on_enable = NULL;
  remote->on_disable = NULL;
  remote->on_status = NULL;
}

void dbus2vdr_remote_delete(DBus2vdrRemote *remote)
{
  if (remote == NULL)
     return;
}

gboolean dbus2vdr_remote_enable(DBus2vdrRemote *remote, DBus2vdrEnableFunc on_enable)
{
  if ((remote == NULL)
   || !dbus2vdr_connection_is_connected(remote->connection)
   || (remote->on_enable != NULL))
     return FALSE;

  remote->on_enable = on_enable;
  g_dbus_connection_call(remote->connection->connection,
                         dbus2vdr_connection_busname(remote->connection),
                         DBUS2VDR_REMOTE_OBJECT, DBUS2VDR_REMOTE_INTERFACE, "Enable",
                         NULL, G_VARIANT_TYPE("(is)"),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         dbus2vdr_remote_enable_finish, remote);
  return TRUE;
}

void dbus2vdr_remote_enable_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  DBus2vdrRemote *remote = (DBus2vdrRemote*)user_data;
  GVariant *reply = g_dbus_connection_call_finish(remote->connection->connection, res, NULL);
  int reply_code = 0;
  const gchar *reply_message = NULL;

  if (reply != NULL)
     g_variant_get(reply, "(i&s)", &reply_code, &reply_message);

  if (remote->on_enable != NULL) {
     remote->on_enable(reply_code, remote->user_data);
     remote->on_enable = NULL;
     }

  if (reply != NULL)
     g_variant_unref(reply);
}

gboolean dbus2vdr_remote_disable(DBus2vdrRemote *remote, DBus2vdrDisableFunc on_disable)
{
  if ((remote == NULL)
   || !dbus2vdr_connection_is_connected(remote->connection)
   || (remote->on_disable != NULL))
     return FALSE;

  remote->on_disable = on_disable;
  g_dbus_connection_call(remote->connection->connection,
                         dbus2vdr_connection_busname(remote->connection),
                         DBUS2VDR_REMOTE_OBJECT, DBUS2VDR_REMOTE_INTERFACE, "Disable",
                         NULL, G_VARIANT_TYPE("(is)"),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         dbus2vdr_remote_disable_finish, remote);
  return TRUE;
}

void dbus2vdr_remote_disable_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  DBus2vdrRemote *remote = (DBus2vdrRemote*)user_data;
  GVariant *reply = g_dbus_connection_call_finish(remote->connection->connection, res, NULL);
  int reply_code = 0;
  const gchar *reply_message = NULL;

  if (reply != NULL)
     g_variant_get(reply, "(i&s)", &reply_code, &reply_message);

  if (remote->on_disable != NULL) {
     remote->on_disable(reply_code, remote->user_data);
     remote->on_disable = NULL;
     }

  if (reply != NULL)
     g_variant_unref(reply);
}

gboolean dbus2vdr_remote_status(DBus2vdrRemote *remote, DBus2vdrStatusFunc on_status)
{
  if ((remote == NULL)
   || !dbus2vdr_connection_is_connected(remote->connection)
   || (remote->on_status != NULL))
     return FALSE;

  remote->on_status = on_status;
  g_dbus_connection_call(remote->connection->connection,
                         dbus2vdr_connection_busname(remote->connection),
                         DBUS2VDR_REMOTE_OBJECT, DBUS2VDR_REMOTE_INTERFACE, "Status",
                         NULL, G_VARIANT_TYPE("(b)"),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         dbus2vdr_remote_status_finish, remote);
  return TRUE;
}

void dbus2vdr_remote_status_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  DBus2vdrRemote *remote = (DBus2vdrRemote*)user_data;
  GVariant *reply = g_dbus_connection_call_finish(remote->connection->connection, res, NULL);
  gboolean status = TRUE;

  if (reply != NULL)
     g_variant_get(reply, "(b)", &status);

  if (remote->on_status != NULL) {
     remote->on_status(status, remote->user_data);
     remote->on_status = NULL;
     }

  if (reply != NULL)
     g_variant_unref(reply);
}
