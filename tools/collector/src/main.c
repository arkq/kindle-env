/*
 * collector - main.c
 * Copyright (c) 2012 Arkadiusz Bokowy
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

#include <sqlite3.h>
#include <curl/curl.h>

#include "find.h"
#include "kbooks.h"


#ifdef DEBUG
static int show_debug = 0;
#endif


int get_or_create_collection_uuid(const char *dirname, uuid_t uuid) {

	int uuid_fp;
	char *fname, uuid_s[36 + 1];

	fname = malloc(strlen(dirname) + sizeof(COLLECTION_UUID_FNAME) + 2);
	sprintf(fname, "%s%c%s", dirname, DIR_SEPARATOR, COLLECTION_UUID_FNAME);

	uuid_fp = open(fname, O_RDWR | O_CREAT, 0644);
	free(fname);

	/* file access error - something is wrong... */
	if (uuid_fp == -1) {
#ifdef DEBUG
		if (show_debug) printf("error: uuid fname access error (%s)\n", dirname);
#endif
		return -1;
	}

	memset(uuid_s, 0, sizeof(uuid_s));
	read(uuid_fp, uuid_s, sizeof(uuid_s) - 1);

	/* we have successfully parsed UUID from file */
	if (uuid_parse(uuid_s, uuid) == 0) {
		close(uuid_fp); return 0;}

	/* generate new UUID and write it to the file */
	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_s);
	lseek(uuid_fp, 0, SEEK_SET);
	write(uuid_fp, uuid_s, sizeof(uuid_s) - 1);
	ftruncate(uuid_fp, sizeof(uuid_s) - 1);

#ifdef DEBUG
	if (show_debug) printf("info: generated new uuid for collection (%s)\n", dirname);
#endif

	close(uuid_fp);
	return 1;
}

int skip_collection(const char *dirname) {
	char *fname;
	int skip;

	fname = malloc(strlen(dirname) + sizeof(COLLECTION_SKIP_FNAME) + 2);
	sprintf(fname, "%s%c%s", dirname, DIR_SEPARATOR, COLLECTION_SKIP_FNAME);

	skip = access(fname, F_OK);
	free(fname);

#ifdef DEBUG
	if (!skip && show_debug)
		printf("info: skipping collection (%s)\n", dirname);
#endif

	return !skip;
}

int get_entry_uuid_from_kindle_db(const char *dirname, const char *entry, uuid_t uuid) {
#define COLLECTION_SQL "SELECT p_uuid FROM Entries WHERE p_location = ?"
	sqlite3 *p_db;
	sqlite3_stmt *stmt;
	char *fname;
	int ret;

	ret = sqlite3_open_v2(KT_COLLECTIONS_DB, &p_db, SQLITE_OPEN_READONLY, NULL);

	/* there was some error during the database opening */
	if (ret != SQLITE_OK) {
#ifdef DEBUG
		if (show_debug)
			printf("error: unable to open Kindle DB\n");
#endif
		sqlite3_close(p_db); return -1;}

	ret = sqlite3_prepare_v2(p_db, COLLECTION_SQL, sizeof(COLLECTION_SQL) + 1, &stmt, NULL);

	/* error during the statement compilation */
	if (ret != SQLITE_OK) {
#ifdef DEBUG
		if (show_debug) printf("error: unable to prepare DB SQL query\n");
#endif
		sqlite3_close(p_db); return -1;}

	fname = malloc(strlen(dirname) + strlen(entry) + 2);
	sprintf(fname, "%s%c%s", dirname, DIR_SEPARATOR, entry);

	/* this function itself will freed the `fname` variable */
	ret = sqlite3_bind_text(stmt, 1, fname, -1, free);

	/* error during the parameter binding */
	if (ret != SQLITE_OK) {
#ifdef DEBUG
		if (show_debug)
			printf("error: unable to bind SQL parameter\n");
#endif
		sqlite3_close(p_db); return -1;}

	ret = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		/* we've got some data in here */
		if (uuid_parse((const char*)sqlite3_column_text(stmt, 0), uuid) == 0)
			ret = 1;

#ifdef DEBUG
	if (show_debug)
		printf("info: Kindle DB query status: %d\n", ret);
#endif

	sqlite3_finalize(stmt);
	sqlite3_close(p_db);
	return ret;
#undef COLLECTION_SQL
}

