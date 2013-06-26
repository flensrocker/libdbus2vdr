#ifndef _libdbus2vdr_remote_h
#define _libdbus2vdr_remote_h

#include "dbus2vdr.h"

void     dbus2vdr_remote_new(DBus2vdrRemote *remote, DBus2vdrConnection *connection, gpointer user_data);
void     dbus2vdr_remote_delete(DBus2vdrRemote *remote);

gboolean dbus2vdr_remote_enable(DBus2vdrRemote *remote, DBus2vdrEnableFunc on_enable);
gboolean dbus2vdr_remote_disable(DBus2vdrRemote *remote, DBus2vdrDisableFunc on_disable);
gboolean dbus2vdr_remote_status(DBus2vdrRemote *remote, DBus2vdrStatusFunc on_status);

#endif
