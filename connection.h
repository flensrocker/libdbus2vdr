#ifndef _libdbus2vdr_connection_h
#define _libdbus2vdr_connection_h

#include "dbus2vdr.h"

void         dbus2vdr_connection_new(DBus2vdrConnection *connection, int instance_id, gpointer user_data);
void         dbus2vdr_connection_delete(DBus2vdrConnection *connection);

gboolean     dbus2vdr_connection_connect(DBus2vdrConnection *connection, GBusType bus_type, DBus2vdrConnectFunc on_connect);

gboolean     dbus2vdr_connection_is_connected(DBus2vdrConnection *connection);
const gchar* dbus2vdr_connection_busname(DBus2vdrConnection *connection);
int          dbus2vdr_connection_instance_id(DBus2vdrConnection *connection);

#endif
