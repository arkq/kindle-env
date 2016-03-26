/*
 * [open]lipc - lipc-probe.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of openlipc.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 * This source code is based on the reverse-engineered application, which
 * is a part of the Kindle firmware. The original lipc-probe application is
 * copyrighted under the terms of the Amazon Technologies, Inc.
 *
 */

#include "openlipc.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>


#define LIPC_PROPERTY_MODE_R 0x01
#define LIPC_PROPERTY_MODE_W 0x02
#define LIPC_PROPERTY_TYPE_INT 1
#define LIPC_PROPERTY_TYPE_STR 2
#define LIPC_PROPERTY_TYPE_HAS 3

struct lipc_property {
	gchar *name;
	guchar mode;
	guchar type;
};


/* Get the list of all available sources. On success, this function returns
 * TRUE and sources (publishers) are populated into the sources argument. On
 * failure FALSE is returned and the sources argument is not modified. */
static gboolean get_sources(GSList **sources) {

	GDBusConnection *dbus;
	GDBusMessage *message;
	GDBusMessage *reply;
	GVariantIter *iter;
	GError *error;
	gchar *address;
	gchar *tmp;
	gboolean rv;

	/* Kindle uses glib-2.29 but g_type_init is deprecated since glib-2.36, so
	 * this call is pretty much required - omitting it will cause segfault. */
	g_type_init();

	message = reply = NULL;
	error = NULL;

	address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	dbus = g_dbus_connection_new_for_address_sync(address,
			G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
			G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
			NULL, NULL, &error);
	if (dbus == NULL) {
		fprintf(stderr, "error: failed to get DBus connection: %s\n", error->message);
		goto return_failure;
	}

	message = g_dbus_message_new_method_call("org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
	reply = g_dbus_connection_send_message_with_reply_sync(dbus, message,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &error);
	if (reply == NULL) {
		fprintf(stderr, "error: failed to get source list: %s\n", error->message);
		goto return_failure;
	}

	g_variant_get(g_dbus_message_get_body(reply), "(as)", &iter);
	while (g_variant_iter_loop(iter, "&s", &tmp)) {
		if (tmp[0] == ':' || g_ascii_strcasecmp(tmp, "org.freedesktop.DBus") == 0)
			continue;
		*sources = g_slist_prepend(*sources, g_strdup(tmp));
	}
	g_variant_iter_free(iter);

	rv = TRUE;
	goto return_success;

return_failure:
	g_error_free(error);
	rv = FALSE;

return_success:
	if (reply != NULL)
		g_object_unref(reply);
	if (message != NULL)
		g_object_unref(message);
	if (dbus != NULL)
		g_object_unref(dbus);
	g_free(address);
	return rv;
}

/* Get the list of all properties for given source. On success, this function
 * returns TRUE and properties are populated into the properties argument. On
 * failure FALSE is returned and the properties argument is not modified. The
 * returned property is in the format of LipcProperty structure which can be
 * freed with g_free() function. */
static gboolean get_properties(LIPC *lipc, const char *source, GSList **properties) {

	char *values;

	if (LipcGetProperties(lipc, source, &values) != LIPC_OK)
		return FALSE;

	struct lipc_property property;
	gchar **tokens;
	guint i, length;

	tokens = g_strsplit(values, " ", 0);
	length = g_strv_length(tokens) / 3;
	for (i = 0; i < length; i++) {

		property.name = g_strdup(tokens[i * 3]);

		property.type = LIPC_PROPERTY_TYPE_HAS;
		if (g_strcmp0(tokens[i * 3 + 1], "Int") == 0)
			property.type = LIPC_PROPERTY_TYPE_INT;
		if (g_strcmp0(tokens[i * 3 + 1], "Str") == 0)
			property.type = LIPC_PROPERTY_TYPE_STR;

		property.mode = 0;
		if (strchr(tokens[i * 3 + 2], 'r') != NULL)
			property.mode |= LIPC_PROPERTY_MODE_R;
		if (strchr(tokens[i * 3 + 2], 'w') != NULL)
			property.mode |= LIPC_PROPERTY_MODE_W;

		*properties = g_slist_prepend(*properties, g_memdup(&property, sizeof(property)));

	}

	g_strfreev(tokens);
	LipcFreeString(values);
	return TRUE;
}

static const gchar *get_property_mode_str(guint mode) {
	switch (mode) {
	case LIPC_PROPERTY_MODE_R:
		return "r";
	case LIPC_PROPERTY_MODE_W:
		return "w";
	default:
		return "rw";
	}
}

static const gchar *get_property_type_str(guint type) {
	switch (type) {
	case LIPC_PROPERTY_TYPE_INT:
		return "Int";
	case LIPC_PROPERTY_TYPE_STR:
		return "Str";
	default:
		return "Has";
	}
}

static gint property_name_cmp(gconstpointer a, gconstpointer b) {
	struct lipc_property *_a = (struct lipc_property *)a;
	struct lipc_property *_b = (struct lipc_property *)b;
	return g_ascii_strcasecmp(_a->name, _b->name);
}

int main(int argc, char *argv[]) {

	int opt;

	int list_all = 0;
	int value_probe = 1;
	int value_get = 0;

	while ((opt = getopt(argc, argv, "hlav")) != -1)
		switch (opt) {
		case 'h':
			printf("usage: %s [-lav] [<publisher>] [<publisher>] ...\n\n"
				"  publisher - the unique name of the publisher\n"
				"\n"
				"options:\n"
				"  -l\tlist all available services in the system\n"
				"  -a\tlist and probe all available services\n"
				"  -v\tshow value for all readable properties\n",
				argv[0]);
			return EXIT_SUCCESS;

		case 'l':
			list_all = 1;
			value_probe = 0;
			break;
		case 'a':
			list_all = 1;
			value_probe = 1;
			break;
		case 'v':
			value_get = 1;
			break;

		default:
return_usage:
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return EXIT_FAILURE;
		}

	GSList *sources = NULL;
	LIPC *lipc;

	if (list_all) {
		if (get_sources(&sources) == FALSE)
			return EXIT_FAILURE;
	}
	else {
		while (argc - optind)
			sources = g_slist_prepend(sources, g_strdup(argv[optind++]));
		if (sources == NULL)
			goto return_usage;
	}

	LipcSetLlog(LAB126_LOG_ALL & ~LAB126_LOG_DEBUG_ALL);
	if ((lipc = LipcOpenNoName()) == NULL) {
		fprintf(stderr, "error: failed to open lipc\n");
		return EXIT_FAILURE;
	}

	/* help reading the output by sorting input sources */
	sources = g_slist_sort(sources, (GCompareFunc)g_ascii_strcasecmp);

	GSList *tmp;
	while (sources) {
		const char *source = (char *)sources->data;

		printf("%s\n", source);
		if (value_probe) {

			GSList *properties = NULL;
			get_properties(lipc, source, &properties);

			/* help reading the output by sorting properties */
			properties = g_slist_sort(properties, property_name_cmp);

			GSList *tmp;
			while (properties) {
				struct lipc_property *property = (struct lipc_property *)properties->data;

				printf("\t%s\t%s\t%s", get_property_mode_str(property->mode),
						get_property_type_str(property->type), property->name);

				if (value_get && property->mode && LIPC_PROPERTY_MODE_R) {

					if (property->type == LIPC_PROPERTY_TYPE_INT) {
						int value;
						if (LipcGetIntProperty(lipc, source, property->name, &value) == LIPC_OK)
							printf("\t[%d]", value);
					}
					else if (property->type == LIPC_PROPERTY_TYPE_STR) {
						char *value;
						if (LipcGetStringProperty(lipc, source, property->name, &value) == LIPC_OK) {
							printf("\t[%s]", value);
							LipcFreeString(value);
						}
					}
					else
						printf("\t[*NOT SHOWN*]");

				}

				printf("\n");

				/* free resources and get next element */
				g_free(property->name);
				g_free(properties->data);
				tmp = g_slist_next(properties);
				g_slist_free_1(properties);
				properties = tmp;

			}

		}

		/* free resources and get next element */
		g_free(sources->data);
		tmp = g_slist_next(sources);
		g_slist_free_1(sources);
		sources = tmp;

	}

	LipcClose(lipc);
	return EXIT_SUCCESS;
}
