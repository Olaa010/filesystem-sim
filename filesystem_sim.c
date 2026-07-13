/* ============================================================
 * Simple File System Simulation
 * ------------------------------------------------------------
 * A menu-driven C program that simulates basic file system
 * operations (create, write, read, delete, rename, list) by
 * combining an in-memory record of "known" files (a struct
 * array) with real file handling calls (fopen, fclose, remove,
 * rename, fread, fwrite) so that the operations actually touch
 * files on disk in the current working directory.
 *
 * Skills demonstrated: File I/O, structs, pointers, error
 * handling, dynamic memory allocation.
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_FILES     100
#define MAX_NAME_LEN  64
#define MAX_LINE_LEN  1024

/* ---------------- Data Structures ---------------- */

typedef struct {
    char name[MAX_NAME_LEN];   /* name of the file as tracked by our "file system" */
    int  inUse;                /* 1 if this slot holds a valid record, 0 otherwise */
} FileRecord;

typedef struct {
    FileRecord files[MAX_FILES];
    int count;                 /* number of files currently tracked */
} FileSystem;

/* ---------------- Function Prototypes ---------------- */

void initFileSystem(FileSystem *fs);
int  findFile(const FileSystem *fs, const char *name);
int  isValidFilename(const char *name);
void readLine(char *buffer, int size);

void createFileOp(FileSystem *fs);
void writeFileOp(FileSystem *fs);
void readFileOp(const FileSystem *fs);
void deleteFileOp(FileSystem *fs);
void listFilesOp(const FileSystem *fs);
void renameFileOp(FileSystem *fs);

int  getMenuChoice(void);
void printMenu(void);

/* ---------------- main ---------------- */

int main(void) {
    FileSystem fs;
    initFileSystem(&fs);

    int choice;
    do {
        printMenu();
        choice = getMenuChoice();

        switch (choice) {
            case 1: createFileOp(&fs);  break;
            case 2: writeFileOp(&fs);   break;
            case 3: readFileOp(&fs);    break;
            case 4: deleteFileOp(&fs);  break;
            case 5: listFilesOp(&fs);   break;
            case 6: renameFileOp(&fs);  break;
            case 7:
                printf("\nExiting file system simulation. Goodbye!\n");
                break;
            default:
                printf("\nError: Invalid choice. Please enter a number between 1 and 7.\n");
        }
    } while (choice != 7);

    return 0;
}

/* ---------------- Setup / Utility ---------------- */

void initFileSystem(FileSystem *fs) {
    fs->count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        fs->files[i].inUse = 0;
        fs->files[i].name[0] = '\0';
    }
}

void printMenu(void) {
    printf("\n===== Simple File System Simulation =====\n");
    printf("1. Create File\n");
    printf("2. Write to File\n");
    printf("3. Read File\n");
    printf("4. Delete File\n");
    printf("5. List Files\n");
    printf("6. Rename File\n");
    printf("7. Exit\n");
    printf("==========================================\n");
}

/* Reads a single menu choice safely. Returns -1 on bad/non-numeric input
 * instead of crashing or looping forever, so main()'s switch can report
 * a clean error message. */
int getMenuChoice(void) {
    char buffer[32];
    int choice;

    printf("Enter your choice: ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        /* stdin closed (EOF) - treat as a request to quit rather than
         * looping forever on repeated "invalid choice" errors. */
        printf("\nEnd of input detected. Exiting.\n");
        return 7;
    }
    if (sscanf(buffer, "%d", &choice) != 1) {
        return -1;
    }
    return choice;
}

/* Reads a line of input into buffer, strips the trailing newline,
 * and guards against overflow / empty stdin. */
void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        /* Input longer than buffer: flush the rest of the line. */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
}

/* Rejects empty names, names that are too long, and names containing
 * path separators (to keep every "file" inside the current directory). */
int isValidFilename(const char *name) {
    size_t len = strlen(name);
    if (len == 0) {
        printf("Error: File name cannot be empty.\n");
        return 0;
    }
    if (len >= MAX_NAME_LEN) {
        printf("Error: File name is too long (max %d characters).\n", MAX_NAME_LEN - 1);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (name[i] == '/' || name[i] == '\\') {
            printf("Error: File name cannot contain '/' or '\\\\'.\n");
            return 0;
        }
    }
    return 1;
}

/* Returns the index of "name" in fs->files, or -1 if not tracked. */
int findFile(const FileSystem *fs, const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->files[i].inUse && strcmp(fs->files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* ---------------- File Operations ---------------- */

void createFileOp(FileSystem *fs) {
    char name[MAX_NAME_LEN];

    printf("\n-- Create File --\n");
    printf("Enter file name: ");
    readLine(name, sizeof(name));

    if (!isValidFilename(name)) {
        return;
    }

    if (findFile(fs, name) != -1) {
        printf("Error: A file named \"%s\" already exists.\n", name);
        return;
    }

    if (fs->count >= MAX_FILES) {
        printf("Error: File system is full (limit of %d files).\n", MAX_FILES);
        return;
    }

    FILE *fp = fopen(name, "w");
    if (fp == NULL) {
        printf("Error: Could not create \"%s\" (%s).\n", name, strerror(errno));
        return;
    }
    fclose(fp);

    /* Record it in an empty slot. */
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fs->files[i].inUse) {
            strcpy(fs->files[i].name, name);
            fs->files[i].inUse = 1;
            fs->count++;
            break;
        }
    }

    printf("File \"%s\" created successfully.\n", name);
}

