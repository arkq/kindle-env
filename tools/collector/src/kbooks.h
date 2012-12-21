/*
 * collector - kbooks.h
 * Copyright (c) 2012 Arkadiusz Bokowy
 *
 * This file is a part of a collector.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#ifndef COLLECTOR_KBOOKS_H_
#define COLLECTOR_KBOOKS_H_

#define APP_NAME "collector"
#define APP_VER "0.1.0"

#include <glib.h>
#include <uuid/uuid.h>

#define KT_DOCUMENTS_DIR "/mnt/us/documents/"
#define KT_COLLECTIONS_DB "/var/local/cc.db"
#define KT_MANAGER_URL "http://localhost:9101/change"

#define COLLECTION_UUID_FNAME ".kmcollection"
#define COLLECTION_SKIP_FNAME ".kmskip"


typedef struct kbooks_book_tag {
	uuid_t uuid;
	char name[128];
} kb_book_t;

/* collection storage structure */
typedef struct kbooks_collection_tag {
	uuid_t uuid;
	char name[128];
	int is_active, is_created;
	GArray *books;
} kb_collection_t;

#endif  /* COLLECTOR_KBOOKS_H_ */
