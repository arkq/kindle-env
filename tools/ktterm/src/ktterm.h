/*
 * ktterm - ktterm.h
 * Copyright (c) 2013 Arkadiusz Bokowy
 *
 * This file is a part of a ktterm.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#define APP_NAME "ktterm"
#define APP_VER "0.1.0"

#ifndef __KTTERM_H
#define __KTTERM_H

/* Kindle mouse gesture (button) definitions. With every button action
 * an appropriate coordinations are associated. "Tap" and "Hold" actions
 * are obvious - action point. For "Double Tap" the coordinations of the
 * "Tap" is returned, and for pinching and stretching the coordinations
 * of an average (in between) point. */
#define KT_TOUCH_TAP 1
#define KT_TOUCH_TAP_DOUBLE 2
#define KT_TOUCH_PINCH 7
#define KT_TOUCH_STRETCH 8
#define KT_TOUCH_HOLD 9

typedef enum {
	/* place window in the application zone */
	KT_WINDOW_PLACEMENT_APPLICATION,
	/* place window in the chrome zone */
	KT_WINDOW_PLACEMENT_MAXIMIZED,
	/* fill-in the whole screen area */
	KT_WINDOW_PLACEMENT_FULLSCREEN,
} KTWindowPlacement;

#define KT_VTE_EXEC "/bin/sh"
#define KT_VTE_FONT_SIZE 6

#define KT_SHARE "/usr/share/ktterm"
#define KT_KB_EMBEDDED_IMG "keyboard.png"
#define KT_KB_EMBEDDED_CFG "keyboard.cfg"

#endif
