#include "cart.h"
#include "catalog.h"
#include "stack.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* cart.c — Trade Hub System
   Cart: shopping cart for selected items implementation */

    //Initialize an empty shopping cart.
void init_cart(ShoppingCart *cart) {
    cart->head = NULL;
    cart->count = 0;
    cart->total = 0.0f;
}

    // Add an item to the shopping cart.
int add_to_cart(ShoppingCart *cart, int item_id, const char *item_title,
                const char *condition, const char *seller, const char *contact,
                const char *buyer_username, float price, int quantity) {
    if (quantity <= 0) {
        return 0;
    }

    // Check if item already exists in cart
    CartItem *current = cart->head;
    while (current != NULL) {
        if (current->item_id == item_id) {
    // Item already in cart, increase quantity
            current->quantity += quantity;
            cart->total += price * quantity;
            return 1; /* Success */
        }
        current = current->next;
    }

    // Item not in cart, add new item
    CartItem *new_item = (CartItem *)malloc(sizeof(CartItem));
    if (new_item == NULL) {
        return 0; /* Memory allocation failed */
    }

    new_item->item_id = item_id;
    strcpy(new_item->item_title, item_title);
    strcpy(new_item->condition, condition);
    strcpy(new_item->seller, seller);
    strcpy(new_item->contact, contact);
    strcpy(new_item->buyer_username, buyer_username);
    new_item->price = price;
    new_item->quantity = quantity;
    new_item->next = cart->head;

    cart->head = new_item;
    cart->count++;
    cart->total += price * quantity;

    return 1; /* Success */
}

    // --- Remove an item from the shopping cart ---
int remove_from_cart(ShoppingCart *cart, int item_id) {
    CartItem *current = cart->head;
    CartItem *previous = NULL;

    while (current != NULL) {
        if (current->item_id == item_id) {
    // Found the item to remove
            if (previous == NULL) {
    // Item is at the head
                cart->head = current->next;
            } else {
    // Item is in the middle or end
                previous->next = current->next;
            }

            cart->total -= (current->price * current->quantity);
            cart->count--;
            free(current);
            return 1; /* Success */
        }
        previous = current;
        current = current->next;
    }

    return 0; /* Item not found */
}

    // --- Display all items in the shopping cart ---
void display_cart(ShoppingCart *cart, const char *username) {
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("                                    YOUR CART                      "
           "             \n");
    printf("==================================================================="
           "=============\n");
    printf("  Logged in as: %s (Buyer)\n\n", username);
    if (is_cart_empty(cart)) {
        printf("  Your cart is empty.\n");
        printf("  Browse products and add items!\n\n");
        printf("  [B] Browse Products   [M] Back to Buyer Menu\n");
        printf("==============================================================="
               "=================\n");
        printf("  Enter choice [0-Back]: ");
        return;
    }

    printf("  No.  Product              Condition        Qty    Unit Price    "
           "Subtotal\n");
    printf("  "
           "-------------------------------------------------------------------"
           "-----------\n");

    CartItem *current = cart->head;
    int item_number = 1;
    while (current != NULL) {
        printf("  [%-2d]  %-18s %-16s %-4d   PHP %-9.2f  PHP %.2f\n",
               item_number, current->item_title, current->condition,
               current->quantity, current->price,
               current->price * current->quantity);
        item_number++;
        current = current->next;
    }

    printf("  "
           "-------------------------------------------------------------------"
           "-----------\n");
    printf("  Total Items : %d item(s)\n", cart->count);
    printf("  Total Price : PHP %.2f\n", cart->total);
    printf("\n");
    printf("  [C] Checkout   [R] Remove Item   [U] Update Quantity   [X] Clear "
           "Cart   [B] Back\n");
    printf("\n");
    printf("==================================================================="
           "=============\n");
}

    // --- Display remove item confirmation dialog ---
void display_remove_item_confirmation(const char *product_name) {
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("  !!REMOVE ITEM\n");
    printf("==================================================================="
           "=============\n");
    printf("  Are you sure you want to remove %s from your cart?\n",
           product_name);
    printf("                        [Y] Yes, Remove   [N] No, Keep It\n");
    printf("==================================================================="
           "=============\n");
}

    // --- Display update quantity dialog ---
