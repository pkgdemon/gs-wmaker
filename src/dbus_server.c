#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <X11/Xlib.h>
#include "framewin.h"
#include "window.h"
#include "event.h"
#include "actions.h"
#include "workspace.h"
#include "dbus_server.h"

char* xGetActiveWindow(char* buff)
{
	for (int i = 0; i < w_global.screen_count; i++) {
		WScreen *scr = wScreenWithNumber(i);
		WWindow *wwin = scr->focused_window;

		while (wwin) {
			if (wwin->frame && wwin->frame->flags.state == WS_FOCUSED) {
				int ws = wwin->frame->workspace + 1;
				sprintf(buff, "%ld/%d/%s/%s", wwin->client_win, ws, wwin->wm_class, wwin->wm_instance);
				return buff;
			}
			wwin = wwin->prev;
		}
	}
	buff[0] = '\0';
	return buff;
}

char* xGetActiveWorkspace(char* buff)
{
	WScreen *scr = wScreenWithNumber(0);
	sprintf(buff, "%d", scr->current_workspace + 1);
	return buff;
}

const char *version = "1.0";
GMainContext *maincontext = NULL;
DBusConnection *mainconn = NULL;
static char last_active_windowid[300];
static int  last_active_windowid_c = 0;

/*
 * This is the XML string describing the interfaces, methods and
 * signals implemented by our 'Server' object. It's used by the
 * 'Introspect' method of 'org.freedesktop.DBus.Introspectable'
 * interface.
 *
 * Currently our tiny server implements only 3 interfaces:
 *
 *    - org.freedesktop.DBus.Introspectable
 *    - org.freedesktop.DBus.Properties
 *    - org.wmaker.WMInterface
 *
 * 'org.wmaker.WMInterface' methods:
 *
 *    - Ping(): makes the server answering the string 'Pong'.
 *              It takes no arguments.
 */
const char *server_introspection_xml =
	DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
	"<node>\n"

	"  <interface name='org.freedesktop.DBus.Introspectable'>\n"
	"    <method name='Introspect'>\n"
	"      <arg name='data' type='s' direction='out' />\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='org.freedesktop.DBus.Properties'>\n"
	"    <method name='Get'>\n"
	"      <arg name='interface' type='s' direction='in' />\n"
	"      <arg name='property'  type='s' direction='in' />\n"
	"      <arg name='value'     type='s' direction='out' />\n"
	"    </method>\n"
	"    <method name='GetAll'>\n"
	"      <arg name='interface'  type='s'     direction='in'/>\n"
	"      <arg name='properties' type='a{sv}' direction='out'/>\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='org.wmaker.WMInterface'>\n"
	"    <property name='Version' type='s' access='read' />\n"
	"    <property name='ActiveWorkspace' type='s' access='read' />\n"
	"    <property name='ActiveWindow' type='s' access='read' />\n"
	"    <method name='Ping' >\n"
	"      <arg type='s' direction='out' />\n"
	"    </method>\n"
	"    <method name='GetWindows'>\n"
	"      <arg name='workspace' direction='in' type='s'/>\n"
	"      <arg type='a(sss)' direction='out' />\n"
	"    </method>\n"
  "    <method name='ActivateWindow'>\n"
	"      <arg name='window' direction='in' type='s'/>\n"
	"      <arg type='s' direction='out' />\n"
	"    </method>\n"
  "    <method name='ActivateWorkspace'>\n"
	"      <arg name='workspace' direction='in' type='s'/>\n"
	"      <arg type='s' direction='out' />\n"
	"    </method>\n"
	"    <method name='Quit'>\n"
	"    </method>\n"
	"    <signal name='OnWindowActivate'>\n"
	"      <arg type='s' direction='out' />\n"
	"    </signal>\n"
	"    <signal name='OnWorkspaceActivate'>\n"
	"      <arg type='s' direction='out' />\n"
	"    </signal>\n"
	"  </interface>\n"

	"</node>\n";

/*
 * This implements 'Get' method of DBUS_INTERFACE_PROPERTIES so a
 * client can inspect the properties/attributes of 'WMInterface'.
 */
