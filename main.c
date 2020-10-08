#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>


sqlite3 *DB_INDEXES = NULL;
sqlite3 *DB_FILES = NULL;

#include "filling_databases.h"

int get_result(char **keys, int n)
{
    if (DB_INDEXES == NULL) {
        int exit = 0; 
        exit = sqlite3_open("indexing.db", &DB_INDEXES); 

        if (exit) { 
            printf("FAILED OPENING indexes.db\n");
            return FAILED_READING; 
        } 
    }

    char buf[WORD_BUFFER];
    const char *DOCUMENTS;
    struct sqlite3_stmt *selectstmt;

    char *err_msg;
    for (int i = 0; i < n; ++i) {
        sprintf(buf, "SELECT DOCUMENTS FROM INDEXING WHERE KEYWORD='%s'", keys[i]);
        int result = sqlite3_prepare_v2(DB_INDEXES, buf, -1, &selectstmt, NULL);
        if (result == SQLITE_OK && sqlite3_step(selectstmt) == SQLITE_ROW) {
            DOCUMENTS = sqlite3_column_text(selectstmt, 0);
            printf("%s --> %s\n", keys[i], DOCUMENTS);
        }
        else { 
            fprintf(stderr, "Sorry %s isn't found in any document present\n", keys[i]);
        }
    }
    return (0); 

}

int main(int argc, char *argv[])
{

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        printf("%s -r\t\tretrive files\n", argv[0]);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "-r") == 0) {
        retrive_files();
    }
    else if (argc == 1){
        printf("Enter keywords: ");
        char buf[WORD_BUFFER];
        char **keys = malloc(80 * sizeof (char *));
        int n = 0;
        while (fscanf(stdin, " %1023s", buf) == 1) {
            keys[n++] = strdup(buf);
        }
        get_result(keys, n);
    }
    else {
        get_result(argv+1, argc-1);
    }
    sqlite3_close(DB_INDEXES);
    sqlite3_close(DB_FILES);

}
