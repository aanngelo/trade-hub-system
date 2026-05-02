#ifndef HASH_H
#define HASH_H

/* ============================================================
   hash.h — Trade Hub System
   Hash Table: fast category index declarations (BONUS DSA)
   ============================================================ */

#define TABLE_SIZE  10      /* number of hash buckets         */
#define MAX_IDS    100      /* max items per category bucket  */

/* ----------------------------------------------------------
   HashNode — one category bucket in the hash table
   Stores a list of item IDs under a category name.
   Chaining is used to handle hash collisions.
   ---------------------------------------------------------- */
typedef struct HashNode {
    char  category[50];         /* e.g., "uniform", "supply"  */
    int   item_ids[MAX_IDS];    /* IDs of items in category   */
    int   count;                /* how many IDs stored        */
    struct HashNode *next;      /* chaining: next node in bucket */
} HashNode;

/* ----------------------------------------------------------
   HashTable — array of bucket pointers
   ---------------------------------------------------------- */
typedef struct HashTable {
    HashNode *buckets[TABLE_SIZE];
} HashTable;

/* --- Hash Table function declarations --- */
int       hash_category(const char *category);
void      init_hash_table(HashTable *ht);
void      ht_insert(HashTable *ht, const char *category, int item_id);
HashNode *ht_lookup(HashTable *ht, const char *category);
void      ht_remove(HashTable *ht, const char *category, int item_id);
void      display_hash_table(HashTable *ht);
void      free_hash_table(HashTable *ht);

#endif /* HASH_H */
