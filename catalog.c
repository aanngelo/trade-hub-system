#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.h"
#include "utils.h"

// ============================================================
// catalog.c — Trade Hub System
// DATA STRUCTURE : Singly Linked List
// Manages all posted item listings.
// ALGORITHMS     : Linear Search, Binary Search,
//                 Bubble Sort, Selection Sort
// ============================================================

// init_catalog
// Initializes an empty catalog (linked list).
void init_catalog(Catalog *catalog)
{
    catalog->head = NULL;
    catalog->count = 0;
}

// post_item
// Creates a new Item node and inserts it at the HEAD
// of the linked list. O(1) insertion.
// Returns the new item's generated ID.
int post_item(Catalog *catalog, const char *title,
              const char *category, const char *condition,
              float price, const char *seller, const char *contact, int quantity)
{

    // Allocate memory for the new item
    Item *new_item = (Item *)malloc(sizeof(Item));
    if (!new_item)
    {
        printf("[ERROR] Memory allocation failed.\n");
        return -1;
    }

    // Fill in the item's data
    new_item->id = generate_id();
    strncpy(new_item->title, title, sizeof(new_item->title) - 1);
    strncpy(new_item->category, category, sizeof(new_item->category) - 1);
    strncpy(new_item->condition, condition, sizeof(new_item->condition) - 1);
    strncpy(new_item->seller, seller, sizeof(new_item->seller) - 1);
    strncpy(new_item->contact, contact, sizeof(new_item->contact) - 1);
    new_item->price = price;
    new_item->quantity = quantity;
    new_item->is_sold = 0;

    // Ensure null termination
    new_item->title[sizeof(new_item->title) - 1] = '\0';
    new_item->category[sizeof(new_item->category) - 1] = '\0';
    new_item->condition[sizeof(new_item->condition) - 1] = '\0';
    new_item->seller[sizeof(new_item->seller) - 1] = '\0';
    new_item->contact[sizeof(new_item->contact) - 1] = '\0';

    // Insert at head of linked list
    new_item->next = catalog->head;
    catalog->head = new_item;
    catalog->count++;

    printf("[SUCCESS] Item posted! ID: %d\n", new_item->id);
    return new_item->id;
}

int post_item_silent(Catalog *catalog, const char *title,
                     const char *category, const char *condition,
                     float price, const char *seller, const char *contact, int quantity)
{
    Item *new_item = (Item *)malloc(sizeof(Item));
    if (!new_item)
    {
        return -1;
    }

    new_item->id = generate_id();
    strncpy(new_item->title, title, sizeof(new_item->title) - 1);
    strncpy(new_item->category, category, sizeof(new_item->category) - 1);
    strncpy(new_item->condition, condition, sizeof(new_item->condition) - 1);
    strncpy(new_item->seller, seller, sizeof(new_item->seller) - 1);
    strncpy(new_item->contact, contact, sizeof(new_item->contact) - 1);
    new_item->price = price;
    new_item->quantity = quantity;
    new_item->is_sold = 0;

    new_item->title[sizeof(new_item->title) - 1] = '\0';
    new_item->category[sizeof(new_item->category) - 1] = '\0';
    new_item->condition[sizeof(new_item->condition) - 1] = '\0';
    new_item->seller[sizeof(new_item->seller) - 1] = '\0';
    new_item->contact[sizeof(new_item->contact) - 1] = '\0';

    new_item->next = catalog->head;
    catalog->head = new_item;
    catalog->count++;

    return new_item->id;
}

