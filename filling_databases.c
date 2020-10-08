#include "filling_databases.h"
#include <time.h>

int indexing(FILE *file, char *name)
{


    struct hashmap_s keywords;
    if (hashmap_create(1, &keywords) != 0) {
        fprintf(stderr, "Failed creating keywords hashmap");
        return MEMORY_FAIL;
    }

    char read_word[WORD_BUFFER];
    char word[WORD_BUFFER];

    //WORD_BUFFER don't work in the next line but it ain't a much of a problem rn
    while (fscanf(file, " %1023s", read_word) == 1) {
        int index=0;
        for (int i = 0; i < strlen(read_word); ++i) {
            if (isalnum(read_word[i])) {
                word[index++] = tolower(read_word[i]);
            }
        }
        word[index++] = '\0';
        //printf("\n\nBREAKPOINT\n\n");
        void *element = hashmap_get(&keywords, word, index);
        if (element != NULL) {
            *(int *)element = (*(int *)element)+1;
        }
        else { //not found add it 
            char *keyword_word_ptr = malloc(index);
            strcpy(keyword_word_ptr, word);

            int *appearance = malloc(sizeof(int));
            *appearance = 1;
            if (hashmap_put(&keywords, keyword_word_ptr, index, appearance) != 0) {
                fprintf(stderr, "Failed adding a keyword into hashmap");
                return MEMORY_FAIL;
            }
        }
    }

    if (hashmap_iterate_pairs(&keywords, add_document_to_keyword, (char * const)name) != 0) {
        fprintf(stderr, "Couldn't iterate on hashmap\n");
    }
    hashmap_destroy(&keywords);
}

int add_document_to_keyword(void *const context, struct hashmap_element_s *const e)
{
    char buf[WORD_BUFFER];
    char *err_msg = 0;
    int rc = 0;
    char value_buf[WORD_BUFFER*10];
    const char *DOCUMENTS;
    struct sqlite3_stmt *selectstmt;

    sprintf(buf, "SELECT DOCUMENTS FROM INDEXING WHERE KEYWORD='%s'", (char *)e->key);
    int result = sqlite3_prepare_v2(DB_INDEXES, buf, -1, &selectstmt, NULL);
    if(result == SQLITE_OK && sqlite3_step(selectstmt) == SQLITE_ROW) {
        DOCUMENTS = sqlite3_column_text(selectstmt, 0);
        if (!check_documents(DOCUMENTS, (char *)context)) {
            sprintf(value_buf, "UPDATE INDEXING SET DOCUMENTS=DOCUMENTS||'(%s:%d)' WHERE KEYWORD='%s'", (char *)context, *(int *)e->data, (char *)e->key);
            sqlite3_prepare_v2(DB_INDEXES,value_buf, -1, &selectstmt, NULL);
            sqlite3_step(selectstmt);
        }
    }
    else {      //keyword doesn't exist
        sprintf(buf, "INSERT INTO INDEXING VALUES('%s','(%s:%d)')", e->key, (char *)context, *(int *)e->data);
        char *err_msg;
        result = sqlite3_exec(DB_INDEXES, buf ,0,0, &err_msg);
        if (result !=SQLITE_OK) {
            printf("\n%s\n", err_msg);
            fprintf(stderr, "%s", "Error writing into indexing.db\n");
        }
    }
    free(e->data);
    return -1;
}

int add_file(char *name)
{

    if (DB_FILES == NULL) {
        int exit = 0;
        exit = sqlite3_open("files.db", &DB_FILES);

        if (exit) {
            printf("FAILED OPENING files.db\n");
            return FAILED_READING;
        }
        else {
            printf("Opened files.db Successfully!\n");
        }
    }

    char buf[WORD_BUFFER];
    char *err_msg = 0;
    int rc = 0;

    sprintf(buf, "SELECT FILE FROM FILES WHERE FILE='%s'", name);

    struct sqlite3_stmt *selectstmt;
    int result = sqlite3_prepare_v2(DB_FILES, buf, -1, &selectstmt, NULL);
    if(result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            printf("%s already exists in files.db.\n", name);
            return FILE_EXISTS;
        }
        else {
            //printf("Opened %s for reading\n", buf);
            sprintf(buf, "INSERT INTO FILES VALUES('%s')", name);
            //rc = sqlite3_exec(DB_FILES, buf ,0,0, &err_msg);
            result = sqlite3_prepare_v2(DB_FILES, buf, -1, &selectstmt, NULL);
            sqlite3_step(selectstmt);
            sprintf(buf, "documents/%s", name);
            FILE *file = fopen(buf, "r");
            indexing(file, name);
            fclose(file);
            return FILE_ADDED;
        }
    }
    return (0);
}

int retrive_files()
{
    if (DB_INDEXES == NULL) {
        int exit = 0; 
        exit = sqlite3_open("indexing.db", &DB_INDEXES); 
      
        if (exit) { 
            printf("FAILED OPENING indexing.db\n");
            return FAILED_READING; 
        } 
        else {
            printf("Opened indexing.db Successfully!\n");
        }
    }

    char *err_msg = NULL;
    sqlite3_exec(DB_INDEXES, "BEGIN TRANSACTION", NULL, NULL, &err_msg);

    //clock_t tic = clock();
    FILE *document;
    DIR *dir;
    struct dirent *in_file;

    dir = opendir("./documents");
    if (dir==NULL) {
        fprintf(stderr, "%s", "FAILED opening directory\n");
        return FAILED_READING;
    }
    while ((in_file=readdir(dir)) != NULL) {
        if (strcmp(in_file->d_name, "..") != 0 && strcmp(in_file->d_name, ".") != 0) {
            printf("READING %s\n", in_file->d_name);
            int result = add_file(in_file->d_name);
            //clock_t toc = clock();
            //printf("Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);
            if (result == FILE_ADDED)
            {
                printf("done reading %s\n", in_file->d_name);
            }
        }

    }
    sqlite3_exec(DB_INDEXES, "END TRANSACTION", NULL, NULL, &err_msg);
}

int check_documents(const char *Documents, char *target)
{
    char buf[WORD_BUFFER];
    int appears;
    char *ptr = (char *)Documents;
    char *start;
    while (*ptr != '\0') {
        if (*ptr == '(') {
            start = ptr+1;
        }
        else if (*ptr == ':') {
            if (strncmp(target, start, ptr-start) == 0)
                return 1;
        }
        ptr++;
    }
    return 0;
}