void display_update_quantity_dialog(const char *product_name, int current_qty) {
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("                                    UPDATE QUANTITY                "
           "             \n");
    printf("==================================================================="
           "=============\n");
    printf("  Update Quantity - %s\n", product_name);
    printf("-------------------------------------------------------------------"
           "-------------\n");
    printf("  Current Quantity : %d pcs\n", current_qty);
    printf("  Enter New Quantity [0-Back]: ");
}

    // --- Display clear cart confirmation dialog ---
void display_clear_cart_confirmation(void) {
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("  !! CLEAR CART\n");
    printf("==================================================================="
           "=============\n");
    printf("  Are you sure you want to remove ALL items from your cart?\n");
    printf("  This action cannot be undone.\n");
    printf("                    [Y] Yes, Clear All   [N] No, Keep Items        "
           "             \n");
    printf("==================================================================="
           "=============\n");
}

    // --- Handle the cart menu interactions ---
int handle_cart_menu(ShoppingCart *cart, const char *username, TxStack *stack,
                     Catalog *catalog) {
    while (1) {
        display_cart(cart, username);

        if (is_cart_empty(cart)) {
            char input[16];
            if (!fgets(input, sizeof(input), stdin)) {
                continue;
            }
            trim_input(input);
            if (strcasecmp(input, "M") == 0 || strcasecmp(input, "0") == 0) {
                return 0;
            } else if (strcasecmp(input, "B") == 0) {
                return 1;
            }
            printf("  [ERROR] Enter B, M, or 0.\n");
            continue;
        }

        printf("  Enter choice [0-Back]: ");
        char input[16];
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }
        trim_input(input);

        if (strcasecmp(input, "B") == 0 || strcmp(input, "0") == 0) {
            return 0;
        } else if (strcasecmp(input, "C") == 0) {
            if (cart->count > 1) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                                    YOUR CART          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Enter item No. to checkout [0-Back]: ");

                char item_choice[16];
                if (!fgets(item_choice, sizeof(item_choice), stdin)) {
                    continue;
                }
                trim_input(item_choice);
                if (strcasecmp(item_choice, "0") == 0) {
                    continue;
                }

                int selected_index = atoi(item_choice);
                if (selected_index <= 0) {
                    printf("  [ERROR] Invalid item number.\n");
                    continue;
                }

                CartItem *current = cart->head;
                int item_num = 1;
                while (current != NULL && item_num < selected_index) {
                    current = current->next;
                    item_num++;
                }

                if (current == NULL || item_num != selected_index) {
                    printf("  [ERROR] Invalid item number.\n");
                    continue;
                }

                ShoppingCart temp_cart;
                init_cart(&temp_cart);
                add_to_cart(&temp_cart, current->item_id, current->item_title,
                            current->condition, current->seller,
                            current->contact, username, current->price,
                            current->quantity);

                int cr =
                    checkout_display(&temp_cart, username, stack, catalog, 1);
                if (cr == 1) {
                    remove_from_cart(cart, current->item_id);
                    if (is_cart_empty(cart)) {
                        free_cart(&temp_cart);
                        return 0;
                    }
                } else if (cr == 2) {
                    free_cart(&temp_cart);
                    return 0;
                }
                free_cart(&temp_cart);
                continue;
            }

            int cr = checkout_display(cart, username, stack, catalog, 1);
            if (cr == 1) {
                free_cart(cart);
                return 0;
            }
            if (cr == 2 || is_cart_empty(cart)) {
                return 0;
            }
            continue;
        } else if (strcasecmp(input, "R") == 0) {
            printf("\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("                                    YOUR CART              "
                   "                    "
                   " \n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Logged in as: %s (Buyer)\n\n", username);
            printf("  Enter No. to Remove [0-Back]: ");
            char remove_input[16];
            if (!fgets(remove_input, sizeof(remove_input), stdin)) {
                continue;
            }
            trim_input(remove_input);
            if (strcasecmp(remove_input, "0") == 0) {
                continue;
            }
            int remove_idx = atoi(remove_input);
            if (remove_idx <= 0) {
                printf("  [ERROR] Invalid item number.\n");
                continue;
            }
            CartItem *current = cart->head;
            int item_num = 1;
            while (current != NULL && item_num < remove_idx) {
                current = current->next;
                item_num++;
            }
            if (current == NULL || item_num != remove_idx) {
                printf("  [ERROR] Invalid item number.\n");
                continue;
            }
            char removed_name[100];
            strncpy(removed_name, current->item_title,
                    sizeof(removed_name) - 1);
            removed_name[sizeof(removed_name) - 1] = '\0';
            printf("\n");
            printf("  Remove %s from your cart?\n", removed_name);
            printf("  [Y] Yes   [N] No\n\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Enter choice [0-Back]: ");
            char confirm[16];
            if (!fgets(confirm, sizeof(confirm), stdin)) {
                continue;
            }
            trim_input(confirm);
            if (strcasecmp(confirm, "Y") == 0) {
                remove_from_cart(cart, current->item_id);
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                                    YOUR CART          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Logged in as: %s (Buyer)\n\n", username);
                printf("  %s has been removed from your cart.\n\n",
                       removed_name);
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Press Enter to continue...");
                fgets(confirm, sizeof(confirm), stdin);
            }
            continue;
        } else if (strcasecmp(input, "U") == 0) {
            printf("\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("                                    YOUR CART              "
                   "                    "
                   " \n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Logged in as: %s (Buyer)\n\n", username);
            printf("  Enter No. to Update [0-Back]: ");
            char update_input[16];
            if (!fgets(update_input, sizeof(update_input), stdin)) {
                continue;
            }
            trim_input(update_input);
            if (strcasecmp(update_input, "0") == 0) {
                continue;
            }
            int update_idx = atoi(update_input);
            if (update_idx <= 0) {
                printf("  [ERROR] Invalid item number.\n");
                continue;
            }
            CartItem *current = cart->head;
            int item_num = 1;
            while (current != NULL && item_num < update_idx) {
                current = current->next;
                item_num++;
            }
            if (current == NULL || item_num != update_idx) {
                printf("  [ERROR] Invalid item number.\n");
                continue;
            }
            Item *it = NULL;
            int available_stock = current->quantity;
            if (catalog != NULL) {
                it = get_item_by_id(catalog, current->item_id);
                if (it != NULL)
                    available_stock = it->quantity;
            }

            printf("\n");
            printf("  Product          : %s\n", current->item_title);
            printf("  Current Quantity : %d pc\n", current->quantity);
            printf("  Available Stock  : %d pcs\n\n", available_stock);
            printf("  Enter New Quantity [0-Back]: ");
            char quantity_input[16];
            if (!fgets(quantity_input, sizeof(quantity_input), stdin)) {
                continue;
            }
            trim_input(quantity_input);
            if (strcasecmp(quantity_input, "0") == 0) {
                continue;
            }
            int new_quantity = atoi(quantity_input);
            if (new_quantity <= 0) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Quantity must be a positive number.\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                continue;
            }
            if (new_quantity > available_stock) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Quantity exceeds available stock.\n");
                printf("  Available Stock : %d pcs\n\n", available_stock);
                printf("  Enter New Quantity [0-Back]: ");
                if (!fgets(quantity_input, sizeof(quantity_input), stdin))
                    continue;
                trim_input(quantity_input);
                if (strcasecmp(quantity_input, "0") == 0)
                    continue;
                new_quantity = atoi(quantity_input);
                if (new_quantity <= 0)
                    continue;
                if (new_quantity > available_stock)
                    continue;
            }
            if (update_cart_item_quantity(cart, current->item_id,
                                          new_quantity)) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                                    YOUR CART          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Logged in as: %s (Buyer)\n\n", username);
                printf("  %s quantity updated to %d pcs.\n",
                       current->item_title, new_quantity);
                printf("  New Subtotal : PHP %.2f\n\n",
                       current->price * current->quantity);
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Press Enter to continue...");
                fgets(quantity_input, sizeof(quantity_input), stdin);
            }
            continue;
        } else if (strcasecmp(input, "X") == 0) {
            printf("\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("                                    YOUR CART              "
                   "                    "
                   " \n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Logged in as: %s (Buyer)\n\n", username);
            printf("  Remove all items from your cart?\n");
            printf("  This action cannot be undone.\n");
            printf("  [Y] Yes   [N] No\n\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Enter choice [0-Back]: ");
            char confirm[16];
            if (!fgets(confirm, sizeof(confirm), stdin)) {
                continue;
            }
            trim_input(confirm);
            if (strcasecmp(confirm, "Y") == 0) {
                free_cart(cart);
                init_cart(cart);
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                                    YOUR CART          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Logged in as: %s (Buyer)\n\n", username);
                printf("  Your cart has been cleared.\n\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Press Enter to continue...");
                fgets(confirm, sizeof(confirm), stdin);
            }
            continue;
        }

        printf("  [ERROR] Invalid choice.\n");
    }
}

    // --- Update quantity of a cart item ---