// remove_item
// Traverses the linked list to find and delete an item by ID.
// Returns 1 if found and removed, 0 if not found.
int remove_item(Catalog *catalog, int item_id)
{
    Item *current = catalog->head;
    Item *previous = NULL;

    // Traverse the list looking for the item
    while (current != NULL)
    {
        if (current->id == item_id)
        {
            // Bypass the node to remove it
            if (previous == NULL)
            {
                catalog->head = current->next; // removing head
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            catalog->count--;
            printf("[SUCCESS] Listing ID %d removed.\n", item_id);
            return 1;
        }
        previous = current;
        current = current->next;
    }

    printf("[ERROR] Item ID %d not found.\n", item_id);
    return 0;
}

// mark_sold
// Finds an item by ID and sets its is_sold flag to 1.
// Returns 1 on success, 0 if not found.
int mark_sold(Catalog *catalog, int item_id)
{
    Item *item = get_item_by_id(catalog, item_id);
    if (item)
    {
        item->is_sold = 1;
        printf("[SUCCESS] Item ID %d marked as SOLD.\n", item_id);
        return 1;
    }
    printf("[ERROR] Item ID %d not found.\n", item_id);
    return 0;
}

// display_all
// Traverses the entire linked list and prints all
// available (not sold) listings.
void display_all(Catalog *catalog)
{
    if (catalog->head == NULL)
    {
        printf("  No listings available.\n");
        return;
    }

    Item *current = catalog->head;
    int shown = 0;

    printf("======================================================================================\n");
    printf("                                       ALL LISTINGS\n");
    printf("======================================================================================\n");
    printf("  No.  ID      Product         Condition        Price        Seller            Contact\n");
    printf("--------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        if (!current->is_sold)
        {
            printf("  [%2d]  %-6d  %-15s %-15s PHP %-8.2f %-15s %s\n",
                   shown + 1,
                   current->id,
                   current->title,
                   current->condition,
                   current->price,
                   current->seller,
                   current->contact);
            shown++;
        }
        current = current->next;
    }

    printf("  ------------------------------------------------------------------------------\n");
    if (shown == 0)
        printf("  No available listings.\n");
    else
        printf("  %d listing(s) shown.\n", shown);
}

// display_by_category
// Traverses the list and prints only items matching category.
void display_by_category(Catalog *catalog, const char *category)
{
    Item *current = catalog->head;
    int found = 0;

    printf("======================================================================================\n");
    printf("                                  CATEGORY: %s\n", category);
    printf("======================================================================================\n");
    printf("  No.  ID      Product         Condition        Price        Seller            Contact\n");
    printf("  ------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        if (!current->is_sold &&
            strcasecmp(current->category, category) == 0)
        {
            printf("  [%2d]  %-6d  %-15s %-15s PHP %-8.2f %-15s %s\n",
                   found + 1, current->id, current->title,
                   current->condition, current->price,
                   current->seller, current->contact);
            found++;
        }
        current = current->next;
    }

    printf("  ------------------------------------------------------------------------------\n");
    if (!found)
        printf("  No items found under '%s'.\n", category);
    else
        printf("  %d item(s) found under '%s'.\n", found, category);
}

// display_by_seller
// Traverses the list and prints only items from a specific seller.
void display_by_seller(Catalog *catalog, const char *seller)
{
    Item *current = catalog->head;
    int found = 0;

    printf("======================================================================================\n");
    printf("                                    MY LISTING                                        \n");
    printf("======================================================================================\n");
    printf("  No.  ID      Product Name     Condition        Price        Stock    Status\n");
    printf("--------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        if (!current->is_sold &&
            strcasecmp(current->seller, seller) == 0)
        {
            char stock_str[20];
            sprintf(stock_str, "%d pcs", current->quantity);
            printf("  [%2d]  %-6d  %-15s %-15s PHP %-8.2f %-8s Active\n",
                   found + 1, current->id, current->title,
                   current->condition, current->price, stock_str);
            found++;
        }
        current = current->next;
    }

    printf("  ------------------------------------------------------------------------------\n");
    if (!found)
        printf("  No listings found.\n");
    else
        printf("  %d listing(s) shown.\n", found);
}

// get_item_by_id
// Linear traversal to find an item by ID.
// Returns pointer to item, or NULL if not found.
Item *get_item_by_id(Catalog *catalog, int item_id)
{
    Item *current = catalog->head;
    while (current != NULL)
    {
        if (current->id == item_id)
            return current;
        current = current->next;
    }
    return NULL;
}

