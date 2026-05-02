#ifndef CATALOG_H
#define CATALOG_H

/* ============================================================
   catalog.h — Trade Hub System
   Linked List: item listing catalog declarations
   ============================================================ */

/* ----------------------------------------------------------
   Item — a single listing node in the linked list
   ---------------------------------------------------------- */
typedef struct Item {
    int   id;               /* unique listing ID               */
    char  title[100];       /* e.g., "PE Uniform Large"        */
    char  category[50];     /* "uniform" | "supply" | "book"   */
    char  condition[20];    /* "new" | "used" | "worn"         */
    float price;            /* price in Philippine Peso (PHP)  */
    char  seller[50];       /* seller's name                   */
    char  contact[50];      /* contact number / messenger      */
    int   quantity;         /* quantity available in stock     */
    int   is_sold;          /* 0 = available, 1 = sold         */
    struct Item *next;      /* pointer to the next item node   */
} Item;

/* ----------------------------------------------------------
   Catalog — the linked list that holds all item listings
   ---------------------------------------------------------- */
typedef struct Catalog {
    Item *head;             /* pointer to the first item       */
    int   count;            /* total number of listings        */
} Catalog;

/* --- Catalog function declarations --- */
void  init_catalog(Catalog *catalog);
int   post_item(Catalog *catalog, const char *title,
                const char *category, const char *condition,
                float price, const char *seller, const char *contact, int quantity);
int   post_item_silent(Catalog *catalog, const char *title,
                const char *category, const char *condition,
                float price, const char *seller, const char *contact, int quantity);
int   remove_item(Catalog *catalog, int item_id);
int   mark_sold(Catalog *catalog, int item_id);
void  display_all(Catalog *catalog);
void  display_by_category(Catalog *catalog, const char *category);
void  display_by_seller(Catalog *catalog, const char *seller);
Item *get_item_by_id(Catalog *catalog, int item_id);
Item *get_item_by_details(Catalog *catalog, const char *title,
                          const char *condition, const char *seller,
                          const char *contact, float price);
int   catalog_apply_purchase(Catalog *catalog, int item_id, int quantity);
void  free_catalog(Catalog *catalog);

/* --- Search & Sort declarations --- */
void search_by_keyword(Catalog *catalog, const char *keyword);
void search_by_exact_price(Catalog *catalog, float target_price);
int  binary_search_price(Item **sorted_items, int count, float target_price);
int  catalog_to_array(Catalog *catalog, Item **out_array, int max_size);
void sort_by_price(Item **items, int count, int ascending);
void sort_by_name(Item **items, int count);
void filter_by_price_range(Catalog *catalog, float min_price, float max_price);

#endif /* CATALOG_H */