int update_cart_item_quantity(ShoppingCart *cart, int item_id,
                              int new_quantity) {
    if (new_quantity <= 0) {
        return remove_from_cart(cart, item_id);
    }

    CartItem *current = cart->head;
    while (current != NULL) {
        if (current->item_id == item_id) {
            /* Update total by removing old quantity cost and adding new
             * quantity cost */
            float old_subtotal = current->price * current->quantity;
            current->quantity = new_quantity;
            float new_subtotal = current->price * current->quantity;
            cart->total = cart->total - old_subtotal + new_subtotal;
            return 1; /* Success */
        }
        current = current->next;
    }
    return 0; /* Item not found */
}

    // --- Check if the shopping cart is empty ---
int is_cart_empty(ShoppingCart *cart) { return cart->head == NULL; }

    // --- Free all memory used by the shopping cart ---
void free_cart(ShoppingCart *cart) {
    CartItem *current = cart->head;
    while (current != NULL) {
        CartItem *temp = current;
        current = current->next;
        free(temp);
    }
    cart->head = NULL;
    cart->count = 0;
    cart->total = 0.0f;
}

    // --- Get the total price of all items in the cart ---
float get_cart_total(ShoppingCart *cart) { return cart->total; }

static void checkout_print_datetime(char *buf, size_t sz) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t)
        strftime(buf, sz, "%Y-%m-%d  %I:%M %p", t);
    else
        buf[0] = '\0';
}

