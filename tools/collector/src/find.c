/*
 * collector - find.c
 * Copyright (c) 2012 Arkadiusz Bokowy
 *
 * This file is a part of a collector.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include "find.h"

#include <stdlib.h>
#include <string.h>


/* This is a simple C implementation of the standard UNIX find tool. This
 * function gets initial directory and traverses over every descending path
 * in the tree. On every item, the callback function is called with the user
 * defined data pointer. If callback returns non-zero value, then match is
 * counted, and the summed-up matches are returned by the find function. */
int find(const char *dirname, find_callback_t callback, void *data) {

	DIR *dir;
	struct dirent *dp;
	struct stat sb;
	int ret, counter = 0;
	char *dirname_c;

	if ((dir = opendir(dirname)) == NULL)
		return -1;

	while ((dp = readdir(dir)) != NULL) {

		/* skip special file names */
		if (strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".") == 0)
			continue;

		/* fill-in file status structure */
		dirname_c = joindir(dirname, dp->d_name);
		if (lstat(dirname_c, &sb) == -1) {
			free(dirname_c);
			continue;
		}

		if (callback(dirname, dp->d_name, &sb, data))
			counter++;

		/* iterate over next tree level */
		if (S_ISDIR(sb.st_mode)) {
			ret = find(dirname_c, callback, data);
			if(ret != -1) counter += ret;
		}

		free(dirname_c);
	}

	closedir(dir);
	return counter;
}

/* Join two paths together and insert directory separator between them if
 * needed. Note, that pointer returned by this function has to be freed. */
char *joindir(const char *path1, const char *path2) {

	char *path;
	int len1 = strlen(path1), len2 = strlen(path2);

	path = malloc(len1 + len2 + 2);

	strcpy(path, path1);
	if (len1 && path[len1 - 1] != DIR_SEPARATOR) {
		path[len1] = DIR_SEPARATOR;
		path[len1 + 1] = 0;
	}

	while (*path2 == DIR_SEPARATOR)
		path2++;

	return strcat(path, path2);
}
