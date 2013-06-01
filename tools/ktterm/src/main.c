/*
 * ktterm - main.c
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include "ktterm.h"

#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "keyboard.h"
#include "ktutils.h"


#ifdef DEBUG
static gboolean show_debug = FALSE;
#endif

struct ktterm_config {
	gboolean show_keyboard;
	gboolean full_screen;
	gboolean kindle_keyboard;
} config = {
	TRUE,
	FALSE,
	FALSE,
};

GtkWidget *window = NULL;
GtkWidget *terminal = NULL;
GtkWidget *keyboard = NULL;


void ktterm_update_window_size() {
	GdkGeometry hints;

	hints.min_width = hints.max_width = 600;
	hints.min_height = hints.max_height = 800;

	/* height corrections for non-full-screen mode */
	if (!config.full_screen)
		hints.min_height = hints.max_height -= 30;

	/* correction for Kindle keyboard */
	if (config.kindle_keyboard && config.show_keyboard)
		hints.min_height = hints.max_height -= 275;

	gtk_window_set_geometry_hints((GtkWindow *)(window), window, &hints,
			(GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

void ktterm_update_keyboard() {
#ifdef LIBLIPC
#else
	pid_t lipc_pid;
	char lipc_set_prop_bin[] = "/usr/bin/lipc-set-prop";
	char lipc_action_open[] = "open";
	char lipc_action_close[] = "close";
	char lipc_action_data[32];
	char *v_argv[8] = {lipc_set_prop_bin, "-s", "com.lab126.keyboard", lipc_action_open, lipc_action_data};
#endif

#ifdef DEBUG
	if (show_debug)
		printf("show_keyboard: %d\n", config.show_keyboard);
#endif

	/* use external Kindle-on-board keyboard */
	if (config.kindle_keyboard) {

		/* show keyboard, and then shrink the terminal window */
		if (config.show_keyboard) {
#ifdef LIBLIPC
#else
			v_argv[3] = lipc_action_open;
			sprintf(lipc_action_data, "%s:abc:0", APP_NAME);
			posix_spawn(&lipc_pid, lipc_set_prop_bin, NULL, NULL, v_argv, NULL);
#endif

			ktterm_update_window_size();
		}
		/* enlarge the terminal window, and then hide keyboard */
		else {
			ktterm_update_window_size();

#ifdef LIBLIPC
#else
			v_argv[3] = lipc_action_close;
			sprintf(lipc_action_data, "%s", APP_NAME);
			posix_spawn(&lipc_pid, lipc_set_prop_bin, NULL, NULL, v_argv, NULL);
#endif
		}

	}
	/* use the embedded keyboard */
	else {
		if(config.show_keyboard)
			gtk_widget_show(keyboard);
		else
			gtk_widget_hide(keyboard);
	}
}

gboolean terminal_toggler(GtkWidget *widget, GdkEventButton *event, gpointer data) {
	GtkRequisition requisition;

#ifdef DEBUG
	if (show_debug)
		printf("bpt: %d %d (%f, %f)\n", event->type, event->button, event->x, event->y);
#endif

	/* process "hold" actions */
	if (event->button == KT_TOUCH_HOLD) {
		gtk_widget_size_request(widget, &requisition);

		/* keyboard toggling (upper-right corner) */
		if (event->x >= requisition.width * 0.90 && event->y <= requisition.height * 0.10) {
			config.show_keyboard ^= TRUE;
			ktterm_update_keyboard();
		}

		/* prevent terminal widget from processing event (just in case) */
		return TRUE;
	}

	return FALSE;
}

/* This callback function handles special sequences (prefixed with the "\uf1f1")
 * from the keyboard configuration file. When such a sequence is recognized, then
 * we are going to proceed it internally. */
gboolean ktterm_key_callback(ktkb_key_mode_t *key, void *data) {
	char prefix[3] = { 0xef, 0x87, 0xb1 }; /* decoded JSON "\uf1f1" */
	int special_value;
	int f_size;

	/* check special sequence length and prefix */
	if (!(key->length == 5 && memcmp(key->sequence, prefix, 3) == 0))
		return FALSE;

	/* XXX: convert last 2 bytes of special sequence into integer value */
	special_value = *((uint16_t *)&key->sequence[3]);

#define _SSV(a, b) (((b) << 8) | (a))
	switch (special_value) {
	case _SSV('F', '+'):
		f_size = ktterm_terminal_get_font_size((VteTerminal *)terminal);
		ktterm_terminal_set_font_size((VteTerminal *)terminal, f_size + 1);
		break;
	case _SSV('F', '-'):
		f_size = ktterm_terminal_get_font_size((VteTerminal *)terminal);
		ktterm_terminal_set_font_size((VteTerminal *)terminal, f_size - 1);
		break;
	case _SSV('F', '0'):
		ktterm_terminal_set_font_size((VteTerminal *)terminal, KT_VTE_FONT_SIZE);
		break;
	case _SSV('W', 'F'):
		config.full_screen ^= TRUE;
		if (config.full_screen)
			ktterm_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_FULLSCREEN);
		else
			ktterm_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_MAXIMIZED);
		ktterm_update_window_size();
		break;
	}

	return TRUE;
}

