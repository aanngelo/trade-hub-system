#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "utils.h"

/* ============================================================
   stack.c — Trade Hub System
   DATA STRUCTURE : Stack (LIFO — Linked List based)
   Records all trade transactions in the system.
   WHY STACK: The most recent transaction is most relevant —
              LIFO naturally shows newest activity first.
   ============================================================ */

/* ----------------------------------------------------------
   init_stack
   Sets up an empty transaction stack.
   ---------------------------------------------------------- */
void init_stack(TxStack *stack)
{
    stack->top = NULL;
    stack->count = 0;
}

/* ----------------------------------------------------------
   push_transaction
   Records a new trade by pushing a TxNode onto the TOP
   of the stack. O(1) insertion.
   Status should be "completed" or "cancelled".
   ---------------------------------------------------------- */
int push_transaction(TxStack *stack,
                     int order_id,
                     int item_id,
                     const char *item_title,
                     const char *item_condition,
                     int quantity,
                     float unit_price,
                     const char *seller,
                     const char *seller_contact,
                     const char *buyer,
                     const char *buyer_contact,
                     const char *payment_method,
                     const char *status)
{

    /* Allocate a new transaction node */
    TxNode *new_tx = (TxNode *)malloc(sizeof(TxNode));
    if (!new_tx)
    {
        printf("[ERROR] Memory allocation failed.\n");
        return 0;
    }

    /* Fill in transaction data */
    new_tx->order_id = order_id;
    new_tx->item_id = item_id;
    strncpy(new_tx->item_title, item_title, sizeof(new_tx->item_title) - 1);
    new_tx->item_title[sizeof(new_tx->item_title) - 1] = '\0';
    strncpy(new_tx->item_condition, item_condition, sizeof(new_tx->item_condition) - 1);
    new_tx->item_condition[sizeof(new_tx->item_condition) - 1] = '\0';
    new_tx->quantity = quantity;
    new_tx->unit_price = unit_price;
    new_tx->price = unit_price * quantity; /* store total */
    strncpy(new_tx->seller, seller, sizeof(new_tx->seller) - 1);
    new_tx->seller[sizeof(new_tx->seller) - 1] = '\0';
    strncpy(new_tx->seller_contact, seller_contact, sizeof(new_tx->seller_contact) - 1);
    new_tx->seller_contact[sizeof(new_tx->seller_contact) - 1] = '\0';
    strncpy(new_tx->buyer, buyer, sizeof(new_tx->buyer) - 1);
    new_tx->buyer[sizeof(new_tx->buyer) - 1] = '\0';
    strncpy(new_tx->buyer_contact, buyer_contact, sizeof(new_tx->buyer_contact) - 1);
    new_tx->buyer_contact[sizeof(new_tx->buyer_contact) - 1] = '\0';
    strncpy(new_tx->payment_method, payment_method, sizeof(new_tx->payment_method) - 1);
    new_tx->payment_method[sizeof(new_tx->payment_method) - 1] = '\0';
    strncpy(new_tx->status, status, sizeof(new_tx->status) - 1);
    new_tx->status[sizeof(new_tx->status) - 1] = '\0';

    /* Auto-generate timestamp */
    get_timestamp(new_tx->date, sizeof(new_tx->date));

    /* Push to top of stack */
    new_tx->next = stack->top;
    stack->top = new_tx;
    stack->count++;

    return 1;
}

/* ----------------------------------------------------------
   pop_transaction
   Removes and returns the TOP (most recent) transaction.
   Caller must free the returned node after use.
   Returns NULL if stack is empty.
   ---------------------------------------------------------- */
TxNode *pop_transaction(TxStack *stack)
{
    if (is_stack_empty(stack))
    {
        printf("[INFO] No transactions on record.\n");
        return NULL;
    }

    TxNode *popped = stack->top;
    stack->top = stack->top->next;
    stack->count--;

    popped->next = NULL; /* clean up dangling pointer */
    return popped;
}

/* ----------------------------------------------------------
   peek_transaction
   Returns the TOP node without removing it.
   Useful to display the most recent transaction.
   Returns NULL if stack is empty.
   ---------------------------------------------------------- */
TxNode *peek_transaction(TxStack *stack)
{
    if (is_stack_empty(stack))
    {
        printf("[INFO] No transactions on record.\n");
        return NULL;
    }
    return stack->top;
}

/* ----------------------------------------------------------
   display_history
   Prints all transactions from most recent to oldest
   by traversing from top to bottom of the stack.
   ---------------------------------------------------------- */
void display_history(TxStack *stack)
{
    if (is_stack_empty(stack))
    {
        printf("  No transaction history yet.\n");
        return;
    }

    TxNode *current = stack->top;
    int num = 1;

    printf("  #   ID      Item                  Seller          Buyer           Price       Status\n");
    printf("  --------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        printf("  [%-2d] %-6d %-21s %-15s %-15s PHP %-9.2f %s\n",
               num,
               current->item_id,
               current->item_title,
               current->seller,
               current->buyer,
               current->price,
               current->status);
        printf("       Date: %s\n", current->date);
        num++;
        current = current->next;
    }

    printf("  --------------------------------------------------------------------------------------\n");
    printf("  Total transactions: %d\n", stack->count);
}

/* ----------------------------------------------------------
   display_user_orders
   Prints only the transactions that belong to the logged-in buyer.
   ---------------------------------------------------------- */
void display_user_orders(TxStack *stack, const char *username)
{
    if (is_stack_empty(stack))
    {
        printf("  You have no orders yet.\n");
        return;
    }

    TxNode *current = stack->top;
    int num = 1;
    int order_count = 0;

    printf("  No.  Order ID   Product    Seller          Price        Status\n");

    while (current != NULL)
    {
        if (strcmp(current->buyer, username) == 0)
        {
            printf("  [%-2d] #%-8d  %-10s %-15s PHP %-9.2f %-10s\n",
                   num,
                   current->order_id,
                   current->item_title,
                   current->seller,
                   current->price,
                   current->status);
            num++;
            order_count++;
        }
        current = current->next;
    }

    if (order_count == 0)
    {
        printf("  You have no orders yet.\n");
    }
    else
    {
        printf("\n");
        printf("  Total Orders : %d order(s)\n", order_count);
    }
}

/* ----------------------------------------------------------
   get_user_order_by_index
   Returns the Nth order for a specific buyer, or NULL if not found.
   ---------------------------------------------------------- */
TxNode *get_user_order_by_index(TxStack *stack, const char *username, int index)
{
    if (is_stack_empty(stack) || index <= 0)
    {
        return NULL;
    }

    TxNode *current = stack->top;
    int num = 1;

    while (current != NULL)
    {
        if (strcmp(current->buyer, username) == 0)
        {
            if (num == index)
            {
                return current;
            }
            num++;
        }
        current = current->next;
    }

    return NULL;
}

/* ----------------------------------------------------------
   is_stack_empty
   Returns 1 if the stack has no transactions, 0 otherwise.
   ---------------------------------------------------------- */
int is_stack_empty(TxStack *stack)
{
    return stack->top == NULL;
}

/* ----------------------------------------------------------
   free_stack
   Frees all transaction nodes in the stack.
   Call before exiting the program.
   ---------------------------------------------------------- */
void free_stack(TxStack *stack)
{
    TxNode *current = stack->top;
    TxNode *next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    stack->top = NULL;
    stack->count = 0;
}