int collect_collections(const char *dirname, const char *entry,
		const struct stat *entry_stat, void *data) {

	GHashTable *collections = (GHashTable*)data;
	kb_collection_t *kb_collection;
	kb_book_t kb_book;
	int created;

	/* we are interested only in the files :) */
	if (!S_ISREG(entry_stat->st_mode)) return 0;

	/* skip processing if we hit configuration files */
	if (strcmp(entry, COLLECTION_UUID_FNAME) == 0 || strcmp(entry, COLLECTION_SKIP_FNAME) == 0)
		return 0;

	kb_collection = (kb_collection_t*)g_hash_table_lookup(collections, dirname);

	/* initialize new collection */
	if (kb_collection == NULL) {
		kb_collection = malloc(sizeof(kb_collection_t));

		created = get_or_create_collection_uuid(dirname, kb_collection->uuid);

		/* something is wrong with the file system, skip collection */
		if (created == -1)
			return 0;

		g_hash_table_insert(collections, (gpointer)dirname, kb_collection);

		strcpy(kb_collection->name, strrchr(dirname, DIR_SEPARATOR) + 1);
		kb_collection->is_active = !skip_collection(dirname);
		kb_collection->is_created = !created;
		kb_collection->books = g_array_sized_new(FALSE, FALSE, sizeof(kb_book_t), 5);

#ifdef DEBUG
		if (show_debug) printf("info: new collection (%s) |%d %d\n",
				kb_collection->name, kb_collection->is_active, kb_collection->is_created);
#endif
	}

	if (get_entry_uuid_from_kindle_db(dirname, entry, kb_book.uuid) == 1) {
		/* add new book to the collection */
		strcpy(kb_book.name, entry);
		g_array_append_val(kb_collection->books, kb_book);

#ifdef DEBUG
		if (show_debug) printf("info: added book (%s) to collection (%s)\n",
				kb_book.name, kb_collection->name);
#endif

		return 1;
	}

	return 0;
}

gboolean drop_empty_or_inactive_collections(gpointer key, gpointer value, gpointer user_data) {
	kb_collection_t *kb_collection = (kb_collection_t*)value;

#ifdef DEBUG
	if (show_debug) printf("info: drop collection: len: %d, active:%d\n",
			kb_collection->books->len, kb_collection->is_active);
#endif

	if (kb_collection->books->len && kb_collection->is_active)
		return FALSE;
	return TRUE;
}

/* Free collection structure allocated by the collect_collections function. */
void free_collection(gpointer data) {
	kb_collection_t *kb_collection = (kb_collection_t*)data;
	g_array_free(kb_collection->books, TRUE);
	free(kb_collection);
}

void kt_manager_add_collections(gpointer key, gpointer value, gpointer user_data) {
#define KT_MANAGER_COMMAND_INSERT "{\"insert\":{\"type\":\"Collection\"," \
	"\"uuid\":\"%s\",\"lastAccess\":%d,\"titles\":[{\"display\":\"%s\"," \
	"\"direction\":\"LTR\",\"language\":\"en-US\"}],\"isVisibleInHome\":true}}"
	kb_collection_t *kb_collection = (kb_collection_t*)value;
	FILE *buffer_f = (FILE*)user_data;
	char uuid_s[36 + 1];

	if (kb_collection->is_created) return;

	uuid_unparse(kb_collection->uuid, uuid_s);
	fprintf(buffer_f, KT_MANAGER_COMMAND_INSERT ",", uuid_s, (int)time(NULL), kb_collection->name);
}

