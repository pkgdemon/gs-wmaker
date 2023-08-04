#include <X11/Xlib.h>
#include <dbus/dbus.h>

int dbus_init(void);
void dbus_run_loop(void);
void xFillWindowsReply(int workspace, DBusMessage *reply);
char* xGetActiveWindow(char*);
char* xGetActiveWorkspace(char*);
int send_event_signal(char* name, char* value);
