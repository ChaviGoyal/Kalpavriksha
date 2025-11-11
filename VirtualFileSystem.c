#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE 512
#define MAX_NAME   50

// Flattened virtual disk
unsigned char *diskMemory = NULL;
int TOTAL_BLOCKS = 1024;

// Free block node
typedef struct FreeBlockNode {
    int index;
    struct FreeBlockNode *prev;
    struct FreeBlockNode *next;
} FreeBlockNode;

FreeBlockNode *freeListHead = NULL;
FreeBlockNode *freeListTail = NULL;
int freeBlockCount = 0;

// Basic file/directory node
typedef struct FSNode {
    char name[MAX_NAME+1];
    int isDirectory;

    struct FSNode *parent;
    struct FSNode *child;      // first child (circular list)
    struct FSNode *nextSibling;
    struct FSNode *prevSibling;

    // file data
    int *blockIndex;
    int blockCount;
    int contentBytes;
} FSNode;

FSNode *rootDir = NULL;
FSNode *currentDir = NULL;

/* ------------------------ Free block list ------------------------ */

void init_free_blocks() {
    freeListHead = freeListTail = NULL;
    freeBlockCount = 0;

    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        FreeBlockNode *n = malloc(sizeof(FreeBlockNode));
        n->index = i;
        n->next = NULL;
        n->prev = freeListTail;

        if (!freeListHead) freeListHead = n;
        if (freeListTail) freeListTail->next = n;
        freeListTail = n;

        freeBlockCount++;
    }
}

int pop_free_block() {
    if (!freeListHead) return -1;
    FreeBlockNode *temp = freeListHead;
    int idx = temp->index;

    freeListHead = temp->next;
    if (freeListHead) freeListHead->prev = NULL;
    else freeListTail = NULL;
    free(temp);

    freeBlockCount--;
    return idx;
}

// append freed block to tail
void push_free_block(int idx) {
    FreeBlockNode *n = malloc(sizeof(FreeBlockNode));
    n->index = idx;
    n->next = NULL;
    n->prev = freeListTail;

    if (freeListTail) freeListTail->next = n;
    freeListTail = n;
    if (!freeListHead) freeListHead = n;

    freeBlockCount++;
}

void free_entire_free_list() {
    FreeBlockNode *t = freeListHead;
    while (t) {
        FreeBlockNode *nx = t->next;
        free(t);
        t = nx;
    }
}

/* ------------------------ File node helpers ------------------------ */

FSNode *new_node(const char *name, int isDir) {
    FSNode *n = malloc(sizeof(FSNode));
    strncpy(n->name, name, MAX_NAME);
    n->name[MAX_NAME] = '\0';

    n->isDirectory = isDir;
    n->parent = NULL;
    n->child = NULL;
    n->nextSibling = n->prevSibling = NULL;

    n->blockIndex = NULL;
    n->blockCount = 0;
    n->contentBytes = 0;
    return n;
}

void add_child(FSNode *parent, FSNode *child) {
    child->parent = parent;
    if (!parent->child) {
        parent->child = child;
        child->nextSibling = child->prevSibling = child;
    } else {
        FSNode *first = parent->child;
        FSNode *last = first->prevSibling;
        last->nextSibling = child;
        child->prevSibling = last;
        child->nextSibling = first;
        first->prevSibling = child;
    }
}

// Remove from circular sibling list
void detach_child(FSNode *parent, FSNode *child) {
    if (!parent->child) return;

    if (child->nextSibling == child && child->prevSibling == child) {
        parent->child = NULL;
    } else {
        child->prevSibling->nextSibling = child->nextSibling;
        child->nextSibling->prevSibling = child->prevSibling;
        if (parent->child == child)
            parent->child = child->nextSibling;
    }
    child->nextSibling = child->prevSibling = NULL;
    child->parent = NULL;
}

// search in cwd
FSNode *find_in_current(const char *name) {
    if (!currentDir->child) return NULL;
    FSNode *c = currentDir->child;
    do {
        if (strcmp(c->name, name) == 0) return c;
        c = c->nextSibling;
    } while (c != currentDir->child);

    return NULL;
}

/* ------------------------ Commands ------------------------ */

// mkdir
void do_mkdir(char *name) {
    if (!name) {
        printf("Usage: mkdir <name>\n");
        return;
    }
    if (find_in_current(name)) {
        printf("Already exists.\n");
        return;
    }
    FSNode *d = new_node(name, 1);
    add_child(currentDir, d);
    printf("Directory '%s' created.\n", name);
}

void do_create(char *name) {
    if (!name) {
        printf("Usage: create <name>\n");
        return;
    }
    if (find_in_current(name)) {
        printf("Already exists.\n");
        return;
    }
    FSNode *f = new_node(name, 0);
    add_child(currentDir, f);
    printf("File '%s' created.\n", name);
}

// list
void do_ls() {
    if (!currentDir->child) {
        printf("(empty)\n");
        return;
    }
    FSNode *c = currentDir->child;
    do {
        printf("%s%s\n", c->name, c->isDirectory ? "/" : "");
        c = c->nextSibling;
    } while (c != currentDir->child);
}

