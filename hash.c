#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hash.h"

/* ============================================================
   hash.c — Trade Hub System
   DATA STRUCTURE : Hash Table (with chaining)
   Provides O(1) average-case category lookups.
   WHY HASH TABLE: Faster than scanning the full linked list
                   when browsing a specific category.
   ============================================================ */

/* ----------------------------------------------------------
   hash_category  [HASH FUNCTION]
   Maps a category string to a bucket index using djb2.
   Always returns a value in range [0, TABLE_SIZE - 1].
   ---------------------------------------------------------- */
int hash_category(const char *category) {
    unsigned long hash = 5381;
    int c;

    /* djb2 algorithm: hash = hash * 33 + char */
    while ((c = tolower((unsigned char)*category++))) {
        hash = ((hash << 5) + hash) + c;
    }

    return (int)(hash % TABLE_SIZE);
}

/* ----------------------------------------------------------
   init_hash_table
   Sets all bucket pointers to NULL (empty table).
   ---------------------------------------------------------- */
void init_hash_table(HashTable *ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
}

/* ----------------------------------------------------------
   ht_insert
   Indexes an item ID under its category.
   If the category already exists in the bucket, adds the ID.
   If not, creates a new HashNode (chaining for collisions).
   ---------------------------------------------------------- */
void ht_insert(HashTable *ht, const char *category, int item_id) {
    int index = hash_category(category);

    /* Search for existing category node in this bucket */
    HashNode *current = ht->buckets[index];
    while (current != NULL) {
        if (strcasecmp(current->category, category) == 0) {
            /* Category found — add item ID if space allows */
            if (current->count < MAX_IDS) {
                current->item_ids[current->count++] = item_id;
            } else {
                printf("[WARN] Category '%s' bucket is full.\n", category);
            }
            return;
        }
        current = current->next;
    }

    /* Category not found — create a new HashNode */
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
    if (!new_node) {
        printf("[ERROR] Memory allocation failed.\n");
        return;
    }

    strncpy(new_node->category, category, sizeof(new_node->category) - 1);
    new_node->item_ids[0] = item_id;
    new_node->count       = 1;

    /* Chain it at the front of this bucket */
    new_node->next         = ht->buckets[index];
    ht->buckets[index]     = new_node;
}

/* ----------------------------------------------------------
   ht_lookup
   Returns the HashNode for a category — O(1) average.
   Returns NULL if the category does not exist.
   ---------------------------------------------------------- */
HashNode *ht_lookup(HashTable *ht, const char *category) {
    int       index   = hash_category(category);
    HashNode *current = ht->buckets[index];

    while (current != NULL) {
        if (strcasecmp(current->category, category) == 0) {
            return current; /* found */
        }
        current = current->next;
    }

    return NULL; /* not found */
}

/* ----------------------------------------------------------
   ht_remove
   Removes a specific item ID from its category bucket.
   Shifts remaining IDs to fill the gap.
   ---------------------------------------------------------- */
void ht_remove(HashTable *ht, const char *category, int item_id) {
    HashNode *node = ht_lookup(ht, category);
    if (!node) return;

    /* Find and remove the item ID from the array */
    for (int i = 0; i < node->count; i++) {
        if (node->item_ids[i] == item_id) {
            /* Shift remaining IDs left */
            for (int j = i; j < node->count - 1; j++) {
                node->item_ids[j] = node->item_ids[j + 1];
            }
            node->count--;
            return;
        }
    }
}

/* ----------------------------------------------------------
   display_hash_table
   Prints all categories and how many items are indexed.
   ---------------------------------------------------------- */
void display_hash_table(HashTable *ht) {
    printf("================================================================================\n");
    printf("|                                 Category Index                               |\n");
    printf("================================================================================\n");
    printf("  No.   Category             Total Listings     Description\n");
    printf("  ------------------------------------------------------------------------------\n");

    int category_num = 1;
    int total_items = 0;

    /* Define category descriptions */
    const char *descriptions[] = {
        "Blouse, Skirt, Pants, PE Uniform",
        "Notebooks, Pens, Art Materials",
        "Textbooks, Reviewers, Novels",
        "Miscellaneous Items"
    };
    
    const char *category_names[] = {
        "Uniform",
        "Supply",
        "Books",
        "Other"
    };

    /* Display all categories */
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL) {
            int count = current->count;
            total_items += count;
            
            /* Find the description for this category */
            const char *desc = "N/A";
            if (strcmp(current->category, "uniform") == 0)
                desc = descriptions[0];
            else if (strcmp(current->category, "supply") == 0)
                desc = descriptions[1];
            else if (strcmp(current->category, "books") == 0)
                desc = descriptions[2];
            else if (strcmp(current->category, "other") == 0)
                desc = descriptions[3];
            
            printf("  [%-2d] %-20s%d item(s)          %s\n",
                   category_num, current->category, count, desc);
            category_num++;
            current = current->next;
        }
    }

    printf("  ------------------------------------------------------------------------------\n");
    printf("         Total Categories: %-2d              Total Listings: %d item(s)         \n", 
           category_num - 1, total_items);
    printf("================================================================================\n");
}

/* ----------------------------------------------------------
   free_hash_table
   Frees all HashNode allocations in the table.
   ---------------------------------------------------------- */
void free_hash_table(HashTable *ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        HashNode *next;
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
        ht->buckets[i] = NULL;
    }
}
