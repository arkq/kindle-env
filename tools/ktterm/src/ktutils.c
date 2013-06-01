/*
 * ktterm - ktutils.c
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include "ktutils.h"


void ktterm_window_set_placement(GtkWindow *window, KTWindowPlacement placement) {
	switch (placement) {
	case KT_WINDOW_PLACEMENT_APPLICATION:
		gtk_window_set_title(window, "L:A_N:application_PC:TS_ID:" APP_NAME);
		break;
	case KT_WINDOW_PLACEMENT_MAXIMIZED:
		gtk_window_set_title(window, "L:A_N:application_ID:" APP_NAME);
		break;
	case KT_WINDOW_PLACEMENT_FULLSCREEN:
		gtk_window_set_title(window, "L:A_N:application_PC:N_ID:" APP_NAME);
		break;
	}
}

void ktterm_terminal_set_colors(VteTerminal *terminal, gboolean reversed) {
	GdkColor color_white = { 0, 0xffff, 0xffff, 0xffff };
	GdkColor color_black = { 0, 0x0000, 0x0000, 0x0000 };

	if (reversed)
		vte_terminal_set_colors(terminal, &color_black, &color_white, NULL, 0);
	else
		vte_terminal_set_colors(terminal, &color_white, &color_black, NULL, 0);
}

gint ktterm_terminal_get_font_size(VteTerminal *terminal) {
	PangoFontDescription *font;
	gint size;

	font = (PangoFontDescription *)vte_terminal_get_font(terminal);
	size = pango_font_description_get_size(font);

	if (!pango_font_description_get_size_is_absolute(font))
		size /= PANGO_SCALE;

	return size;
}

void ktterm_terminal_set_font_size(VteTerminal *terminal, gint size) {
	PangoFontDescription *font;

	font = (PangoFontDescription *)vte_terminal_get_font(terminal);
	pango_font_description_set_size(font, size * PANGO_SCALE);
	vte_terminal_set_font(terminal, font);
}
