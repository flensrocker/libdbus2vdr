#include "connection.h"

void dbus2vdr_connection_connect_finish(GObject *source_object, GAsyncResult *res, gpointer user_data);

void     dbus2vdr_connection_new(DBus2vdrConnection *connection, int instance_id, gpointer user_data)
{
  if (connection == NULL)
     return;

  connection->user_data = user_data;
  connection->instance_id = instance_id;
  connection->connection = NULL;
  connection->unref_connection = FALSE;
  connection->connected = FALSE;
  if (instance_id == 0)
     connection->busname = g_strdup(DBUS2VDR_BUSNAME);
  else
     connection->busname = g_strdup_printf("%s%d", DBUS2VDR_BUSNAME, instance_id);
  connection->on_connect = NULL;
}

void dbus2vdr_connection_delete(DBus2vdrConnection *connection)
{
  if (connection == NULL)
     return;

  if (connection->connection != NULL) {
     if (connection->unref_connection)
        g_object_unref(connection->connection);
     connection->connection = NULL;
     }

  if (connection->busname != NULL) {
     g_free(connection->busname);
     connection->busname = NULL;
     }
}

gboolean dbus2vdr_connection_connect(DBus2vdrConnection *connection, GBusType bus_type, DBus2vdrConnectFunc on_connect)
{
  if ((connection == NULL)
   || (connection->on_connect != NULL)
   || dbus2vdr_connection_is_connected(connection))
     return FALSE;

  connection->on_connect = on_connect;
  g_bus_get(bus_type, NULL, dbus2vdr_connection_connect_finish, connection);
  return TRUE;
}

void dbus2vdr_connection_connect_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  if (user_data == NULL)
     return;

  DBus2vdrConnection *connection = (DBus2vdrConnection*)user_data;
  connection->connection = g_bus_get_finish(res, NULL);
  if (connection->connection != NULL) {
     connection->connected = TRUE;
     g_dbus_connection_set_exit_on_close(connection->connection, FALSE);
     }
  else
     connection->connected = FALSE;
  if (connection->on_connect != NULL) {
     connection->on_connect(connection->user_data);
     connection->on_connect = NULL;
     }
}

gboolean dbus2vdr_connection_is_connected(DBus2vdrConnection *connection)
{
  return ((connection != NULL)
       && (connection->connection != NULL)
       && connection->connected);
}

const gchar* dbus2vdr_connection_busname(DBus2vdrConnection *connection)
{
  if (connection == NULL)
     return NULL;
  return connection->busname;
}

int          dbus2vdr_connection_instance_id(DBus2vdrConnection *connection)
{
  if (connection == NULL)
     return -1;
  return connection->instance_id;
}