// write to file (overwrite)
void write_file_data(FSNode *file, const char *data) {
    int len = strlen(data);
    int needed = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (needed > freeBlockCount) {
        printf("Disk full.\n");
        return;
    }

    // free old
    for (int i = 0; i < file->blockCount; i++)
        push_free_block(file->blockIndex[i]);

    free(file->blockIndex);

    if (len == 0) {
        file->blockIndex = NULL;
        file->blockCount = 0;
        file->contentBytes = 0;
        printf("(empty data)\n");
        return;
    }

    file->blockIndex = malloc(sizeof(int) * needed);
    file->blockCount = needed;
    file->contentBytes = len;

    int w = 0;
    for (int i = 0; i < needed; i++) {
        int idx = pop_free_block();
        file->blockIndex[i] = idx;

        int chunk = BLOCK_SIZE;
        if (w + chunk > len) chunk = len - w;
        memcpy(diskMemory + idx * BLOCK_SIZE, data + w, chunk);

        if (chunk < BLOCK_SIZE)
            memset(diskMemory + idx * BLOCK_SIZE + chunk, 0, BLOCK_SIZE - chunk);

        w += chunk;
    }

    printf("Written %d bytes.\n", len);
}

void do_write(char *filename, char *text) {
    FSNode *f = find_in_current(filename);
    if (!f) {
        printf("File not found.\n");
        return;
    }
    if (f->isDirectory) {
        printf("Can't write to directory.\n");
        return;
    }
    write_file_data(f, text);
}

void do_read(char *name) {
    FSNode *f = find_in_current(name);
    if (!f) {
        printf("Not found.\n");
        return;
    }
    if (f->isDirectory) {
        printf("'%s' is a directory.\n", name);
        return;
    }
    if (f->blockCount == 0) {
        printf("(empty)\n");
        return;
    }

    int remain = f->contentBytes;
    for (int i = 0; i < f->blockCount; i++) {
        int idx = f->blockIndex[i];
        int chunk = remain < BLOCK_SIZE ? remain : BLOCK_SIZE;
        fwrite(diskMemory + idx * BLOCK_SIZE, 1, chunk, stdout);
        remain -= chunk;
    }
    printf("\n");
}

// delete file
void do_delete(char *name) {
    FSNode *f = find_in_current(name);
    if (!f) {
        printf("Not found.\n");
        return;
    }
    if (f->isDirectory) {
        printf("Use rmdir.\n");
        return;
    }
    for (int i = 0; i < f->blockCount; i++)
        push_free_block(f->blockIndex[i]);

    free(f->blockIndex);
    detach_child(currentDir, f);
    free(f);
    printf("File removed.\n");
}

void do_rmdir(char *name) {
    FSNode *d = find_in_current(name);
    if (!d) {
        printf("Not found.\n");
        return;
    }
    if (!d->isDirectory) {
        printf("Not a directory.\n");
        return;
    }
    if (d->child) {
        printf("Directory not empty.\n");
        return;
    }
    detach_child(currentDir, d);
    free(d);
    printf("Removed dir.\n");
}

// cd
void do_cd(char *name) {
    if (!name) {
        printf("Usage: cd <dir>\n");
        return;
    }
    if (strcmp(name, "/") == 0) {
        currentDir = rootDir;
        printf("Moved to /\n");
        return;
    }
    if (strcmp(name, "..") == 0) {
        if (currentDir->parent)
            currentDir = currentDir->parent;

        // TODO: maybe simplify this path printing
        FSNode *t = currentDir;
        char p[4096] = "";
        while (t && t != rootDir) {
            char buf[256];
            snprintf(buf, sizeof(buf), "/%s", t->name);
            char tmp[4096];
            strcpy(tmp, buf);
            strcat(tmp, p);
            strcpy(p, tmp);
            t = t->parent;
        }
        if (strlen(p) == 0) strcpy(p, "/");
        printf("Moved to %s\n", p);
        return;
    }

    FSNode *d = find_in_current(name);
    if (!d) {
        printf("Not found.\n");
        return;
    }
    if (!d->isDirectory) {
        printf("Not a dir.\n");
        return;
    }
    currentDir = d;

    char p[4096] = "";
    FSNode *t = currentDir;
    while (t && t != rootDir) {
        char buf[256];
        snprintf(buf, sizeof(buf), "/%s", t->name);
        char tmp[4096];
        strcpy(tmp, buf);
        strcat(tmp, p);
        strcpy(p, tmp);
        t = t->parent;
    }
    if (strlen(p) == 0) strcpy(p, "/");
    printf("Moved to %s\n", p);
}

void do_pwd() {
    FSNode *t = currentDir;
    if (t == rootDir) {
        printf("/\n");
        return;
    }
    char p[4096] = "";
    while (t && t != rootDir) {
        char buf[256];
        snprintf(buf, sizeof(buf), "/%s", t->name);
        char tmp[4096];
        strcpy(tmp, buf);
        strcat(tmp, p);
        strcpy(p, tmp);
        t = t->parent;
    }
    if (strlen(p) == 0) strcpy(p, "/");
    printf("%s\n", p);
}

