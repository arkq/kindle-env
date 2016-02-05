/*
 * ktterm - main.c
 * Copyright (c) 2013-2016 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#if HAVE_CONFIG_H
#include "../config.h"
#endif

#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "keyboard.h"
#include "ktutils.h"


#define KT_VTE_FONT_SIZE 6

/* Default configuration. */
gboolean ktterm_full_screen = FALSE;
gboolean ktterm_reverse_video = FALSE;
gboolean ktterm_show_keyboard = TRUE;
gboolean ktterm_use_kindle_keyboard = FALSE;

/* Global window (widget) handlers. */
static GtkWidget *window = NULL;
static GtkWidget *terminal = NULL;
static GtkWidget *keyboard = NULL;


static void set_window_size(gboolean keyboard_visible) {

	GdkGeometry hints;

	hints.min_width = hints.max_width = 600;
	hints.min_height = hints.max_height = 800;

	/* height corrections for non-full-screen mode */
	if (!ktterm_full_screen)
		hints.min_height = hints.max_height -= 30;

	/* correction for Kindle keyboard */
	if (ktterm_use_kindle_keyboard && keyboard_visible)
		hints.min_height = hints.max_height -= 275;

	gtk_window_set_geometry_hints((GtkWindow *)(window), window, &hints,
			(GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

static void show_keyboard(gboolean show) {

#if HAVE_LIPC_H
#else
	pid_t lipc_pid;
	char lipc_set_prop_bin[] = "/usr/bin/lipc-set-prop";
	char lipc_action_open[] = "open";
	char lipc_action_close[] = "close";
	char lipc_action_data[32];
	char *v_argv[8] = { lipc_set_prop_bin, "-s", "com.lab126.keyboard", lipc_action_open, lipc_action_data, NULL };
#endif

#ifdef DEBUG
	printf("show_keyboard: %d\n", config.show_keyboard);
#endif

	/* use external Kindle-on-board keyboard */
	if (ktterm_use_kindle_keyboard) {

		/* show keyboard, and then shrink the terminal window */
		if (show) {
#if HAVE_LIPC_H
#else
			v_argv[3] = lipc_action_open;
			sprintf(lipc_action_data, "%s:abc:0", PACKAGE);
			posix_spawn(&lipc_pid, lipc_set_prop_bin, NULL, NULL, v_argv, NULL);
#endif

			set_window_size(TRUE);
		}
		/* enlarge the terminal window, and then hide keyboard */
		else {
			set_window_size(FALSE);

#if HAVE_LIPC_H
#else
			v_argv[3] = lipc_action_close;
			sprintf(lipc_action_data, "%s", PACKAGE);
			posix_spawn(&lipc_pid, lipc_set_prop_bin, NULL, NULL, v_argv, NULL);
#endif
		}

	}
	/* use the embedded keyboard */
	else {
		if (show)
			gtk_widget_show(keyboard);
		else
			gtk_widget_hide(keyboard);
	}
}

/* Callback function which handles terminal button press event. Here, we can
 * define an action based on the press position in the terminal widget. */
static gboolean terminal_event_callback(GtkWidget *widget, GdkEventButton *event, gpointer data) {
	(void)data;

#ifdef DEBUG
	printf("bpt: %d %d (%f, %f)\n", event->type, event->button, event->x, event->y);
#endif

	/* process "hold" actions */
	if (event->button == KT_TOUCH_HOLD) {
		GtkRequisition requisition;

		gtk_widget_size_request(widget, &requisition);

		/* keyboard toggling (upper-right corner) */
		if (event->x >= requisition.width * 0.90 && event->y <= requisition.height * 0.10) {
			ktterm_show_keyboard ^= TRUE;
			show_keyboard(ktterm_show_keyboard);
		}

		/* prevent terminal widget from processing event (just in case) */
		return TRUE;
	}

	return FALSE;
}

/* This callback function handles special sequences (prefixed with the "\uf1f1")
 * from the keyboard configuration file. When such a sequence is recognized, then
 * we are going to proceed it internally. */
static gboolean keyboard_key_callback(ktkb_key_mode *key, void *data) {
	(void)data;

	/* decoded sequence prefix used for special action - JSON "\uf1f1" */
	const char prefix[3] = { 0xef, 0x87, 0xb1 };

	int special_value;
	int font_size;

	/* check special sequence length and prefix */
	if (!(key->length == 5 && memcmp(key->sequence, prefix, 3) == 0))
		return FALSE;

	/* XXX: convert last 2 bytes of special sequence into integer value */
	special_value = *((uint16_t *)&key->sequence[3]);

#define _SSV(a, b) (((b) << 8) | (a))
	switch (special_value) {
	case _SSV('F', '+'):
		font_size = kt_terminal_get_font_size((VteTerminal *)terminal);
		kt_terminal_set_font_size((VteTerminal *)terminal, font_size + 1);
		break;
	case _SSV('F', '-'):
		font_size = kt_terminal_get_font_size((VteTerminal *)terminal);
		kt_terminal_set_font_size((VteTerminal *)terminal, font_size - 1);
		break;
	case _SSV('F', '0'):
		kt_terminal_set_font_size((VteTerminal *)terminal, KT_VTE_FONT_SIZE);
		break;
	case _SSV('W', 'F'):
		ktterm_full_screen ^= TRUE;
		if (ktterm_full_screen)
			kt_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_FULLSCREEN, PACKAGE);
		else
			kt_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_MAXIMIZED, PACKAGE);
		set_window_size(ktterm_show_keyboard);
		break;
	}

	return TRUE;
}

int main(int argc, char *argv[]) {

	/* initialize GTK and process standard arguments */
	gtk_init(&argc, &argv);

	int opt;

	int font_size = KT_VTE_FONT_SIZE;
	const char *resources_dir = DATADIR;

	while ((opt = getopt(argc, argv, "hKR:e:f:rv")) != -1)
		switch (opt) {
		case 'h':
			printf("usage: %s [options] [-e CMD [ args ]]\n"
					"  -K\t\tuse build-in Kindle keyboard\n"
					"  -R DIR\tuse DIR as a resources directory\n"
					"  -e CMD\trun command with its arguments\n"
					"  -f NB\t\tset terminal font size to NB\n"
					"  -r\t\tuse reversed color palette\n"
					"  -v\t\trun in the full-screen mode\n",
					argv[0]);
			return EXIT_SUCCESS;

		case 'K':
			ktterm_use_kindle_keyboard = TRUE;
			break;
		case 'R':
			resources_dir = optarg;
			break;
		case 'e':
			argv = &argv[optind - 1];
			goto main;
		case 'f':
			font_size = atoi(optarg);
			break;
		case 'r':
			ktterm_reverse_video = TRUE;
			break;
		case 'v':
			ktterm_full_screen = TRUE;
			break;

		default:
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return EXIT_FAILURE;
		}

	/* use default Unix shell as a main command */
	const char *shell[] = { "/bin/sh", NULL };
	argv = (char **)&shell;

main:
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	kt_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_MAXIMIZED, PACKAGE);
	if (ktterm_full_screen)
		kt_window_set_placement((GtkWindow *)window, KT_WINDOW_PLACEMENT_FULLSCREEN, PACKAGE);

	gtk_window_set_resizable((GtkWindow *)window, FALSE);

	terminal = vte_terminal_new();
	keyboard = gtk_event_box_new();

	/* initialize VTE widget */
	vte_terminal_set_cursor_blink_mode((VteTerminal *)terminal, VTE_CURSOR_BLINK_OFF);
	vte_terminal_set_cursor_shape((VteTerminal *)terminal, VTE_CURSOR_SHAPE_UNDERLINE);
	vte_terminal_set_scroll_on_output((VteTerminal *)terminal, TRUE);
	kt_terminal_set_colors((VteTerminal *)terminal, ktterm_reverse_video);
	kt_terminal_set_font_size((VteTerminal *)terminal, font_size);
	vte_terminal_fork_command_full((VteTerminal *)terminal, VTE_PTY_DEFAULT,
			NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);

	/* load resources for the embedded keyboard */
	if (!ktterm_use_kindle_keyboard) {

		gchar *tmp1 = g_strdup_printf("%s/keyboard.png", resources_dir);
		gchar *tmp2 = g_strdup_printf("%s/keyboard.cfg", resources_dir);
		ktkb_keyboard *kb;

		if ((kb = embedded_kb_new(keyboard, (VteTerminal *)terminal, tmp1, tmp2)) == NULL) {
			fprintf(stderr, "error: unable to load embedded keyboard\n");
			return EXIT_FAILURE;
		}

		embedded_kb_set_key_callback(kb, keyboard_key_callback, NULL);

		g_free(tmp1);
		g_free(tmp2);
	}

	/* stitch everything together */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start((GtkBox *)vbox, terminal, TRUE, TRUE, 0);
	gtk_box_pack_end((GtkBox *)vbox, keyboard, FALSE, FALSE, 0);
	gtk_container_add((GtkContainer *)window, vbox);

	show_keyboard(TRUE);
	set_window_size(TRUE);

	gtk_widget_show_all(window);

	gtk_widget_add_events(terminal, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(terminal, "motion-notify-event", G_CALLBACK(gtk_true), NULL);
	g_signal_connect(terminal, "button-press-event", G_CALLBACK(terminal_event_callback), NULL);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(terminal, "child-exited", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	/* proper shut down of the external keyboard is required */
	show_keyboard(FALSE);

	/* Skip explicit memory free action, since it will be freed
	 * by the OS anyway - save precious bytes. */
	return EXIT_SUCCESS;
}
