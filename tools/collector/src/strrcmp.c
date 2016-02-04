/*
 * collector - strrcmp.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of a collector.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include "strrcmp.h"

#include <string.h>


/* Compare two strings s1 and s2 in the reversed order. The return value is
 * compatible with the standard strcmp() function. This function compares only
 * the last (at most) n bytes of s1 and s2. */
int strrncmp(const char *s1, const char *s2, size_t n) {

	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char diff;

	for (; len1 && len2; len1--, len2--) {
		if ((diff = s1[len1 - 1] - s2[len2 - 1]) != 0)
			return diff;
		if (--n == 0)
			return 0;
	}

	return len1 - len2;
}
