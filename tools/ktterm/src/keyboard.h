/*
 * ktterm - keyboard.h
 * Copyright (c) 2013-2016 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#ifndef KTTERM_KEYBOARD_
#define KTTERM_KEYBOARD_

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <vte/vte.h>


/* Structure which holds a key sequence. For standard characters the sequence
 * is a simple ASCII code - one byte. */
typedef struct embedded_kb_key_mode {
	char *sequence;
	char flag, length;
} ktkb_key_mode;

/* Physical key representation on the keyboard. Every key within a keyboard
 * is placed at the specified (X, Y) coordinates with the given width and
 * height. Also, every key can hold multiple number of modes - keyboard flag
 * (modifier) can alternate key sequence. */
typedef struct embedded_kb_key {
	int x, y, width, height;
	ktkb_key_mode *modes;
	char length;
} ktkb_key;

/* Special key, which acts as a keyboard flag (aka modifier). */
typedef struct embedded_kb_flag {
	int x, y, width, height;
	char value;
} ktkb_flag;

/* User-defined callback function for key processing. */
typedef gboolean (*ktkb_key_event)(ktkb_key_mode *key, void *data);

typedef struct embedded_kb {

	ktkb_key *keys;
	ktkb_flag *flags;
	int keys_size;
	int flags_size;

	char static_flags;
	char dynamic_flags;
	VteTerminal *terminal;

	GdkPixbuf *kb_pixbuf;
	GtkWidget *kb_image;
	int width, height;

	ktkb_key_event callback;
	void *callback_data;

} ktkb_keyboard;


ktkb_keyboard *embedded_kb_new(GtkWidget *kb_box, VteTerminal *terminal,
		const char *image, const char *configuration);
void embedded_kb_free(ktkb_keyboard *kb);

void embedded_kb_scale(ktkb_keyboard *kb, int width, int height);
void embedded_kb_set_key_callback(ktkb_keyboard *kb, ktkb_key_event callback, void *data);

#endif  /* KTTERM_KEYBOARD_ */
