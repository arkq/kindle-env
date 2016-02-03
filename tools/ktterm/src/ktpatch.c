/*
 * ktterm - ktpatch.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include <stdlib.h>
#include <gtk/gtk.h>


/* Function available since GTK >= 2.22, but Kindle uses version 2.10. */
GtkWidget *gtk_accessible_get_widget(GtkAccessible *accessible) {
	return NULL;
}

/* POSIX Thread based GLIB mutex implementation. This function seems to be
 * used by the VTE module, but the library available on the Kindle device
 * lacks mutex support. */
void g_mutex_lock(GMutex *mutex) {

	if G_UNLIKELY (mutex->p == NULL) {
		mutex->p = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mutex->p, NULL);
	}

	pthread_mutex_lock(mutex->p);
}

void g_mutex_unlock(GMutex *mutex) {
	pthread_mutex_unlock(mutex->p);
}