DBusHandlerResult server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply)
{
	char ba[300];
	char bb[100];
	char* win = xGetActiveWindow(ba);
	char* work = xGetActiveWorkspace(bb);

	if (!strcmp(property, "Version")) {
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &version,
					 DBUS_TYPE_INVALID);
	} else if (!strcmp(property, "ActiveWorkspace")) {
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &work,
					 DBUS_TYPE_INVALID);
	} else if (!strcmp(property, "ActiveWindow")) {
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &win,
					 DBUS_TYPE_INVALID);
	} else
		/* Unknown property */
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (!dbus_connection_send(conn, reply, NULL))
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	g_main_context_iteration(maincontext, FALSE);
	return DBUS_HANDLER_RESULT_HANDLED;
}

/*
 * This implements 'GetAll' method of DBUS_INTERFACE_PROPERTIES. This
 * one seems required by g_dbus_proxy_get_cached_property().
 */
DBusHandlerResult server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply)
{
	DBusHandlerResult result;
	DBusMessageIter array, dict, iter, variant;
	const char *property = "Version";

	char ba[300];
	char bb[100];
	char* win = xGetActiveWindow(ba);
	char* work = xGetActiveWorkspace(bb);

	/*
	 * All dbus functions used below might fail due to out of
	 * memory error. If one of them fails, we assume that all
	 * following functions will fail too, including
	 * dbus_connection_send().
	 */
	result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array);

	/* Append all properties name/value pairs */
	property = "Version";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);

	property = "ActiveWorkspace";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &work);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);

	property = "ActiveWindow";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &win);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);


	dbus_message_iter_close_container(&iter, &array);

	if (dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_HANDLED;

	g_main_context_iteration(maincontext, FALSE);
	return result;
}

void xFillWindowsReply(int workspace, DBusMessage *reply)
{
	DBusMessageIter array, dict, iter;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(sss)", &array);

	WScreen *scr = wScreenWithNumber(0);
	WWindow *wwin = scr->focused_window;

	char *buff = malloc(1024);

	while (wwin) {
		if (wwin->flags.internal_window || \
				wwin->flags.hidden || \
				!wwin->frame || \
				WFLAGP(wwin, skip_window_list) || \
				IS_GNUSTEP_POPUP_MENU(wwin) || \
				IS_GNUSTEP_MENU(wwin)) {
			continue;
		}
		if (workspace > 0 && wwin->frame->workspace != (workspace-1)) {
			continue;
		}

		dbus_message_iter_open_container(&array, DBUS_TYPE_STRUCT, NULL, &dict);

		int ws = wwin->frame->workspace + 1;
		sprintf(buff, "%ld/%d/%s/%s", wwin->client_win, ws, wwin->wm_class, wwin->wm_instance);
		dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &buff);

		sprintf(buff, "%s", wwin->frame->title);
		dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &buff);

		if (wwin->flags.hidden) {
			strcpy(buff, "H");
		} else if (wwin->flags.miniaturized) {
			strcpy(buff, "M");
		} else if (wwin->flags.focused) {
			strcpy(buff, "F");
		} else if (wwin->flags.shaded) {
			strcpy(buff, "S");
		} else {
			strcpy(buff, "W");
		}
		dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &buff);

		dbus_message_iter_close_container(&array, &dict);

		wwin = wwin->prev;
	}

	free(buff);

	dbus_message_iter_close_container(&iter, &array);
}

int send_event_signal(char* name, char* value)
{
  DBusMessage *reply = NULL;
	if (!(reply = dbus_message_new_signal("/org/wmaker/WMDesktop",
								"org.wmaker.WMInterface",
								name)))
		return 0;

	dbus_message_append_args(reply,
		DBUS_TYPE_STRING, &value,
		DBUS_TYPE_INVALID);

	if (!dbus_connection_send(mainconn, reply, NULL))
		return 0;

	dbus_run_loop();
	return 1;
}

/*
 * This function implements the 'WMInterface' interface for the
 * 'Server' DBus object.
 *
 * It also implements 'Introspect' method of
 * 'org.freedesktop.DBus.Introspectable' interface which returns the
 * XML string describing the interfaces, methods, and signals
 * implemented by 'Server' object. This also can be used by tools such
 * as d-feet(1) and can be queried by:
 *
 * $ gdbus introspect --session --dest org.wmaker.WMServer --object-path /org/example/WMDesktop
 */
DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusHandlerResult result;
  DBusMessage *reply = NULL;
	DBusError err;
	bool quit = false;

	fprintf(stderr, "Got D-Bus request: %s.%s on %s\n",
		dbus_message_get_interface(message),
		dbus_message_get_member(message),
		dbus_message_get_path(message));

	/*
	 * Does not allocate any memory; the error only needs to be
	 * freed if it is set at some point.
	 */
	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &server_introspection_xml,
					 DBUS_TYPE_INVALID);

	}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {
		const char *interface, *property;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &interface,
					   DBUS_TYPE_STRING, &property,
					   DBUS_TYPE_INVALID))
			goto fail;

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		result = server_get_properties_handler(property, conn, reply);
		dbus_message_unref(reply);
		return result;

	}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		result = server_get_all_properties_handler(conn, reply);

		dbus_message_unref(reply);
		return result;

	}  else if (dbus_message_is_method_call(message, "org.wmaker.WMInterface", "Ping")) {
		const char *pong = "Pong";

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &pong,
					 DBUS_TYPE_INVALID);

	}  else if (dbus_message_is_method_call(message, "org.wmaker.WMInterface", "ActivateWorkspace")) {
		const char *msg;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &msg,
					   DBUS_TYPE_INVALID))
			goto fail;

		int num = 0;
		sscanf(msg, "%d", &num);

		if (num > 0) {
			WScreen *scr = wScreenWithNumber(0);
			wWorkspaceChange(scr, num-1);
		}

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &msg,
					 DBUS_TYPE_INVALID);

	}  else if (dbus_message_is_method_call(message, "org.wmaker.WMInterface", "ActivateWindow")) {
		const char *msg;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &msg,
					   DBUS_TYPE_INVALID))
			goto fail;

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		Window winid = 0;
		sscanf(msg, "%lu", &winid);

		WWindow *wwin = wWindowFor(winid);
		if (wwin) {
			wWindowSingleFocus(wwin);
		}

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &msg,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.wmaker.WMInterface", "GetWindows")) {
		const char *msg;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &msg,
					   DBUS_TYPE_INVALID))
			goto fail;

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		int workspace = 0;
		sscanf(msg, "%d", &workspace);

		xFillWindowsReply(workspace, reply);

	} else if (dbus_message_is_method_call(message, "org.wmaker.WMInterface", "Quit")) {
		/*
		 * Quit() has no return values but a METHOD_RETURN
		 * reply is required, so the caller will know the
		 * method was successfully processed.
		 */
		reply = dbus_message_new_method_return(message);
		quit  = true;

	} else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

fail:
	if (dbus_error_is_set(&err)) {
		if (reply)
			dbus_message_unref(reply);
		reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	/*
	 * In any cases we should have allocated a reply otherwise it
	 * means that we failed to allocate one.
	 */
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	/* Send the reply which might be an error one too. */
	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, reply, NULL)) {
		fprintf(stderr, "failed\n");
		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	g_main_context_iteration(maincontext, FALSE);
	dbus_message_unref(reply);

	return result;
}


const DBusObjectPathVTable server_vtable = {
	.message_function = server_message_handler
};

void dbus_run_loop(void)
{
	if (!maincontext) return;

	char ba[300];
	xGetActiveWindow(ba);

	if (last_active_windowid[0] == '\0') {
		strcpy(last_active_windowid, ba);
	}
	else if (strcmp(ba, last_active_windowid)) {
		strcpy(last_active_windowid, ba);
		last_active_windowid_c = 0;
	}
	else {
		last_active_windowid_c++;
		if (last_active_windowid_c == 6) {
			send_event_signal("OnWindowActivate", last_active_windowid);
		}
	}

	g_main_context_iteration(maincontext, FALSE);
}

int dbus_init(void)
{
	DBusError err;
	int rv;

	last_active_windowid[0] = '\0';

  dbus_error_init(&err);

	/* connect to the daemon bus */
	mainconn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!mainconn) {
		fprintf(stderr, "Failed to get a session DBus connection: %s\n", err.message);
		goto fail;
	}

	rv = dbus_bus_request_name(mainconn, "org.wmaker.WMServer", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
		goto fail;
	}

	if (!dbus_connection_register_object_path(mainconn, "/org/wmaker/WMDesktop", &server_vtable, NULL)) {
		fprintf(stderr, "Failed to register a object path for 'WMDesktop'\n");
		goto fail;
	}

	maincontext = g_main_context_new();

	/* Set up the DBus connection to work in a GLib event loop */
	dbus_connection_setup_with_g_main(mainconn, maincontext);

	fprintf(stderr, "WM started dbus server v%s\n", version);

	return EXIT_SUCCESS;
fail:
	maincontext = NULL;
	dbus_error_free(&err);
	return EXIT_FAILURE;
}


