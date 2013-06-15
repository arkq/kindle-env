/*
 * ktterm - keyboard.c
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <json/json.h>

#include "keyboard.h"
#include "ktutils.h"


/* Load keyboard configuration (keys and corresponding modes) from the JSON
 * configuration file. Upon success this function returns TRUE, otherwise
 * FALSE is returned. */
static gboolean kb_load_keys(ktkb_keyboard *kb, const char *fname) {

	json_object *configuration;
	json_object *array, *tmp1, *tmp2;
	const char *sequence;
	int i, ii;

	if ((configuration = json_object_from_file(fname)) == NULL)
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

	for (i = 0; i < key->length; i++)
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
	gint i;

	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_TAP)
		for (i = 0; i < kb->flags_size; i++)
			if (kb_is_key_match(kb->flags[i], event->x, event->y)) {
				kb_process_flag(kb, &kb->flags[i], FALSE);
				return FALSE;
			}

	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_HOLD)
		for (i = 0; i < kb->flags_size; i++)
			if (kb_is_key_match(kb->flags[i], event->x, event->y)) {
				kb_process_flag(kb, &kb->flags[i], TRUE);
				return FALSE;
			}

	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_TAP)
		for (i = 0; i < kb->keys_size; i++)
			if (kb_is_key_match(kb->keys[i], event->x, event->y)) {
				kb_process_key(kb, &kb->keys[i]);
				return FALSE;
			}

  return FALSE;
}

/* Initialize embedded keyboard widget using given image and configuration.
 * When keyboard is no longer required, the resources used by this widget
 * should be released by the call to the embedded_kb_free() function. */
ktkb_keyboard *embedded_kb_new(GtkWidget *kb_box, VteTerminal *terminal,
		const char *image, const char *configuration) {

	GtkWidget *kb_image;
	ktkb_keyboard *kb;

	if ((kb = (ktkb_keyboard *)calloc(1, sizeof(*kb))) == NULL)
		return NULL;

	kb->terminal = terminal;
	if (!kb_load_keys(kb, configuration)) {
		free(kb);
		return NULL;
	}

	kb_image = gtk_image_new_from_file(image);
	gtk_container_add((GtkContainer *)(kb_box), kb_image);
	g_signal_connect(kb_box, "button-press-event", G_CALLBACK(kb_press_event), kb);

	return kb;
}

void embedded_kb_free(ktkb_keyboard *kb) {

	int i, ii;

	if (kb == NULL)
		return;

	for (i = 0; i < kb->keys_size; i++) {
		for (ii = 0; ii < kb->keys[i].length; ii++)
			free(kb->keys[i].modes[ii].sequence);
		free(kb->keys[i].modes);
	}

	free(kb->keys);
	free(kb->flags);
	free(kb);
}

/* Set user-defined callback function. This function will be called for every
 * key-press action except flag keys. If the callback function returns TRUE,
 * the standard key press is not forwarded to the terminal. */
void embedded_kb_set_key_callback(ktkb_keyboard *kb, ktkb_key_event callback, void *data) {
	kb->callback = callback;
	kb->callback_data = data;
}
