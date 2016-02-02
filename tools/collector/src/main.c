/*
 * collector - main.c
 * Copyright (c) 2012-2016 Arkadiusz Bokowy
 *
 * This file is a part of a collector.
 *
 * This projected is licensed under the terms of the MIT license.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <glib.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <uuid/uuid.h>

#include "find.h"


#define KT_DOCUMENTS_DIR "/mnt/us/documents/"
#define KT_COLLECTIONS_DB "/var/local/cc.db"
#define KT_MANAGER_URL "http://localhost:9101/change"

#define COLLECTION_UUID_FNAME ".kmcollection"
#define COLLECTION_SKIP_FNAME ".kmskip"


/* Collection entry structure. */
struct collection_entry {
	uuid_t uuid;
	char name[128];
};

/* Collection structure. */
struct collection {
	uuid_t uuid;
	char name[128];
	GArray *entries;
};


/* Global handler for the Kindle database. */
static sqlite3 *kindle_ppdb = NULL;


/* Get or create collection UUID from the given path. When the UUID id created
 * or not created respectively 1 or 0 is returned. If the given path should be
 * skipped (path explicitly excluded) this function returns 2. Upon failure -1
 * is returned and the uuid parameter is not modified. */
static int get_or_create_collection_uuid(const char *path, uuid_t uuid) {

	char uuid_s[36 + 1] = { 0 };
	FILE *uuid_f;
	char *tmp;
	int retval;

	tmp = joindir(path, COLLECTION_SKIP_FNAME);
	retval = access(tmp, F_OK);
	free(tmp);

	/* path is explicitly marked to be skipped */
	if (retval == 0)
		return 2;

	tmp = joindir(path, COLLECTION_UUID_FNAME);
	uuid_f = fopen(tmp, "a+");
	free(tmp);

	/* unable to open or create collection UUID file */
	if (uuid_f == NULL)
		return -1;

	retval = 0;
	rewind(uuid_f);

	/* regenerate UUID upon reading or parsing failure */
	if (fread(uuid_s, sizeof(uuid_s) - 1, 1, uuid_f) != 1 ||
			uuid_parse(uuid_s, uuid) == -1) {

		uuid_generate(uuid);
		uuid_unparse(uuid, uuid_s);

		uuid_f = freopen(NULL, "w", uuid_f);
		fwrite(uuid_s, sizeof(uuid_s) - 1, 1, uuid_f);

		retval = 1;
	}

	fclose(uuid_f);
	return retval;
}

/* Retrieve entry UUID from the Kindle database. If the UUID was successfully
 * retrieved (given entry was present in the database), this function returns
 * 1, otherwise 0 is returns. Upon error this function returns -1. */
static int kindle_db_get_entry_uuid(const char *path, const char *entry, uuid_t uuid) {

	const char sql[] = "SELECT p_uuid FROM Entries WHERE p_location = ?";
	sqlite3_stmt *stmt;
	int retval = 0;

	if (sqlite3_prepare_v2(kindle_ppdb, sql, sizeof(sql), &stmt, NULL) != SQLITE_OK)
		return -1;

	if (sqlite3_bind_text(stmt, 1, joindir(path, entry), -1, free) != SQLITE_OK)
		return -1;

	/* execute query and retrieve entry UUID */
	if (sqlite3_step(stmt) == SQLITE_ROW)
		if (uuid_parse((char *)sqlite3_column_text(stmt, 0), uuid) == 0)
			retval = 1;

	sqlite3_finalize(stmt);
	return retval;
}

/* Callback function for find utility which adds new entry do the collection
 * table. If required, this function creates new collection (with new UUID)
 * as well. */
static int collection_add_entry(const char *path, const char *entry,
		const struct stat *entry_stat, void *data) {

	GHashTable *collections = (GHashTable *)data;
	struct collection *collection;
	struct collection_entry collection_entry;
	uuid_t collection_uuid;
	int status;

	/* we are interested in regular files only */
	if (!S_ISREG(entry_stat->st_mode))
		return 0;

	/* skip processing if we hit configuration files */
	if (strcmp(entry, COLLECTION_UUID_FNAME) == 0 || strcmp(entry, COLLECTION_SKIP_FNAME) == 0)
		return 0;

	/* try to get collection UUID */
	status = get_or_create_collection_uuid(path, collection_uuid);
	if (status == -1 || status == 2)
		return 0;

	/* initialize new collection if required */
	if ((collection = (struct collection *)g_hash_table_lookup(collections, path)) == NULL) {

		collection = g_malloc(sizeof(*collection));
		strcpy(collection->name, strrchr(path, DIR_SEPARATOR) + 1);
		memcpy(collection->uuid, collection_uuid, sizeof(collection->uuid));
		collection->entries = g_array_sized_new(FALSE, FALSE, sizeof(struct collection_entry), 5);

		g_hash_table_insert(collections, (gpointer)path, collection);
	}

	/* add new book to the collection */
	if (kindle_db_get_entry_uuid(path, entry, collection_entry.uuid) == 1) {
		strcpy(collection_entry.name, entry);
		g_array_append_val(collection->entries, collection_entry);
		return 1;
	}

	return 0;
}

