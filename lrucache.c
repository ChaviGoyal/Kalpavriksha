#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 2003
#define MAX_VALUE_LEN 100

int REJECT_DUPLICATES = 0;

typedef struct Node {
    int key;
    char value[MAX_VALUE_LEN];
    struct Node *prev, *next;
} Node;

// HashMap Entry

typedef struct Entry {
    int key;
    Node *node;
    struct Entry *next;
} Entry;

// LRU Cache Structure

typedef struct {
    int capacity, size;
    Node *head, *tail;
    Entry *hashMap[HASH_SIZE];
} LRUCache;

// Hash Function

int hash(int key) {
    if (key < 0) key = -key;
    return key % HASH_SIZE;
}

void mapInsert(LRUCache *cache, int key, Node *node) {
    int h = hash(key);
    Entry *entry = (Entry*)malloc(sizeof(Entry));
    entry->key = key;
    entry->node = node;
    entry->next = cache->hashMap[h];
    cache->hashMap[h] = entry;
}

Node* mapGet(LRUCache *cache, int key) {
    int h = hash(key);
    Entry *cur = cache->hashMap[h];

    while (cur) {
        if (cur->key == key)
            return cur->node;
        cur = cur->next;
    }
    return NULL;
}

void mapDelete(LRUCache *cache, int key) {
    int h = hash(key);
    Entry *cur = cache->hashMap[h];
    Entry *prev = NULL;

    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else cache->hashMap[h] = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void addToHead(LRUCache *cache, Node *node) {
    node->prev = NULL;
    node->next = cache->head;

    if (cache->head)
        cache->head->prev = node;

    cache->head = node;

    if (cache->tail == NULL)
        cache->tail = node;
}

void removeNode(LRUCache *cache, Node *node) {
    if (node->prev)
        node->prev->next = node->next;
    else
        cache->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        cache->tail = node->prev;
}

void moveToHead(LRUCache *cache, Node *node) {
    removeNode(cache, node);
    addToHead(cache, node);
}

Node* removeTail(LRUCache *cache) {
    if (cache->tail == NULL) {
        printf("Error: Attempt to remove from an empty list.\n");
        return NULL;
    }
    Node *temp = cache->tail;
    removeNode(cache, temp);
    return temp;
}

LRUCache* createCache(int capacity) {
    if (capacity <= 0 || capacity > 1000) {
        printf("Invalid capacity. Must be 1–1000.\n");
        return NULL;
    }

    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;

    for (int i = 0; i < HASH_SIZE; i++)
        cache->hashMap[i] = NULL;

    return cache;
}

char* get(LRUCache *cache, int key) {
    if (!cache) return NULL;

    Node *node = mapGet(cache, key);
    if (!node)
        return NULL;

    moveToHead(cache, node);
    return node->value;
}

void put(LRUCache *cache, int key, char *value) {
    if (!cache) {
        printf("Cache not created.\n");
        return;
    }

    if (key < 0) {
        printf("Invalid key.\n");
        return;
    }

    if (strlen(value) == 0) {
        printf("Empty value not allowed.\n");
        return;
    }

    Node *node = mapGet(cache, key);

    // Reject duplicate keys 
    if (node && REJECT_DUPLICATES) {
        printf("Duplicate key not allowed.\n");
        return;
    }

    if (node) {
        strcpy(node->value, value);
        moveToHead(cache, node);
        return;
    }

    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->key = key;
    strcpy(newNode->value, value);
    newNode->prev = newNode->next = NULL;

    addToHead(cache, newNode);
    mapInsert(cache, key, newNode);
    cache->size++;

    if (cache->size > cache->capacity) {
        Node *lru = removeTail(cache);
        if (lru) {
            mapDelete(cache, lru->key);
            free(lru);
            cache->size--;
        }
    }
}

int main() {
    char command[50];
    LRUCache *cache = NULL;

    while (scanf("%s", command) != EOF) {

        if (strcmp(command, "createCache") == 0) {
            int cap;
            scanf("%d", &cap);
            cache = createCache(cap);
        }

        else if (strcmp(command, "put") == 0) {
            int key;
            char value[MAX_VALUE_LEN];
            scanf("%d %s", &key, value);
            put(cache, key, value);
        }

        else if (strcmp(command, "get") == 0) {
            int key;
            scanf("%d", &key);
            char *val = get(cache, key);
            printf("%s\n", val ? val : "NULL");
        }

        else if (strcmp(command, "exit") == 0) {
            break;
        }

        else {
            printf("Invalid command.\n");
        }
    }
    return 0;
}
