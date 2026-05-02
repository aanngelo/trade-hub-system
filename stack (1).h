#ifndef STACK_H
#define STACK_H

/* ============================================================
   stack.h — Trade Hub System
   Stack: transaction history declarations
   ============================================================ */

/* ----------------------------------------------------------
   TxNode — a single transaction record (stack node)
   ---------------------------------------------------------- */
typedef struct TxNode
{
   int order_id;         /* Order identifier               */
   int item_id;          /* ID of the traded item          */
   char item_title[100]; /* title of the traded item       */
   char item_condition[30]; /* condition of the item        */
   char seller[50];      /* seller's name                  */
   char seller_contact[20]; /* seller's contact              */
   char buyer[50];       /* buyer's name                   */
   int quantity;         /* quantity ordered               */
   float unit_price;    /* unit price                     */
   float price;         /* total price (unit_price * quantity) */
   char buyer_contact[20];      /* buyer contact (best-effort) */
   char payment_method[30];     /* payment method */
   char date[20];        /* timestamp of transaction       */
   char status[32];      /* "Pending" | "Awaiting Confirmation" | "Completed" | "Cancelled" */
   struct TxNode *next;  /* pointer to the node below      */
} TxNode;

/* ----------------------------------------------------------
   TxStack — LIFO stack of all trade transactions
   top = most recent transaction
   ---------------------------------------------------------- */
typedef struct TxStack
{
   TxNode *top;
   int count;
} TxStack;

/* --- Stack function declarations --- */
void init_stack(TxStack *stack);
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
                     const char *status);
TxNode *pop_transaction(TxStack *stack);
TxNode *peek_transaction(TxStack *stack);
void display_history(TxStack *stack);
void display_user_orders(TxStack *stack, const char *username);
TxNode *get_user_order_by_index(TxStack *stack, const char *username, int index);
int is_stack_empty(TxStack *stack);
void free_stack(TxStack *stack);

#endif /* STACK_H */
