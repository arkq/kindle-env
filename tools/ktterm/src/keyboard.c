/*
 * ktterm - keyboard.c
 * Copyright (c) 2013-2016 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include "keyboard.h"

#include <stdlib.h>
#include <string.h>
#include <json/json.h>

#include "ktutils.h"


/* Load keyboard configuration (keys and corresponding modes) from the JSON
 * configuration file. Upon success this function returns TRUE, otherwise
 * FALSE is returned. */
static gboolean kb_load_keys(ktkb_keyboard *kb, const char *fname) {

	json_object *configuration;
	json_object *array, *tmp1, *tmp2;
	const char *sequence;
	int i, ii;

	if G_UNLIKELY ((configuration = json_object_from_file(fname)) == NULL)
		return FALSE;

	if (json_object_object_get_ex(configuration, "keys", &array)) {
		kb->keys_size = json_object_array_length(array);
		kb->keys = (ktkb_key *)malloc(kb->keys_size * sizeof(ktkb_key));
		for (i = 0; i < kb->keys_size; i++) {
			tmp1 = json_object_array_get_idx(array, i);

			kb->keys[i].x = json_object_get_int(json_object_array_get_idx(tmp1, 0));
			kb->keys[i].y = json_object_get_int(json_object_array_get_idx(tmp1, 1));
			kb->keys[i].width = json_object_get_int(json_object_array_get_idx(tmp1, 2));
			kb->keys[i].height = json_object_get_int(json_object_array_get_idx(tmp1, 3));

			kb->keys[i].length = json_object_array_length(tmp1) - 4;
			kb->keys[i].modes = (ktkb_key_mode *)malloc(kb->keys[i].length * sizeof(ktkb_key_mode));
			for (ii = 0; ii < kb->keys[i].length; ii++) {
				tmp2 = json_object_array_get_idx(tmp1, ii + 4);

				kb->keys[i].modes[ii].flag = json_object_get_int(json_object_array_get_idx(tmp2, 0));
				sequence = json_object_get_string(json_object_array_get_idx(tmp2, 1));

				kb->keys[i].modes[ii].length = strlen(sequence);
				kb->keys[i].modes[ii].sequence = (char *)malloc(kb->keys[i].modes[ii].length);
				memcpy(kb->keys[i].modes[ii].sequence, sequence, kb->keys[i].modes[ii].length);
			}
		}
	}

	if (json_object_object_get_ex(configuration, "flags", &array)) {
		kb->flags_size = json_object_array_length(array);
		kb->flags = (ktkb_flag *)malloc(kb->flags_size * sizeof(ktkb_flag));
		for (i = 0; i < kb->flags_size; i++) {
			tmp1 = json_object_array_get_idx(array, i);

			kb->flags[i].x = json_object_get_int(json_object_array_get_idx(tmp1, 0));
			kb->flags[i].y = json_object_get_int(json_object_array_get_idx(tmp1, 1));
			kb->flags[i].width = json_object_get_int(json_object_array_get_idx(tmp1, 2));
			kb->flags[i].height = json_object_get_int(json_object_array_get_idx(tmp1, 3));
			kb->flags[i].value = json_object_get_int(json_object_array_get_idx(tmp1, 4));
		}
	}

	json_object_put(configuration);
	return TRUE;
}

/* Process action for given key. */
static void kb_process_key(ktkb_keyboard *kb, ktkb_key *key) {

	ktkb_key_mode *key_mode;
	int i, flags;

	flags = kb->static_flags | kb->dynamic_flags;
	key_mode = &key->modes[0];
	kb->dynamic_flags = 0;

	for (i = key->length - 1; i >= 0; i--)
		if (key->modes[i].flag == flags) {
			key_mode = &key->modes[i];
			break;
		}

	/* run user-defined key event callback */
	if (kb->callback != NULL)
		if (kb->callback(key_mode, kb->callback_data) == TRUE)
			return;

	vte_terminal_feed_child_binary(kb->terminal, key_mode->sequence, key_mode->length);
}

/* Process action for given flag. If the lock parameter is TRUE, then the new
 * state of the flag is saved - persistent state. */
static void kb_process_flag(ktkb_keyboard *kb, ktkb_flag *flag, gboolean lock) {
	if (lock)
		kb->static_flags |= flag->value;
	else {
		kb->static_flags &= !flag->value;
		kb->dynamic_flags ^= flag->value;
	}
}

#define kb_is_key_match(_key, _x, _y) \
	(_x >= _key.x && _x <= _key.x + _key.width && _y >= _key.y && _y <= _key.y + _key.height)

