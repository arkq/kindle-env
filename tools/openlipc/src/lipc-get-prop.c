/*
 * [open]lipc - lipc-get-prop.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of openlipc.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 * This source code is based on the reverse-engineered application, which is
 * a part of the Kindle firmware. The original lipc-get-prop application is
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

	int integer = 1;
	int end_nl = 1;
	int quiet = 0;

	while ((opt = getopt(argc, argv, "hiseq")) != -1)
		switch (opt) {
		case 'h':
			printf("usage: %s [-iseq] <publisher> <property>\n\n"
				"  publisher - the unique name of the publisher\n"
				"  property  - the name of the property to get\n"
				"\n"
				"options:\n"
				"  -i\tpublisher published an integer property\n"
				"  -s\tpublisher published a string property\n"
				"  -e\tdo not print new line at the end\n"
				"  -q\tdo not print error message\n",
				argv[0]);
			return EXIT_SUCCESS;

		case 'i':
			integer = 1;
			break;
		case 's':
			integer = 0;
			break;
		case 'e':
			end_nl = 0;
			break;
		case 'q':
			quiet = 1;
			break;

		default:
return_usage:
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return EXIT_FAILURE;
		}

	if (argc - optind != 2)
		goto return_usage;

	const char *source = argv[optind];
	const char *property = argv[optind + 1];
	LIPCcode code;
	LIPC *lipc;

	openlog("lipc-get-prop", LOG_PID | LOG_CONS, LOG_LOCAL0);
	LipcSetLlog(LAB126_LOG_ALL & ~LAB126_LOG_DEBUG_ALL);

	if ((lipc = LipcOpenNoName()) == NULL) {
		if (g_lab126_log_mask & LAB126_LOG_ERROR)
			syslog(LOG_ERR, "E def:open::Failed to open LIPC");
		fprintf(stderr, "error: failed to open lipc\n");
		return EXIT_FAILURE;
	}

	if (integer) {
		int value;
		if ((code = LipcGetIntProperty(lipc, source, property, &value)) == LIPC_OK)
			printf("%d", value);
	}
	else {
		char *value;
		if ((code = LipcGetStringProperty(lipc, source, property, &value)) == LIPC_OK) {
			printf("%s", value);
			LipcFreeString(value);
		}
	}

	if (code == LIPC_OK && end_nl)
		printf("\n");

	if (code != LIPC_OK && !quiet) {
		if (g_lab126_log_mask & LAB126_LOG_ERROR)
			syslog(LOG_ERR, "E def:fail:source=%s, prop=%s:Failed to get property", source, property);
		fprintf(stderr, "error: %s failed to access property %s (0x%x %s)\n",
				source, property, code, LipcGetErrorString(code));
	}

	LipcClose(lipc);
	return code == LIPC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
