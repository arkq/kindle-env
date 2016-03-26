/*
 * [open]lipc - lipc-set-prop.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of openlipc.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 * This source code is based on the reverse-engineered application, which is
 * a part of the Kindle firmware. The original lipc-set-prop application is
 * copyrighted under the terms of the Amazon Technologies, Inc.
 *
 */

#include "openlipc.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>


int main(int argc, char *argv[]) {

	int opt;

	int integer = 0;
	int string = 0;
	int quiet = 0;

	while ((opt = getopt(argc, argv, "hisq")) != -1)
		switch (opt) {
		case 'h':
			printf("usage: %s [-isq] <publisher> <property> <value>\n\n"
				"  publisher - the unique name of the publisher\n"
				"  property  - the name of the property to set\n"
				"  value     - the value to set\n"
				"\n"
				"options:\n"
				"  -i\tpublisher published an integer property\n"
				"  -s\tpublisher published a string property\n"
				"  -q\tdo not print error message\n"
				"\n"
				"The type of the value is assumed to be an integer by default.  If the coercion\n"
				"fails, the string is used instead. One can override this behavior by using one\n"
				"of the -i or -s option.\n",
				argv[0]);
			return EXIT_SUCCESS;

		case 'i':
			integer = 1;
			break;
		case 's':
			string = 1;
			break;
		case 'q':
			quiet = 1;
			break;

		default:
return_usage:
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return EXIT_FAILURE;
		}

	if (argc - optind != 3)
		goto return_usage;

	const char *source = argv[optind];
	const char *property = argv[optind + 1];
	const char *value = argv[optind + 2];
	LIPCcode code;
	LIPC *lipc;

	openlog("lipc-set-prop", LOG_PID | LOG_CONS, LOG_LOCAL0);
	LipcSetLlog(LAB126_LOG_ALL & ~LAB126_LOG_DEBUG_ALL);

	if ((lipc = LipcOpenNoName()) == NULL) {
		if (g_lab126_log_mask & LAB126_LOG_ERROR)
			syslog(LOG_ERR, "E def:open::Failed to open LIPC");
		fprintf(stderr, "error: failed to open lipc\n");
		return EXIT_FAILURE;
	}

	char *tmp;
	int value_int;
	value_int = strtol(value, &tmp, 10);

	if (integer) {
		if (*tmp != '\0') {
			if (!quiet)
				fprintf(stderr, "error: value is not an integer\n");
			code = LIPC_ERROR_INVALID_ARG;
			goto return_failure;
		}
		code = LipcSetIntProperty(lipc, source, property, value_int);
	}
	else if (string) {
		code = LipcSetStringProperty(lipc, source, property, value);
	}
	else {
		if (*tmp == '\0')
			code = LipcSetIntProperty(lipc, source, property, value_int);
		else
			code = LipcSetStringProperty(lipc, source, property, value);
	}

	if (code != LIPC_OK && !quiet) {
		if (g_lab126_log_mask & LAB126_LOG_ERROR)
			syslog(LOG_ERR, "E def:fail:source=%s, prop=%s:Failed to set property", source, property);
		fprintf(stderr, "error: %s failed to set value for property %s (0x%x %s)\n",
				source, property, code, LipcGetErrorString(code));
	}

return_failure:
	LipcClose(lipc);
	return code == LIPC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