void do_df() {
    int used = TOTAL_BLOCKS - freeBlockCount;
    double usage = (double)used / TOTAL_BLOCKS * 100.0;
    printf("Total: %d\nUsed: %d\nFree: %d\nUsage: %.2f%%\n",
            TOTAL_BLOCKS, used, freeBlockCount, usage);
}

// free everything (called on exit)
void free_recursive(FSNode *n) {
    if (!n) return;
    if (n->child) {
        FSNode *c = n->child;
        do {
            FSNode *nx = c->nextSibling;
            free_recursive(c);
            c = nx;
        } while (c != n->child);
    }
    if (!n->isDirectory) {
        for (int i = 0; i < n->blockCount; i++)
            push_free_block(n->blockIndex[i]);
        free(n->blockIndex);
    }
    free(n);
}

void do_exit() {
    if (rootDir->child) {
        FSNode *c = rootDir->child;
        do {
            FSNode *nx = c->nextSibling;
            free_recursive(c);
            c = nx;
        } while (c != rootDir->child);
    }
    free(rootDir);
    free_entire_free_list();
    free(diskMemory);
    printf("Goodbye.\n");
    exit(0);
}

/* ---------------- Parsing helpers ---------------- */

char *remove_quotes(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len-1] == '"') ||
                    (s[0] == '\'' && s[len-1] == '\''))) {
        s[len-1] = '\0';
        return s+1;
    }
    return s;
}

void handle_input(char *line) {
    while (*line && isspace((unsigned char)*line)) line++;
    if (*line == '\0') return;

    char backup[8192];
    strncpy(backup, line, sizeof(backup)-1);
    backup[sizeof(backup)-1] = '\0';

    char *cmd = strtok(backup, " \t\n");
    if (!cmd) return;

    if (strcmp(cmd, "mkdir") == 0) {
        do_mkdir(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "create") == 0) {
        do_create(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "ls") == 0) {
        do_ls();
    }
    else if (strcmp(cmd, "write") == 0) {
        char *p = line;
        while (*p && isspace(*p)) p++;
        p += strlen("write");
        while (*p && isspace(*p)) p++;

        char filename[256];
        int i = 0;
        while (*p && !isspace(*p) && i < 255)
            filename[i++] = *p++;
        filename[i] = '\0';

        while (*p && isspace(*p)) p++;

        char content[8192];
        strncpy(content, p, sizeof(content)-1);
        content[sizeof(content)-1] = '\0';

        size_t L = strlen(content);
        if (L > 0 && content[L-1] == '\n')
            content[L-1] = '\0';

        char *c = remove_quotes(content);

        // interpret only \n
        char parsed[8192];
        int pi = 0;
        for (int k = 0; k < strlen(c); k++) {
            if (c[k] == '\\' && c[k+1] == 'n') {
                parsed[pi++] = '\n';
                k++;
            } else parsed[pi++] = c[k];
        }
        parsed[pi] = '\0';

        do_write(filename, parsed);
    }
    else if (strcmp(cmd, "read") == 0) {
        do_read(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "delete") == 0) {
        do_delete(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "rmdir") == 0) {
        do_rmdir(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "cd") == 0) {
        do_cd(strtok(NULL, " \t\n"));
    }
    else if (strcmp(cmd, "pwd") == 0) {
        do_pwd();
    }
    else if (strcmp(cmd, "df") == 0) {
        do_df();
    }
    else if (strcmp(cmd, "exit") == 0) {
        do_exit();
    }
    else {
        printf("Unknown command: %s\n", cmd);
    }
}

void init_vfs(int blocks) {
    if (blocks < 1) blocks = 1;
    if (blocks > 5000) blocks = 5000;

    TOTAL_BLOCKS = blocks;
    diskMemory = malloc((size_t)TOTAL_BLOCKS * BLOCK_SIZE);
    memset(diskMemory, 0, (size_t)TOTAL_BLOCKS * BLOCK_SIZE);

    init_free_blocks();
    rootDir = new_node("/", 1);
    currentDir = rootDir;
}

/* ---------------- main ---------------- */

int main(int argc, char **argv) {
    int blocks = 1024;
    if (argc >= 2) {
        int n = atoi(argv[1]);
        if (n > 0) blocks = n;
    }
    init_vfs(blocks);

    printf("VFS ready. Type 'exit' to quit.\n");

    char prompt[512];
    char line[8192];

    while (1) {
        if (currentDir == rootDir) strcpy(prompt, "/ > ");
        else {
            FSNode *t = currentDir;
            char tmp[4096] = "";
            while (t && t != rootDir) {
                char buf[256];
                snprintf(buf, sizeof(buf), "/%s", t->name);
                char join[4096];
                strcpy(join, buf);
                strcat(join, tmp);
                strcpy(tmp, join);
                t = t->parent;
            }
            if (strlen(tmp) == 0) strcpy(tmp, "/");
            snprintf(prompt, sizeof(prompt), "%s > ", tmp);
        }

        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            do_exit();
        }
        handle_input(line);
    }
    return 0;
}