static gboolean kb_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
	(void)widget;

	ktkb_keyboard *kb = (ktkb_keyboard *)data;
	gint x = event->x, y = event->y;
	gint src_w, src_h;
	gint i;

	/* this callback might get double-click or event triple-click events */
	if G_UNLIKELY (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	/* scale button press event according to the internal scale */
	src_w = gdk_pixbuf_get_width(kb->kb_pixbuf);
	src_h = gdk_pixbuf_get_height(kb->kb_pixbuf);
	x = (x * src_w) / kb->width;
	y = (y * src_h) / kb->height;

	for (i = kb->flags_size - 1; i >= 0; i--)
		if (kb_is_key_match(kb->flags[i], x, y)) {
			kb_process_flag(kb, &kb->flags[i], event->button == KT_TOUCH_HOLD);
			return TRUE;
		}

	if (event->button == KT_TOUCH_TAP)
		for (i = kb->keys_size - 1; i >= 0; i--)
			if (kb_is_key_match(kb->keys[i], x, y)) {
				kb_process_key(kb, &kb->keys[i]);
				return TRUE;
			}

  return FALSE;
}

/* Initialize embedded keyboard widget using given image and configuration.
 * When keyboard is no longer required, the resources used by this widget
 * should be released by the call to the embedded_kb_free() function. */
ktkb_keyboard *embedded_kb_new(GtkWidget *kb_box, VteTerminal *terminal,
		const char *image, const char *configuration) {

	ktkb_keyboard *kb;

	if G_UNLIKELY ((kb = (ktkb_keyboard *)calloc(1, sizeof(*kb))) == NULL)
		return NULL;

	kb->terminal = terminal;

	if G_UNLIKELY (!kb_load_keys(kb, configuration)) {
		free(kb);
		return NULL;
	}

	kb->kb_pixbuf = gdk_pixbuf_new_from_file(image, NULL);
	if G_UNLIKELY (kb->kb_pixbuf == NULL) {
		free(kb);
		return NULL;
	}

	kb->kb_image = gtk_image_new_from_pixbuf(kb->kb_pixbuf);
	gtk_container_add(GTK_CONTAINER(kb_box), kb->kb_image);

	g_signal_connect(kb_box, "button-press-event", G_CALLBACK(kb_press_event), kb);
	return kb;
}

void embedded_kb_free(ktkb_keyboard *kb) {

	int i, ii;

	if G_UNLIKELY (kb == NULL)
		return;

	gdk_pixbuf_unref(kb->kb_pixbuf);

	for (i = 0; i < kb->keys_size; i++) {
		for (ii = 0; ii < kb->keys[i].length; ii++)
			free(kb->keys[i].modes[ii].sequence);
		free(kb->keys[i].modes);
	}

	free(kb->keys);
	free(kb->flags);
	free(kb);
}

/* Scale embedded keyboard widget according to the given width and height. If
 * one (and only one) of this parameters is set to -1, then widget is scaled
 * with the aspect ratio preserved. If both parameters are set to -1, then the
 * widget size is set to the size of provided keyboard image. */
void embedded_kb_scale(ktkb_keyboard *kb, int width, int height) {

	GdkPixbuf *tmp;
	int src_w, src_h;

	src_w = gdk_pixbuf_get_width(kb->kb_pixbuf);
	src_h = gdk_pixbuf_get_height(kb->kb_pixbuf);

	if (width == -1 && height == -1) {
		width = src_w;
		height = src_h;
	}
	else if (width == -1)
		width = (src_w * height) / src_h;
	else if (height == -1)
		height = (src_h * width) / src_w;

	/* Skip resizing the keyboard image if the requested size has not been
	 * changed since the last call - resize operation is very expensive. */
	if (kb->width == width && kb->height == height)
		return;

	kb->width = width;
	kb->height = height;
	tmp = gdk_pixbuf_scale_simple(kb->kb_pixbuf, width, height, GDK_INTERP_HYPER);
	if G_UNLIKELY (tmp == NULL)
		return;

	gtk_image_set_from_pixbuf(GTK_IMAGE(kb->kb_image), tmp);
	gdk_pixbuf_unref(tmp);
}

/* Set user-defined callback function. This function will be called for every
 * key-press action except flag keys. If the callback function returns TRUE,
 * the standard key press is not forwarded to the terminal. */
void embedded_kb_set_key_callback(ktkb_keyboard *kb, ktkb_key_event callback, void *data) {
	kb->callback = callback;
	kb->callback_data = data;
}
