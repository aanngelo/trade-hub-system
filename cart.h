#ifndef CART_H
#define CART_H

#include "stack.h"
#include "catalog.h"

/* ============================================================
   cart.h — Trade Hub System
   Cart: shopping cart for selected items declarations
   ============================================================ */

/* ----------------------------------------------------------
   CartItem — a single item in the shopping cart
   ---------------------------------------------------------- */
typedef struct CartItem
{
   int item_id;             /* ID of the selected item         */
   char item_title[100];    /* title of the selected item      */
   char condition[20];      /* condition of the item           */
   char seller[50];         /* seller's name                   */
   char contact[50];        /* seller's contact                */
   char buyer_username[50]; /* buyer's username               */
   float price;             /* price of the item               */
   int quantity;            /* quantity selected (usually 1)   */
   struct CartItem *next;   /* pointer to the next cart item   */
} CartItem;

/* ----------------------------------------------------------
   ShoppingCart — linked list of selected items
   ---------------------------------------------------------- */
typedef struct ShoppingCart
{
   CartItem *head; /* pointer to the first cart item  */
   int count;      /* total number of items in cart   */
   float total;    /* total price of all items        */
} ShoppingCart;

/* --- Cart function declarations --- */
void init_cart(ShoppingCart *cart);
int add_to_cart(ShoppingCart *cart, int item_id,
                const char *item_title, const char *condition, const char *seller,
                const char *contact, const char *buyer_username,
                float price, int quantity);
int remove_from_cart(ShoppingCart *cart, int item_id);
int update_cart_item_quantity(ShoppingCart *cart, int item_id, int new_quantity);
void display_cart(ShoppingCart *cart, const char *username);
int handle_cart_menu(ShoppingCart *cart, const char *username, TxStack *stack, Catalog *catalog);
void display_remove_item_confirmation(const char *product_name);
void display_update_quantity_dialog(const char *product_name, int current_qty);
void display_clear_cart_confirmation(void);
int checkout_display(ShoppingCart *cart, const char *username, TxStack *stack, Catalog *catalog, int suppress_unavailable_message);
int is_cart_empty(ShoppingCart *cart);
void free_cart(ShoppingCart *cart);
float get_cart_total(ShoppingCart *cart);
void display_add_to_cart_success(const char *product_name, const char *condition, 
                                float price, const char *seller, int quantity,
                                ShoppingCart *cart);

#endif /* CART_H */