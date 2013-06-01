/*
 * ktterm - ktutils.h
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include <vte/vte.h>

#include "ktterm.h"


void ktterm_window_set_placement(GtkWindow *window, KTWindowPlacement placement);
void ktterm_terminal_set_colors(VteTerminal *terminal, gboolean reversed);
gint ktterm_terminal_get_font_size(VteTerminal *terminal);
void ktterm_terminal_set_font_size(VteTerminal *terminal, gint size);
