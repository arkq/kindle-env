/*
 * collector - find.h
 * Copyright (c) 2012 Arkadiusz Bokowy
 *
 * This file is a part of a collector.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#ifndef COLLECTOR_FIND_H_
#define COLLECTOR_FIND_H_

#include <dirent.h>
#include <sys/stat.h>


#define DIR_SEPARATOR '/'


char *joindir(const char *path1, const char *path2);

typedef int (*find_callback_t)(const char *dirname, const char *entry,
		const struct stat *entry_stat, void *data);
int find(const char *dirname, find_callback_t callback, void *data);

#endif  /* COLLECTOR_FIND_H_ */
