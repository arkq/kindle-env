/*
 * ktterm - keyboard.h
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include <vte/vte.h>


typedef struct embedded_kb_key_mode_tag {
	char *sequence;
	char flag, length;
} ktkb_key_mode_t;

typedef struct embedded_kb_key_tag {
	int x, y, width, height;
	ktkb_key_mode_t *modes;
	char length;
} ktkb_key_t;

typedef struct embedded_kb_flag_tag {
	int x, y, width, height;
	char value;
} ktkb_flag_t;

typedef gboolean (*ktkb_key_event_t)(ktkb_key_mode_t *key, void *data);

typedef struct embedded_kb_tag {
	ktkb_key_t *keys;
	ktkb_flag_t *flags;
	int keys_size, flags_size;

	char static_flags;
	char dynamic_flags;
	VteTerminal *terminal;

	ktkb_key_event_t callback;
	void *callback_data;
} ktkb_keyboard_t;


ktkb_keyboard_t *embedded_kb_new(GtkWidget *kb_box, VteTerminal *terminal, const char *image_fname, const char *keys_fname);
void embedded_kb_free(ktkb_keyboard_t *kb);
gboolean embedded_kb_load_keys(ktkb_keyboard_t *kb, const char *fname);
gboolean embedded_kb_events(GtkWidget *widget, GdkEventButton *event, gpointer data);
void embedded_kb_action_flag(ktkb_keyboard_t *kb, ktkb_flag_t *flag, gboolean lock);
void embedded_kb_action_key(ktkb_keyboard_t *kb, ktkb_key_t *key);
void embedded_kb_set_key_callback(ktkb_keyboard_t *kb, ktkb_key_event_t callback, void *data);