void print_usage() {
	printf("usage: %s [options]\n"
#ifdef DEBUG
			"  -d\tshow extra debug informations\n"
			"  -T\tset main window title\n"
#endif
			"  -K\tuse the Kindle keyboard\n"
			"  -R\tresources directory\n"
			"  -f\tset terminal font size\n"
			"  -r\tuse reversed color palette\n"
			"  -v\trun in the full-screen mode\n",
		APP_NAME);
}

int main(int argc, char *argv[]) {
	GtkWidget *vbox;
	int t_font_size = KT_VTE_FONT_SIZE;
	gboolean reversed_palette = FALSE;
	char *v_argv[16] = {KT_VTE_EXEC, NULL};
	ktkb_keyboard_t *embedded_kb = NULL;
	char resources_dir[128] = KT_SHARE;
	char kb_image_fname[128], kb_config_fname[128];
	int opt;

#ifdef DEBUG
	char w_title[64];
	memset(w_title, 0, sizeof(w_title));
#endif

	while ((opt = getopt(argc, argv, "hdT:Kf:vR:r")) != -1)
		switch (opt) {
		case 'h':
			print_usage();
			return EXIT_SUCCESS;
#ifdef DEBUG
		case 'd':
			show_debug = TRUE;
			break;
		case 'T':
			strncpy(w_title, optarg, sizeof(w_title) - 1);
			break;
#endif
		case 'K':
			config.kindle_keyboard = TRUE;
			break;
		case 'R':
			strncpy(resources_dir, optarg, sizeof(resources_dir) - 1);
			break;
		case 'f':
			t_font_size = atoi(optarg);
			break;
		case 'r':
			reversed_palette = TRUE;
			break;
		case 'v':
			config.full_screen = TRUE;
			break;
		default:
			printf("Try '%s -h' for more information.\n", APP_NAME);
			return EXIT_FAILURE;
		}

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	ktterm_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_MAXIMIZED);
	if (config.full_screen)
		ktterm_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_FULLSCREEN);

	gtk_window_set_resizable((GtkWindow *)window, FALSE);
	ktterm_update_window_size();

#ifdef DEBUG
	if (w_title[0] != 0)
		gtk_window_set_title((GtkWindow *)window, w_title);
#endif

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add((GtkContainer *)window, vbox);

	terminal = vte_terminal_new();
	gtk_box_pack_start((GtkBox *)vbox, terminal, TRUE, TRUE, 0);

	vte_terminal_set_cursor_blink_mode((VteTerminal *)terminal, VTE_CURSOR_BLINK_OFF);
	vte_terminal_set_cursor_shape((VteTerminal *)terminal, VTE_CURSOR_SHAPE_UNDERLINE);
	vte_terminal_set_scroll_on_output((VteTerminal *)terminal, TRUE);
	ktterm_terminal_set_colors((VteTerminal *)terminal, reversed_palette);
	ktterm_terminal_set_font_size((VteTerminal *)terminal, t_font_size);
	vte_terminal_fork_command_full((VteTerminal *)terminal, VTE_PTY_DEFAULT,
			NULL, v_argv, NULL, G_SPAWN_LEAVE_DESCRIPTORS_OPEN, NULL, NULL, NULL, NULL);

	keyboard = gtk_event_box_new();
	/* load resources for the embedded keyboard */
	if (!config.kindle_keyboard) {
		sprintf(kb_image_fname, "%s/"KT_KB_EMBEDDED_IMG, resources_dir);
		sprintf(kb_config_fname, "%s/"KT_KB_EMBEDDED_CFG, resources_dir);
		embedded_kb = embedded_kb_new(keyboard, (VteTerminal *)terminal, kb_image_fname, kb_config_fname);
		embedded_kb_set_key_callback(embedded_kb, ktterm_key_callback, NULL);
		gtk_box_pack_end((GtkBox *)vbox, keyboard, FALSE, FALSE, 0);
	}

	ktterm_update_keyboard();
	gtk_widget_show_all(window);

	gtk_widget_add_events(terminal, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(terminal, "motion-notify-event", G_CALLBACK(gtk_true), NULL);
	g_signal_connect(terminal, "button-press-event", G_CALLBACK(terminal_toggler), NULL);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(terminal, "child-exited", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	config.show_keyboard = FALSE;
	ktterm_update_keyboard();

	/* free embedded keyboard resources */
	if (!config.kindle_keyboard)
		embedded_kb_free(embedded_kb);

	return EXIT_SUCCESS;
}