static void print_checkout_step1_summary(const ShoppingCart *cart) {
    printf("  No.  Product              Condition    Qty    Unit Price    "
           "Subtotal\n");
    printf("  "
           "-------------------------------------------------------------------"
           "-----------\n");

    CartItem *cur = cart->head;
    int n = 1;
    while (cur != NULL) {
        printf("  [ %2d] %-19s %-12s %-4d   PHP %-9.2f  PHP %.2f\n", n,
               cur->item_title, cur->condition, cur->quantity, cur->price,
               cur->price * cur->quantity);
        n++;
        cur = cur->next;
    }

    printf("  "
           "-------------------------------------------------------------------"
           "-----------\n");
    printf("  Total Items : %d item(s)\n", cart->count);
    printf("  Total Price : PHP %.2f\n", cart->total);
    printf("\n");
    printf("  HOW WOULD YOU LIKE TO PAY?\n");
    printf("  [1]  Cash on Delivery\n");
    printf("  [2]  GCash\n");
    printf("\n");
    printf("==================================================================="
           "=============\n");
}

/* checkout_display: 0 = resume cart, 1 = order placed, 2 = leave cart menu
 * (back to buyer menu) */
int checkout_display(ShoppingCart *cart, const char *username, TxStack *stack,
                     Catalog *catalog, int suppress_unavailable_message) {
    char payment_record[80];
    char gcash_ref[64];

    if (is_cart_empty(cart)) {
        while (1) {
            printf("\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("                              !! CHECKOUT FAILED           "
                   "                    "
                   " \n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("\n");
            printf("                  Your cart is empty.\n");
            printf("             Add items before checking out!\n");
            printf("\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  [B] Browse Products   [M] Back to Buyer Menu\n");
            printf("==========================================================="
                   "===================="
                   "=\n");
            printf("  Enter choice [0-Back]: ");

            char nav[32];
            if (!fgets(nav, sizeof(nav), stdin))
                continue;
            trim_input(nav);
            if (strcasecmp(nav, "B") == 0 || strcasecmp(nav, "M") == 0 ||
                strcmp(nav, "0") == 0)
                return 2;
            printf("  [ERROR] Enter B, M, or 0.\n");
        }
    }

    if (catalog != NULL) {
        if (!suppress_unavailable_message) {
            CartItem *chk = cart->head;
            while (chk != NULL) {
                CartItem *next_chk = chk->next;
                Item *it = get_item_by_id(catalog, chk->item_id);
                if (it == NULL) {
                    it = get_item_by_details(catalog, chk->item_title,
                                             chk->condition, chk->seller,
                                             chk->contact, chk->price);
                    if (it != NULL) {
                        chk->item_id = it->id;
                    }
                }

                if (it == NULL || it->is_sold || it->quantity < chk->quantity) {
                    remove_from_cart(cart, chk->item_id);
                }
                chk = next_chk;
            }
        }

        if (is_cart_empty(cart)) {
            while (1) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                              !! CHECKOUT FAILED       "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("\n");
                printf("                  No available items in cart.\n");
                printf("         Some items became unavailable or out of "
                       "stock.\n");
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  [B] Browse Products   [M] Back to Buyer Menu\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Enter choice [0-Back]: ");

                char nav[32];
                if (!fgets(nav, sizeof(nav), stdin))
                    continue;
                trim_input(nav);
                if (strcasecmp(nav, "B") == 0 || strcasecmp(nav, "M") == 0 ||
                    strcmp(nav, "0") == 0)
                    return 2;
                printf("  [ERROR] Enter B, M, or 0.\n");
            }
        }
    }

payment_select:
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("                                   CHECKOUT                        "
           "            \n");
    printf("==================================================================="
           "=============\n");
    printf("  Logged in as: %s (Buyer)\n\n", username);
    print_checkout_step1_summary(cart);

    char payment_input[16];
    int pay_choice = -1;
    while (1) {
        printf("  Enter choice [0-Back]: ");
        if (!fgets(payment_input, sizeof(payment_input), stdin))
            continue;
        trim_input(payment_input);
        if (strcasecmp(payment_input, "B") == 0 ||
            strcmp(payment_input, "0") == 0)
            return 0;
        if (strcmp(payment_input, "1") == 0) {
            pay_choice = 1;
            strcpy(payment_record, "Cash on Delivery");
            break;
        }
        if (strcmp(payment_input, "2") == 0) {
            pay_choice = 2;
            break;
        }
        printf("  [ERROR] Invalid choice. Enter 1, 2, or 0.\n");
    }

    gcash_ref[0] = '\0';
    if (pay_choice == 2) {
    gcash_ref_step:
        printf("\n");
        printf("==============================================================="
               "=================\n");
        printf("                               GCASH PAYMENT                   "
               "                 \n");
        printf("==============================================================="
               "=================\n");
        printf("  Amount to Pay  : PHP %.2f\n", cart->total);
        printf("---------------------------------------------------------------"
               "-----------------\n");
        printf("  GCash Number   : 0917-XXX-XXXX\n");
        printf("  Account Name   : Trade Hub\n");
        printf("---------------------------------------------------------------"
               "-----------------\n");
        printf("  Enter GCash Reference No. [0-Back]: ");

        char ref_line[128];
        if (!fgets(ref_line, sizeof(ref_line), stdin))
            goto gcash_ref_step;
        trim_input(ref_line);
        if (strcmp(ref_line, "0") == 0)
            goto payment_select;
        if (strlen(ref_line) == 0) {
            printf("  [ERROR] Enter a reference number or 0 to go back.\n");
            goto gcash_ref_step;
        }
        strncpy(gcash_ref, ref_line, sizeof(gcash_ref) - 1);
        gcash_ref[sizeof(gcash_ref) - 1] = '\0';

    gcash_confirm:
        printf("\n");
        printf("==============================================================="
               "=================\n");
        printf("                               GCASH PAYMENT                   "
               "                 \n");
        printf("==============================================================="
               "=================\n");
        printf("  Reference No.  : %s\n", gcash_ref);
        printf("  Amount         : PHP %.2f\n", cart->total);
        printf("---------------------------------------------------------------"
               "-----------------\n");
        printf("  Confirm GCash Payment?\n");
        printf("  [Y] Yes, Confirm   [N] No, Go Back\n");
        printf("==============================================================="
               "=================\n");
        printf("  Enter choice [0-Back]: ");

        char gc_nav[32];
        if (!fgets(gc_nav, sizeof(gc_nav), stdin))
            goto gcash_confirm;
        trim_input(gc_nav);
        if (strcasecmp(gc_nav, "Y") == 0) {
            snprintf(payment_record, sizeof(payment_record), "GCash (Ref: %s)",
                     gcash_ref);
        } else if (strcasecmp(gc_nav, "N") == 0) {
            goto gcash_ref_step;
        } else if (strcasecmp(gc_nav, "B") == 0 || strcmp(gc_nav, "0") == 0) {
            goto payment_select;
        } else {
            printf("  [ERROR] Enter Y, N, or 0.\n");
            goto gcash_confirm;
        }
    }

confirm_order:
    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("                                   CHECKOUT                        "
           "            \n");
    printf("==================================================================="
           "=============\n");
    printf("  Logged in as: %s (Buyer)\n\n", username);
    printf("  Product  : %s\n", cart->head->item_title);
    printf("  Qty      : %d pc\n", cart->head->quantity);
    printf("  Total    : PHP %.2f\n", cart->total);
    printf("  Payment  : %s\n", pay_choice == 1 ? "Cash on Delivery" : "GCash");
    printf("\n");
    printf("  Confirm Order?\n");
    printf("  [Y] Yes, Place Order   [N] No, Go Back\n");
    printf("==================================================================="
           "=============\n");
    printf("  Enter choice [0-Back]: ");

    char co_nav[32];
    if (!fgets(co_nav, sizeof(co_nav), stdin))
        goto confirm_order;
    trim_input(co_nav);
    if (strcasecmp(co_nav, "Y") != 0) {
        if (strcasecmp(co_nav, "N") == 0 || strcasecmp(co_nav, "B") == 0 ||
            strcmp(co_nav, "0") == 0) {
            while (1) {
                printf("\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                             ORDER NOT PLACED          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Your order was cancelled.\n");
                printf("  No transaction was recorded.\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  [C] Continue Shopping   [B] Back to Buyer Menu\n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Enter choice [0-Back]: ");

                char np[32];
                if (!fgets(np, sizeof(np), stdin))
                    continue;
                trim_input(np);
                if (strcasecmp(np, "C") == 0)
                    return 0;
                if (strcasecmp(np, "B") == 0 || strcmp(np, "0") == 0)
                    return 2;
                printf("  [ERROR] Enter C, B, or 0.\n");
            }
        }
        printf("  [ERROR] Enter Y, N, or 0.\n");
        goto confirm_order;
    }

    // Place order
    int order_id = generate_id();
    char order_date[64];
    checkout_print_datetime(order_date, sizeof(order_date));

    CartItem *current = cart->head;
    while (current != NULL) {
        push_transaction(stack, order_id, current->item_id, current->item_title,
                         current->condition, current->quantity, current->price,
                         current->seller, current->contact,
                         current->buyer_username, username, payment_record,
                         "Pending");
        current = current->next;
    }

    if (catalog != NULL) {
        CartItem *ci = cart->head;
        while (ci != NULL) {
            catalog_apply_purchase(catalog, ci->item_id, ci->quantity);
            ci = ci->next;
        }
    }

    printf("\n");
    printf("==================================================================="
           "=============\n");
    printf("                              ORDER CONFIRMED!                     "
           "             \n");
    printf("==================================================================="
           "=============\n");
    printf("  Logged in as: %s (Buyer)\n\n", username);
    printf("  Your order has been placed successfully!\n\n");
    printf("  Order ID  : #%05d\n", order_id);
    printf("  Date      : %s\n", order_date);
    printf("  Product   : %s\n", cart->head->item_title);
    printf("  Qty       : %d pc\n", cart->head->quantity);
    printf("  Total     : PHP %.2f\n", cart->total);
    printf("  Payment   : %s\n", payment_record);
    printf("  Status    : Pending\n\n");
    printf("  [V] View My Orders   [B] Back to Buyer Menu\n");
    printf("==================================================================="
           "=============\n");

    while (1) {
        printf("  Enter choice [0-Back]: ");
        char order_nav[32];
        if (!fgets(order_nav, sizeof(order_nav), stdin))
            continue;
        trim_input(order_nav);

        if (strcasecmp(order_nav, "B") == 0 || strcmp(order_nav, "0") == 0)
            break;
        if (strcasecmp(order_nav, "V") == 0) {
            while (1) {
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("                                     MY ORDERS         "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                display_user_orders(stack, username);
                printf("-------------------------------------------------------"
                       "--------------------"
                       "-----\n");
                printf("                       [V] View Order Details          "
                       "                    "
                       "     \n");
                printf("======================================================="
                       "===================="
                       "=====\n");
                printf("  Enter choice [0-Back]: ");

                char detail_nav[32];
                if (!fgets(detail_nav, sizeof(detail_nav), stdin))
                    break;
                trim_input(detail_nav);

                if (strcasecmp(detail_nav, "B") == 0 ||
                    strcmp(detail_nav, "0") == 0)
                    break;
                if (strcasecmp(detail_nav, "V") == 0) {
                    printf("  Enter No. to View Details: ");
                    char detail_input[32];
                    if (!fgets(detail_input, sizeof(detail_input), stdin))
                        continue;
                    int detail_idx = atoi(detail_input);
                    TxNode *selected_order =
                        get_user_order_by_index(stack, username, detail_idx);
                    if (!selected_order) {
                        printf("  [ERROR] Invalid order number.\n");
                        continue;
                    }

                    while (1) {
                        printf("==============================================="
                               "===================="
                               "=============\n");
                        printf("                                   Order "
                               "Details                   "
                               "             \n");
                        printf("==============================================="
                               "===================="
                               "=============\n");
                        printf("  Order ID  : #%05d\n",
                               selected_order->order_id);
                        printf("  Product   : %s\n",
                               selected_order->item_title);
                        printf("  Seller    : %s\n", selected_order->seller);
                        printf("  Total     : PHP %.2f\n",
                               selected_order->price);
                        printf("  Status    : %s\n", selected_order->status);
                        printf("  Date      : %s\n", selected_order->date);
                        printf("-----------------------------------------------"
                               "--------------------"
                               "-------------\n");
                        printf("              [C] Cancel Order                 "
                               "                    "
                               "           \n");
                        printf("==============================================="
                               "===================="
                               "=============\n");
                        printf("  Enter choice [0-Back]: ");

                        char detail_choice[32];
                        if (!fgets(detail_choice, sizeof(detail_choice), stdin))
                            break;
                        trim_input(detail_choice);

                        if (strcasecmp(detail_choice, "B") == 0 ||
                            strcmp(detail_choice, "0") == 0)
                            break;
                        if (strcasecmp(detail_choice, "C") == 0) {
                            if (strcasecmp(selected_order->status, "Pending") !=
                                0) {
                                printf("======================================="
                                       "===================="
                                       "=====================\n");
                                printf("                                CANNOT "
                                       "CANCEL ORDER        "
                                       "                     \n");
                                printf("======================================="
                                       "===================="
                                       "=====================\n");
                                printf("  Order #%05d is already %s.\n",
                                       selected_order->order_id,
                                       selected_order->status);
                                printf("  Only PENDING orders can be "
                                       "cancelled.\n");
                                printf("---------------------------------------"
                                       "--------------------"
                                       "---------------------\n");
                                printf("  Enter choice [0-Back]: ");
                                char back_input[32];
                                fgets(back_input, sizeof(back_input), stdin);
                                continue;
                            }

                            printf("==========================================="
                                   "===================="
                                   "=================\n");
                            printf("                                  CANCEL "
                                   "ORDER                 "
                                   "                 \n");
                            printf("==========================================="
                                   "===================="
                                   "=================\n");
                            printf("  Are you sure you want to cancel Order "
                                   "#%05d?\n",
                                   selected_order->order_id);
                            printf("  Product : %s\n",
                                   selected_order->item_title);
                            printf("  Total   : PHP %.2f\n",
                                   selected_order->price);
                            printf("-------------------------------------------"
                                   "--------------------"
                                   "-----------------\n");
                            printf("                        [Y] Yes, Cancel   "
                                   "[N] No, Go Back      "
                                   "                 \n");
                            printf("==========================================="
                                   "===================="
                                   "=================\n");
                            printf("  Enter choice [0-Back]: ");

                            char confirm_cancel[16];
                            if (!fgets(confirm_cancel, sizeof(confirm_cancel),
                                       stdin))
                                continue;
                            trim_input(confirm_cancel);

                            char confirm_char =
                                toupper((unsigned char)confirm_cancel[0]);
                            if (confirm_char == 'Y') {
                                strncpy(selected_order->status, "Cancelled",
                                        sizeof(selected_order->status) - 1);
                                selected_order
                                    ->status[sizeof(selected_order->status) -
                                             1] = '\0';
                                printf("======================================="
                                       "===================="
                                       "=====================\n");
                                printf("  Order #%05d has been successfully "
                                       "cancelled.\n",
                                       selected_order->order_id);
                                printf("======================================="
                                       "===================="
                                       "=====================\n");
                                printf("  Enter choice [0-Back]: ");
                                char back_input[32];
                                fgets(back_input, sizeof(back_input), stdin);
                                break;
                            }
                            printf("==========================================="
                                   "===================="
                                   "=================\n");
                            printf("  Order #%05d was NOT cancelled.\n",
                                   selected_order->order_id);
                            printf("  Your order remains PENDING.\n");
                            printf("==========================================="
                                   "===================="
                                   "=================\n");
                            printf("  Enter choice [0-Back]: ");
                            char back_input[32];
                            fgets(back_input, sizeof(back_input), stdin);
                            continue;
                        }
                        printf("  [ERROR] Invalid choice. Enter C or 0.\n");
                    }
                    continue;
                }
                printf("  [ERROR] Invalid choice. Enter V or 0.\n");
            }
            continue;
        }
        printf("  [ERROR] Invalid choice. Enter V, B, or 0.\n");
    }

    free_cart(cart);
    init_cart(cart);
    return 1;
}

    // --- Display add-to-cart success message ---
void display_add_to_cart_success(const char *product_name,
                                 const char *condition, float price,
                                 const char *seller, int quantity,
                                 ShoppingCart *cart) {
    printf("==================================================================="
           "=============\n");
    printf("                                   Add to Cart                     "
           "             \n");
    printf("==================================================================="
           "=============\n");
    printf("  Item successfully added to your cart!\n");
    printf("-------------------------------------------------------------------"
           "-------------\n");
    printf("  Product   : %s\n", product_name);
    printf("  Condition : %s\n", condition);
    printf("  Price     : PHP %.2f\n", price);
    printf("  Seller    : %s\n", seller);
    printf("  Quantity  : %d pc\n", quantity);
    printf("  Subtotal  : PHP %.2f\n", price * quantity);
    printf("-------------------------------------------------------------------"
           "-------------\n");
    printf("  Cart Summary  :\n");
    printf("  Items in Cart : %d item(s)\n", cart->count);
    printf("  Cart Total    : PHP %.2f\n", cart->total);
    printf("-------------------------------------------------------------------"
           "-------------\n");
    printf("  [1] Continue Browsing\n");
    printf("  [2] View Cart\n");
    printf("  [3] Checkout Now\n");
    printf("==================================================================="
           "=============\n");
    printf("  Enter choice [0-Back]: ");
}
