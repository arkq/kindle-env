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

#include <jansson.h>

#include "keyboard.h"
#include "ktterm.h"


#define embedded_kb_is_key_match(_key, _x, _y) \
	(_x >= _key.x && _x <= _key.x + _key.width && _y >= _key.y && _y <= _key.y + _key.height)

ktkb_keyboard_t *embedded_kb_new(GtkWidget *kb_box, VteTerminal *terminal, const char *image_fname, const char *keys_fname) {

	GtkWidget *kb_image;
	ktkb_keyboard_t *kb;

	kb = (ktkb_keyboard_t *)malloc(sizeof(ktkb_keyboard_t));
	memset(kb, 0, sizeof(ktkb_keyboard_t));

	kb->terminal = terminal;
	embedded_kb_load_keys(kb, keys_fname);

	kb_image = gtk_image_new_from_file(image_fname);
	gtk_container_add((GtkContainer *)(kb_box), kb_image);
	g_signal_connect(kb_box, "button-press-event", G_CALLBACK(embedded_kb_events), kb);

	return kb;
}

void embedded_kb_free(ktkb_keyboard_t *kb) {

	int i, ii;

	for (i = 0; i < kb->keys_size; i++) {
		for (ii = 0; ii < kb->keys[i].length; ii++)
			free(kb->keys[i].modes[ii].sequence);
		free(kb->keys[i].modes);
	}

	free(kb->keys);
	free(kb->flags);
	free(kb);
}

gboolean embedded_kb_load_keys(ktkb_keyboard_t *kb, const char *fname) {

	json_t *json, *json_flags, *json_keys;
	json_t *json_flag, *json_key, *json_key_mode;
	const char *sequence;
	int i, ii;

	if ((json = json_load_file(fname, 0, NULL)) == NULL)
		return FALSE;

	if (json_is_object(json)) {

		json_flags = json_object_get(json, "flags");
		kb->flags_size = json_array_size(json_flags);
		kb->flags = (ktkb_flag_t *)malloc(kb->flags_size * sizeof(ktkb_flag_t));
		for (i = 0; i < kb->flags_size; i++) {
			json_flag = json_array_get(json_flags, i);

			kb->flags[i].x = (int)json_integer_value(json_array_get(json_flag, 0));
			kb->flags[i].y = (int)json_integer_value(json_array_get(json_flag, 1));
			kb->flags[i].width = (int)json_integer_value(json_array_get(json_flag, 2));
			kb->flags[i].height = (int)json_integer_value(json_array_get(json_flag, 3));
			kb->flags[i].value = (char)json_integer_value(json_array_get(json_flag, 4));
		}

		json_keys = json_object_get(json, "keys");
		kb->keys_size = json_array_size(json_keys);
		kb->keys = (ktkb_key_t *)malloc(kb->keys_size * sizeof(ktkb_key_t));
		for (i = 0; i < kb->keys_size; i++) {
			json_key = json_array_get(json_keys, i);

			kb->keys[i].x = (int)json_integer_value(json_array_get(json_key, 0));
			kb->keys[i].y = (int)json_integer_value(json_array_get(json_key, 1));
			kb->keys[i].width = (int)json_integer_value(json_array_get(json_key, 2));
			kb->keys[i].height = (int)json_integer_value(json_array_get(json_key, 3));

			kb->keys[i].length = json_array_size(json_key) - 4;
			kb->keys[i].modes = (ktkb_key_mode_t *)malloc(kb->keys[i].length * sizeof(ktkb_key_mode_t));
			for (ii = 0; ii < kb->keys[i].length; ii++) {
				json_key_mode = json_array_get(json_key, ii + 4);

				kb->keys[i].modes[ii].flag = (char)json_integer_value(json_array_get(json_key_mode, 0));
				sequence = json_string_value(json_array_get(json_key_mode, 1));

				kb->keys[i].modes[ii].length = strlen(sequence);
				kb->keys[i].modes[ii].sequence = (char *)malloc(kb->keys[i].modes[ii].length);
				memcpy(kb->keys[i].modes[ii].sequence, sequence, kb->keys[i].modes[ii].length);
			}
		}
	}

	json_decref(json);
	return TRUE;
}

gboolean embedded_kb_events(GtkWidget *widget, GdkEventButton *event, gpointer data) {

	ktkb_keyboard_t *kb = (ktkb_keyboard_t *)data;
	gint i;

	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_TAP)
		for (i = 0; i < kb->flags_size; i++)
			if (embedded_kb_is_key_match(kb->flags[i], event->x, event->y)) {
				embedded_kb_action_flag(kb, &kb->flags[i], FALSE);
				return FALSE;
			}
	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_HOLD)
		for (i = 0; i < kb->flags_size; i++)
			if (embedded_kb_is_key_match(kb->flags[i], event->x, event->y)) {
				embedded_kb_action_flag(kb, &kb->flags[i], TRUE);
				return FALSE;
			}

	if (event->type == GDK_BUTTON_PRESS && event->button == KT_TOUCH_TAP)
		for (i = 0; i < kb->keys_size; i++)
			if (embedded_kb_is_key_match(kb->keys[i], event->x, event->y)) {
				embedded_kb_action_key(kb, &kb->keys[i]);
				return FALSE;
			}

  return FALSE;
}

void embedded_kb_action_flag(ktkb_keyboard_t *kb, ktkb_flag_t *flag, gboolean lock) {
	if (lock) {
		kb->static_flags |= flag->value;
	}
	else {
		kb->static_flags &= !flag->value;
		kb->dynamic_flags ^= flag->value;
	}
}

void embedded_kb_action_key(ktkb_keyboard_t *kb, ktkb_key_t *key) {

	ktkb_key_mode_t *key_mode;
	int i, flags;

	flags = kb->static_flags | kb->dynamic_flags;
	key_mode = &key->modes[0];
	kb->dynamic_flags = 0;

	for (i = 0; i < key->length; i++)
		if (key->modes[i].flag == flags) {
			key_mode = &key->modes[i];
			break;
		}

	/* run user's key event callback */
	if (kb->callback != NULL)
		if (kb->callback(key_mode, kb->callback_data) == TRUE)
			return;

	vte_terminal_feed_child_binary(kb->terminal, key_mode->sequence, key_mode->length);
}

void embedded_kb_set_key_callback(ktkb_keyboard_t *kb, ktkb_key_event_t callback, void *data) {
	kb->callback = callback;
	kb->callback_data = data;
}
