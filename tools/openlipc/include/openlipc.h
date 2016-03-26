/*
 * [open]lipc - openlipc.h
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of openlipc.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 * This header file is based on the reverse-engineered proprietary library
 * lipc, which is a part of the Kindle firmware. The lipc library itself is
 * copyrighted under the terms of the Amazon Technologies, Inc.
 *
 */

#ifndef OPENLIPC_H
#define OPENLIPC_H

/* Library internal data types. */
typedef void LIPC;
typedef void LIPCevent;

/* Status (error) codes returned by all sorts of lipc library functions.
 * This list was obtained from the LipcGetErrorString function and may
 * be not complete - be prepared for other values as well. */
typedef enum {
	LIPC_OK = 0,
	LIPC_ERROR_UNKNOWN,                 /* 1 */
	LIPC_ERROR_INTERNAL,                /* 2 */
	LIPC_ERROR_NO_SUCH_SOURCE,          /* 3 */
	LIPC_ERROR_OPERATION_NOT_SUPPORTED, /* 4 */
	LIPC_ERROR_OUT_OF_MEMORY,           /* 5 */
	LIPC_ERROR_SUBSCRIPTION_FAILED,     /* 6 */
	LIPC_ERROR_NO_SUCH_PARAM,           /* 7 */
	LIPC_ERROR_NO_SUCH_PROPERTY,        /* 8 */
	LIPC_ERROR_ACCESS_NOT_ALLOWED,      /* 9 */
	LIPC_ERROR_BUFFER_TOO_SMALL,        /* 10 */
	LIPC_ERROR_INVALID_HANDLE,          /* 11 */
	LIPC_ERROR_INVALID_ARG,             /* 12 */
	LIPC_ERROR_OPERATION_NOT_ALLOWED,   /* 13 */
	LIPC_ERROR_PARAMS_SIZE_EXCEEDED,    /* 14 */
	LIPC_ERROR_TIMED_OUT,               /* 15 */
	LIPC_ERROR_SERVICE_NAME_TOO_LONG,   /* 16 */
	LIPC_ERROR_DUPLICATE_SERVICE_NAME,  /* 17 */
	LIPC_ERROR_INIT_DBUS,               /* 18 */
	LIPC_PROP_ERROR_INVALID_STATE   = 0x100,
	LIPC_PROP_ERROR_NOT_INITIALIZED = 0x101,
	LIPC_PROP_ERROR_INTERNAL        = 0x102,
} LIPCcode;

LIPC *LipcOpenNoName();
void LipcClose(LIPC *lipc);

/* Get the list of all available properties for the given source. The list
 * of properties is returned in the space-delimited string. The returned data
 * format is as follows: "<property> <type> <mode> [next list element ]" */
#define LipcGetProperties(lipc, source, value) \
	LipcGetStringProperty(lipc, source, "_properties", value)

LIPCcode LipcGetStringProperty(LIPC *lipc, const char *source, const char *property, char **value);
LIPCcode LipcSetStringProperty(LIPC *lipc, const char *source, const char *property, const char *value);
LIPCcode LipcGetIntProperty(LIPC *lipc, const char *source, const char *property, int *value);
LIPCcode LipcSetIntProperty(LIPC *lipc, const char *source, const char *property, int value);
void LipcFreeString(char *string);

/* Get status code in the string format. */
const char *LipcGetErrorString(LIPCcode code);

/* One can turn on/off logs written to the system log using these logging
 * masks. Valid values for debug levels are from 1 to 8. */
#define LAB126_LOG_DEBUG (n) ((1 << ((n) - 1)) << 8)
#define LAB126_LOG_INFO      (0x0080 << 16)
#define LAB126_LOG_WARNING   (0x0100 << 16)
#define LAB126_LOG_ERROR     (0x0200 << 16)
#define LAB126_LOG_CRITICAL  (0x0400 << 16)
/* Mask shortcuts for the sake of brevity. */
#define LAB126_LOG_DEBUG_ALL  0x0000FF00
#define LAB126_LOG_ALL        0xFFFFFF00

/* Global logging mask. */
extern int g_lab126_log_mask;

/* Set the LIPC internal logging mask. For more information about how to use
 * this function refer to the LAB126_LOG_* definitions. The current state of
 * the logging mask is available via the g_lab126_log_mask global variable. */
int LipcSetLlog(int mask);

#endif /* OPENLIPC_H */
