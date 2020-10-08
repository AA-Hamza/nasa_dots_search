#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "hashmap.h"

#define WORD_BUFFER 1024
#define FAILED_READING -1
#define FILE_ADDED -2
#define FILE_EXISTS -3
#define FILE_FAILED_ADDING -4
#define MEMORY_FAIL -5
#define SUCCESS 0

typedef struct {
    char *word;
    int appear;
} keyword;

extern sqlite3 *DB_INDEXES;
extern sqlite3 *DB_FILES;

int indexing(FILE *file, char *name);
int add_file(char *name);
int retrive_files();
int check_documents(const char *Documents, char *target);
int add_document_to_keyword(void *const context, struct hashmap_element_s *const e);