// get_item_by_details
// Finds a catalog item by exact details when the ID is invalid
// or stale. This helps cart checkout recover missing IDs.
Item *get_item_by_details(Catalog *catalog, const char *title,
                          const char *condition, const char *seller,
                          const char *contact, float price)
{
    if (catalog == NULL || title == NULL || condition == NULL || seller == NULL || contact == NULL)
        return NULL;

    Item *current = catalog->head;
    while (current != NULL)
    {
        if (strcasecmp(current->title, title) == 0 &&
            strcasecmp(current->condition, condition) == 0 &&
            strcasecmp(current->seller, seller) == 0 &&
            strcasecmp(current->contact, contact) == 0 &&
            current->price == price)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// catalog_apply_purchase
// Reduces listed quantity after a sale; marks sold when stock hits 0.
int catalog_apply_purchase(Catalog *catalog, int item_id, int quantity)
{
    Item *it = get_item_by_id(catalog, item_id);
    if (it == NULL || it->is_sold || it->quantity < quantity)
        return 0;
    it->quantity -= quantity;
    if (it->quantity <= 0)
    {
        it->quantity = 0;
        it->is_sold = 1;
    }
    return 1;
}

// free_catalog
// Frees all dynamically allocated Item nodes.
// Always call before exiting the program.
void free_catalog(Catalog *catalog)
{
    Item *current = catalog->head;
    Item *next;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    catalog->head = NULL;
    catalog->count = 0;
}

// ==========================================================
// ALGORITHMS
// ==========================================================

// search_by_keyword  [ALGORITHM: Linear Search]
// Scans all item titles for a keyword using strstr().
// Case-insensitive. O(n) time complexity.
void search_by_keyword(Catalog *catalog, const char *keyword)
{
    Item *current = catalog->head;
    int found = 0;

    // Make a lowercase copy of the keyword for comparison
    char kw_lower[100];
    strncpy(kw_lower, keyword, sizeof(kw_lower) - 1);
    to_lowercase(kw_lower);

    printf("  Search results for \"%s\":\n\n", keyword);

    while (current != NULL)
    {
        // Make a lowercase copy of the title to compare
        char title_lower[100];
        strncpy(title_lower, current->title, sizeof(title_lower) - 1);
        to_lowercase(title_lower);

        if (!current->is_sold && strstr(title_lower, kw_lower))
        {
            printf("  [%d] %s â€” PHP %.2f (%s) | Seller: %s\n",
                   current->id, current->title, current->price,
                   current->condition, current->seller);
            found++;
        }
        current = current->next;
    }

    if (!found)
        printf("  No results found for \"%s\".\n", keyword);
    else
        printf("\n  %d result(s) found.\n", found);
}

// catalog_to_array
// Copies available item pointers into a flat array.
// Required before sorting or binary search.
// Returns the number of items copied (up to max_size).
int catalog_to_array(Catalog *catalog, Item **out_array, int max_size)
{
    int i = 0;
    Item *current = catalog->head;

    while (current != NULL && i < max_size)
    {
        if (!current->is_sold)
        {
            out_array[i++] = current;
        }
        current = current->next;
    }
    return i;
}

// sort_by_price  [ALGORITHM: Bubble Sort]
// Sorts an array of Item pointers by price.
// ascending = 1  â†’  low to high
// ascending = 0  â†’  high to low
// O(nÂ²) time complexity.
void sort_by_price(Item **items, int count, int ascending)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            int should_swap = ascending
                                  ? (items[j]->price > items[j + 1]->price)
                                  : (items[j]->price < items[j + 1]->price);

            if (should_swap)
            {
                // Swap pointers
                Item *temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

// sort_by_name  [ALGORITHM: Selection Sort]
// Sorts an array of Item pointers alphabetically by title.
// O(nÂ²) time complexity.
void sort_by_name(Item **items, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        int min_idx = i;

        // Find the item with the smallest title from i onward
        for (int j = i + 1; j < count; j++)
        {
            if (strcasecmp(items[j]->title, items[min_idx]->title) < 0)
            {
                min_idx = j;
            }
        }

        // Swap the found minimum with position i
        if (min_idx != i)
        {
            Item *temp = items[i];
            items[i] = items[min_idx];
            items[min_idx] = temp;
        }
    }
}

// binary_search_price  [ALGORITHM: Binary Search]
// Searches a SORTED array for a target price.
// Returns the index of a match, or -1 if not found.
// The array MUST be sorted by price first.
// O(log n) time complexity.
int binary_search_price(Item **sorted_items, int count, float target_price)
{
    int left = 0;
    int right = count - 1;
    const float tolerance = 0.05f;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        float diff = sorted_items[mid]->price - target_price;

        if (diff >= -tolerance && diff <= tolerance)
        {
            return mid; // match found within tolerance
        }
        else if (diff < 0)
        {
            left = mid + 1; // search right half
        }
        else
        {
            right = mid - 1; // search left half
        }
    }

    return -1; // not found
}

// search_by_exact_price
// Finds all available listings matching the target price.
void search_by_exact_price(Catalog *catalog, float target_price)
{
    Item *current = catalog->head;
    const float tolerance = 0.05f;
    int found = 0;

    printf("  Exact matches for PHP %.2f:\n\n", target_price);
    while (current != NULL)
    {
        if (!current->is_sold &&
            current->price >= target_price - tolerance &&
            current->price <= target_price + tolerance)
        {
            printf("  [%d] %s â€” PHP %.2f (%s) | Seller: %s | Contact: %s\n",
                   current->id, current->title,
                   current->price, current->condition,
                   current->seller, current->contact);
            found++;
        }
        current = current->next;
    }

    if (!found)
    {
        printf("  No listing found at exactly PHP %.2f.\n", target_price);
    }
    else
    {
        printf("\n  %d listing(s) found.\n", found);
    }
}

// filter_by_price_range
// Prints all available items within [min_price, max_price].
void filter_by_price_range(Catalog *catalog, float min_price, float max_price)
{
    Item *current = catalog->head;
    int found = 0;

    printf("  Items priced PHP %.2f â€” PHP %.2f:\n\n", min_price, max_price);

    while (current != NULL)
    {
        if (!current->is_sold &&
            current->price >= min_price &&
            current->price <= max_price)
        {
            printf("  [%d] %s â€” PHP %.2f | Seller: %s\n",
                   current->id, current->title,
                   current->price, current->seller);
            found++;
        }
        current = current->next;
    }

    if (!found)
        printf("  No items found in that price range.\n");
}