void writeFileOp(FileSystem *fs) {
    char name[MAX_NAME_LEN];
    char line[MAX_LINE_LEN];

    printf("\n-- Write to File --\n");
    printf("Enter file name: ");
    readLine(name, sizeof(name));

    if (findFile(fs, name) == -1) {
        printf("Error: File \"%s\" does not exist. Create it first (option 1).\n", name);
        return;
    }

    printf("Choose mode: 1) Overwrite  2) Append: ");
    char modeInput[8];
    readLine(modeInput, sizeof(modeInput));
    int mode = atoi(modeInput);
    const char *fopenMode = (mode == 2) ? "a" : "w";

    printf("Enter the text to write (single line): ");
    readLine(line, sizeof(line));

    FILE *fp = fopen(name, fopenMode);
    if (fp == NULL) {
        printf("Error: Could not open \"%s\" for writing (%s).\n", name, strerror(errno));
        return;
    }

    if (fprintf(fp, "%s\n", line) < 0) {
        printf("Error: Failed to write to \"%s\".\n", name);
        fclose(fp);
        return;
    }

    fclose(fp);
    printf("Data written to \"%s\" successfully.\n", name);
}

void readFileOp(const FileSystem *fs) {
    char name[MAX_NAME_LEN];

    printf("\n-- Read File --\n");
    printf("Enter file name: ");
    readLine(name, sizeof(name));

    if (findFile(fs, name) == -1) {
        printf("Error: File \"%s\" is not tracked by this file system.\n", name);
        return;
    }

    FILE *fp = fopen(name, "r");
    if (fp == NULL) {
        printf("Error: Could not open \"%s\" for reading (%s).\n", name, strerror(errno));
        return;
    }

    /* Determine file size, then read it all into a dynamically
     * allocated buffer via a pointer, demonstrating malloc/free. */
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        printf("(File \"%s\" is empty.)\n", name);
        fclose(fp);
        return;
    }

    char *buffer = (char *) malloc((size_t) size + 1);
    if (buffer == NULL) {
        printf("Error: Out of memory while reading \"%s\".\n", name);
        fclose(fp);
        return;
    }

    size_t bytesRead = fread(buffer, 1, (size_t) size, fp);
    buffer[bytesRead] = '\0';
    fclose(fp);

    printf("\n----- Contents of \"%s\" -----\n%s", name, buffer);
    printf("----- End of file -----\n");

    free(buffer);
}

void deleteFileOp(FileSystem *fs) {
    char name[MAX_NAME_LEN];

    printf("\n-- Delete File --\n");
    printf("Enter file name: ");
    readLine(name, sizeof(name));

    int idx = findFile(fs, name);
    if (idx == -1) {
        printf("Error: File \"%s\" does not exist.\n", name);
        return;
    }

    if (remove(name) != 0) {
        printf("Error: Could not delete \"%s\" (%s).\n", name, strerror(errno));
        return;
    }

    fs->files[idx].inUse = 0;
    fs->files[idx].name[0] = '\0';
    fs->count--;

    printf("File \"%s\" deleted successfully.\n", name);
}

void listFilesOp(const FileSystem *fs) {
    printf("\n-- Files in the System --\n");

    if (fs->count == 0) {
        printf("(No files created yet.)\n");
        return;
    }

    int shown = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->files[i].inUse) {
            long size = -1;
            FILE *fp = fopen(fs->files[i].name, "r");
            if (fp != NULL) {
                fseek(fp, 0, SEEK_END);
                size = ftell(fp);
                fclose(fp);
            }

            if (size >= 0) {
                printf("  %2d. %-30s (%ld bytes)\n", ++shown, fs->files[i].name, size);
            } else {
                printf("  %2d. %-30s (size unavailable)\n", ++shown, fs->files[i].name);
            }
        }
    }
    printf("Total files: %d\n", fs->count);
}

void renameFileOp(FileSystem *fs) {
    char oldName[MAX_NAME_LEN];
    char newName[MAX_NAME_LEN];

    printf("\n-- Rename File --\n");
    printf("Enter current file name: ");
    readLine(oldName, sizeof(oldName));

    int idx = findFile(fs, oldName);
    if (idx == -1) {
        printf("Error: File \"%s\" does not exist.\n", oldName);
        return;
    }

    printf("Enter new file name: ");
    readLine(newName, sizeof(newName));

    if (!isValidFilename(newName)) {
        return;
    }

    if (findFile(fs, newName) != -1) {
        printf("Error: A file named \"%s\" already exists.\n", newName);
        return;
    }

    if (rename(oldName, newName) != 0) {
        printf("Error: Could not rename \"%s\" to \"%s\" (%s).\n",
               oldName, newName, strerror(errno));
        return;
    }

    strcpy(fs->files[idx].name, newName);
    printf("File \"%s\" renamed to \"%s\" successfully.\n", oldName, newName);
}