/* Free resources allocated by the collection_add_entry() function. */
static void collection_free(gpointer data) {
	struct collection *collection = (struct collection *)data;
	g_array_free(collection->entries, TRUE);
	g_free(collection);
}

/* Callback function for GLib table foreach iterator, which selects empty
 * collections. */
static gboolean collection_filter_empty(gpointer key, gpointer value, gpointer user_data) {
	(void)key;
	(void)user_data;
	return !((struct collection *)value)->entries->len;
}

/* Callback function for GLib table foreach iterator, which adds "insert
 * collection" command to the Kindle content manager action commands. */
static void manager_insert_collection(gpointer key, gpointer value, gpointer user_data) {
	(void)key;

	struct collection *collection = (struct collection *)value;
	FILE *buffer_f = (FILE *)user_data;
	char uuid_s[36 + 1];

	uuid_unparse(collection->uuid, uuid_s);
	fprintf(buffer_f, "{\"insert\":{\"type\":\"Collection\",\"uuid\":\"%s\"," \
			"\"lastAccess\":%d,\"titles\":[{\"display\":\"%s\",\"direction\":\"LTR\"," \
			"\"language\":\"en-US\"}],\"isVisibleInHome\":true}},",
			uuid_s, (int)time(NULL), collection->name);
}

/* Callback function for GLib table foreach iterator, which adds "update
 * collection" command to the Kindle content manager action commands. */
static void manager_update_collection(gpointer key, gpointer value, gpointer user_data) {
	(void)key;

	struct collection *collection = (struct collection *)value;
	FILE *buffer_f = (FILE *)user_data;
	char uuid_s[36 + 1];
	size_t i;

	uuid_unparse(collection->uuid, uuid_s);
	fprintf(buffer_f, "{\"update\":{\"type\":\"Collection\",\"uuid\":\"%s\",\"members\":[", uuid_s);

	for (i = 0; i < collection->entries->len; i++) {
		uuid_unparse(g_array_index(collection->entries, struct collection_entry, i).uuid, uuid_s);
		fprintf(buffer_f, "\"%s\",", uuid_s);
	}

	if (collection->entries->len)
		fseek(buffer_f, -1, SEEK_CUR);

	fprintf(buffer_f, "]}},");
}

/* Perform Kindle collections update. */
static void update_collections(GHashTable *collections) {

	CURL *curl;
	CURLcode errornum;
	FILE *buffer_f;
	int buffer_size, pos;

	if ((curl = curl_easy_init()) == NULL)
		return;

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_URL, KT_MANAGER_URL);

	buffer_f = tmpfile();

	/* build action commands for Kindle Touch content manager */

	fprintf(buffer_f, "{\"commands\":[");

	pos = ftell(buffer_f);
	g_hash_table_foreach(collections, manager_insert_collection, (gpointer)buffer_f);
	g_hash_table_foreach(collections, manager_update_collection, (gpointer)buffer_f);

	/* XXX: strip trailing comma character */
	if (pos != ftell(buffer_f))
		fseek(buffer_f, -1, SEEK_CUR);

	fprintf(buffer_f, "],\"type\":\"ChangeRequest\",\"id\":%u}", (int)time(NULL));

	/* execute content manager action commands */

	fseek(buffer_f, 0, SEEK_END);
	buffer_size = ftell(buffer_f);
	rewind(buffer_f);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buffer_size);
	curl_easy_setopt(curl, CURLOPT_READDATA, buffer_f);

	if ((errornum = curl_easy_perform(curl)) != CURLE_OK)
		fprintf(stderr, "error: %s\n", curl_easy_strerror(errornum));

	curl_easy_cleanup(curl);
	fclose(buffer_f);
}

int main(void) {

	GHashTable *collections;
	int count;

	/* try to acquire a handler for the Kindle database */
	if (sqlite3_open_v2(KT_COLLECTIONS_DB, &kindle_ppdb, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		fprintf(stderr, "error: %s\n", sqlite3_errmsg(kindle_ppdb));
		sqlite3_close(kindle_ppdb);
		return EXIT_FAILURE;
	}

	/* collect all entries (books) associated with collections */
	collections = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, collection_free);
	count = find(KT_DOCUMENTS_DIR, collection_add_entry, collections);
	g_hash_table_foreach_remove(collections, collection_filter_empty, NULL);

	fprintf(stderr, "info: found %d book(s) associated with collections\n", count);

	update_collections(collections);

	sqlite3_close(kindle_ppdb);
	return EXIT_SUCCESS;
}
