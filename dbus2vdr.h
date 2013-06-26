#ifndef _libdbus2vdr_types_h
#define _libdbus2vdr_types_h

#include <gio/gio.h>

#define DBUS2VDR_BUSNAME    "de.tvdr.vdr"
#define DBUS2VDR_NAMESPACE  "de.tvdr.vdr"


typedef struct _DBus2vdrConnection  DBus2vdrConnection;

typedef void (*DBus2vdrConnectFunc)(gpointer user_data);

struct _DBus2vdrConnection
{
  gpointer            user_data;

  int                 instance_id;
  GDBusConnection    *connection;
  gboolean            connected;
  gchar              *busname;

  gboolean            unref_connection;
  DBus2vdrConnectFunc on_connect;
};


typedef struct _DBus2vdrRemote  DBus2vdrRemote;

typedef void (*DBus2vdrEnableFunc)(int reply_code, gpointer user_data);
typedef void (*DBus2vdrDisableFunc)(int reply_code, gpointer user_data);
typedef void (*DBus2vdrStatusFunc)(gboolean enabled, gpointer user_data);

struct _DBus2vdrRemote
{
  gpointer            user_data;

  DBus2vdrConnection *connection;

  DBus2vdrEnableFunc  on_enable;
  DBus2vdrDisableFunc on_disable;
  DBus2vdrStatusFunc  on_status;
};

#endif
