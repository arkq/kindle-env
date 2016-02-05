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


/* Set window placement withing the Kindle Touch application framework. The
 * window parameter should be a top-level one. */
void kt_window_set_placement(GtkWindow *window, KTWindowPlacement placement, const char *app) {

	gchar *tmp;

	switch (placement) {
	default:
	case KT_WINDOW_PLACEMENT_APPLICATION:
		tmp = g_strdup_printf("L:A_N:application_PC:TS_ID:%s", app);
		break;
	case KT_WINDOW_PLACEMENT_MAXIMIZED:
		tmp = g_strdup_printf("L:A_N:application_ID:%s", app);
		break;
	case KT_WINDOW_PLACEMENT_FULLSCREEN:
		tmp = g_strdup_printf("L:A_N:application_PC:N_ID:%s", app);
		break;
	}

	gtk_window_set_title(window, tmp);
	g_free(tmp);
}

/* Set terminal color in the standard fashion (black font on white background)
 * or reversed one. Other color combinations are not required, because Kindle
 * Touch has an E-Ink display which supports gray scale only. */
void kt_terminal_set_colors(VteTerminal *terminal, gboolean reversed) {

	GdkColor color_white = { 0, 0xffff, 0xffff, 0xffff };
	GdkColor color_black = { 0, 0x0000, 0x0000, 0x0000 };

	if (reversed)
		vte_terminal_set_colors(terminal, &color_white, &color_black, NULL, 0);
	else
		vte_terminal_set_colors(terminal, &color_black, &color_white, NULL, 0);
}

/* Get current font size. */
gint kt_terminal_get_font_size(VteTerminal *terminal) {

	PangoFontDescription *font;
	gint size;

	font = (PangoFontDescription *)vte_terminal_get_font(terminal);
	size = pango_font_description_get_size(font);

	if (!pango_font_description_get_size_is_absolute(font))
		size /= PANGO_SCALE;

	return size;
}

/* Set terminal fort size. */
void kt_terminal_set_font_size(VteTerminal *terminal, gint size) {

	PangoFontDescription *font;

	font = (PangoFontDescription *)vte_terminal_get_font(terminal);
	pango_font_description_set_size(font, size * PANGO_SCALE);
	vte_terminal_set_font(terminal, font);
}