void kt_manager_update_collections(gpointer key, gpointer value, gpointer user_data) {
#define KT_MANAGER_COMMAND_UPDATE "{\"update\":{\"type\":\"Collection\"," \
	"\"uuid\":\"%s\",\"members\":[%s]}}"
	kb_collection_t *kb_collection = (kb_collection_t*)value;
	kb_book_t *kb_book;
	FILE *buffer_f = (FILE*)user_data;
	char uuid_s[36 + 1], json_uuid_s[36 + 2 + 1], *members_uuids;
	int i;

	/* create buffer big enough to contains UUID string separated with ',' */
	members_uuids = malloc((sizeof(json_uuid_s) + 1 - 1) * kb_collection->books->len + 1);
	/* initialize string to the zero length */
	members_uuids[0] = 0;
	for (i = 0; i < kb_collection->books->len; i++) {
		kb_book = &g_array_index(kb_collection->books, kb_book_t, i);

		uuid_unparse(kb_book->uuid, uuid_s);
		sprintf(json_uuid_s, "\"%s\"", uuid_s);
		strcat(members_uuids, json_uuid_s);

		if (i + 1 != kb_collection->books->len) strcat(members_uuids, ",");
	}

	uuid_unparse(kb_collection->uuid, uuid_s);
	fprintf(buffer_f, KT_MANAGER_COMMAND_UPDATE ",", uuid_s, members_uuids);
	free(members_uuids);
}

void kt_manager_execute(GHashTable *collections) {
	FILE *buffer_f;
	CURL *curl;
	int prev_pos, buffer_size;

	/* prepare commands for Kindle Touch content manager */
#define KT_MANAGER_ACTIONS_BEGIN "{\"commands\":["
#define KT_MANAGER_ACTIONS_END "],\"type\":\"ChangeRequest\",\"id\":666}"
	buffer_f = tmpfile();
	fwrite(KT_MANAGER_ACTIONS_BEGIN, sizeof(KT_MANAGER_ACTIONS_BEGIN) - 1, 1, buffer_f);

	prev_pos = ftell(buffer_f);
	g_hash_table_foreach(collections, kt_manager_add_collections, (gpointer)buffer_f);
	g_hash_table_foreach(collections, kt_manager_update_collections, (gpointer)buffer_f);

	/* hack: skip traling ',' character */
	if (prev_pos != ftell(buffer_f))
		fseek(buffer_f, -1, SEEK_CUR);

	fwrite(KT_MANAGER_ACTIONS_END, sizeof(KT_MANAGER_ACTIONS_END) - 1 , 1, buffer_f);

	fseek(buffer_f, 0, SEEK_END);
	buffer_size = ftell(buffer_f);

#ifdef DEBUG
	/* dump buffer_f to stdout */
	if (show_debug) {
		char *buffer = malloc(buffer_size + 1);

		rewind(buffer_f);
		fread(buffer, buffer_size, 1, buffer_f);
		buffer[buffer_size] = 0;

		printf("info: KT manager - execute:\n%s\n", buffer);
		free(buffer);
	}
#endif

	if ((curl = curl_easy_init()) == NULL) {
#ifdef DEBUG
		if (show_debug) printf("error: unable to initialize curl\n");
#endif
		fclose(buffer_f); return;}

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_URL, KT_MANAGER_URL);

	rewind(buffer_f);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buffer_size);
	curl_easy_setopt(curl, CURLOPT_READDATA, buffer_f);

	if (curl_easy_perform(curl) != 0)
		printf("error: curl_easy_perform failed\n");

	curl_easy_cleanup(curl);
	fclose(buffer_f);
}

int main(int argc, char *argv[]) {

	GHashTable *collections;
	int prev_count, count;

#ifdef DEBUG
	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		show_debug = 1;
#endif

	/* initialize collections table */
	collections = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free_collection);
	count = find(KT_DOCUMENTS_DIR, collect_collections, collections);
	printf("info: found %d book(s) associated with collections\n", count);

	prev_count = g_hash_table_size(collections);
	count = g_hash_table_foreach_remove(collections, drop_empty_or_inactive_collections, NULL);
	printf("info: dropped %d of %d empty (inactive) collections\n", count, prev_count);

	kt_manager_execute(collections);

	g_hash_table_destroy(collections);
	return 0;
}
