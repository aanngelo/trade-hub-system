#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

/* ============================================================
   queue.c — Trade Hub System
   DATA STRUCTURE : Queue (FIFO — Linked List based)
   Manages buyer inquiries for item listings.
   WHY QUEUE: Buyers are served in order they inquired —
              first come, first served. Fair for all buyers.
   ============================================================ */

/* ----------------------------------------------------------
   init_queue
   Sets up an empty inquiry queue.
   ---------------------------------------------------------- */
void init_queue(InquiryQueue *q) {
    q->front = NULL;
    q->rear  = NULL;
    q->count = 0;
}

/* ----------------------------------------------------------
   enqueue_inquiry
   Adds a new buyer inquiry to the REAR of the queue.
   This is the "join the line" operation. O(1).
   Returns 1 on success, 0 on failure.
   ---------------------------------------------------------- */
int enqueue_inquiry(InquiryQueue *q, int item_id,
                    const char *buyer_name,
                    const char *buyer_contact,
                    const char *message) {

    /* Allocate new inquiry node */
    InquiryNode *new_node = (InquiryNode *)malloc(sizeof(InquiryNode));
    if (!new_node) {
        printf("[ERROR] Memory allocation failed.\n");
        return 0;
    }

    /* Fill in inquiry data */
    new_node->item_id = item_id;
    strncpy(new_node->buyer_name,    buyer_name,    sizeof(new_node->buyer_name) - 1);
    strncpy(new_node->buyer_contact, buyer_contact, sizeof(new_node->buyer_contact) - 1);
    strncpy(new_node->message,       message,       sizeof(new_node->message) - 1);
    new_node->next = NULL;

    /* If queue is empty, both front and rear point to this node */
    if (q->rear == NULL) {
        q->front = new_node;
        q->rear  = new_node;
    } else {
        /* Otherwise, link it after the current rear */
        q->rear->next = new_node;
        q->rear       = new_node;
    }

    q->count++;
    printf("[SUCCESS] Inquiry sent! You are #%d in line.\n", q->count);
    return 1;
}

/* ----------------------------------------------------------
   dequeue_inquiry
   Removes and returns the FRONT node (next buyer to serve).
   Caller is responsible for freeing the returned node.
   Returns NULL if queue is empty.
   ---------------------------------------------------------- */
InquiryNode *dequeue_inquiry(InquiryQueue *q) {
    if (is_queue_empty(q)) {
        printf("[INFO] No pending inquiries.\n");
        return NULL;
    }

    /* Take the front node out */
    InquiryNode *served = q->front;
    q->front = q->front->next;

    /* If queue is now empty, reset rear too */
    if (q->front == NULL) {
        q->rear = NULL;
    }

    q->count--;
    served->next = NULL; /* clean up the pointer */
    return served;
}

/* ----------------------------------------------------------
   peek_inquiry
   Returns the front inquiry WITHOUT removing it.
   Returns NULL if queue is empty.
   ---------------------------------------------------------- */
InquiryNode *peek_inquiry(InquiryQueue *q) {
    if (is_queue_empty(q)) {
        printf("[INFO] No pending inquiries.\n");
        return NULL;
    }
    return q->front;
}

/* ----------------------------------------------------------
   display_inquiries
   Prints all pending inquiries in order (front to rear).
   ---------------------------------------------------------- */
void display_inquiries(InquiryQueue *q) {
    if (is_queue_empty(q)) {
        printf("  No pending inquiries.\n");
        return;
    }

    InquiryNode *current = q->front;
    int           pos    = 1;

    printf("  %-4s %-12s %-15s %-12s %s\n",
           "#", "Item ID", "Buyer", "Contact", "Message");
    printf("  %s\n", "-------------------------------------------------------------");

    while (current != NULL) {
        printf("  %-4d %-12d %-15s %-12s %s\n",
               pos,
               current->item_id,
               current->buyer_name,
               current->buyer_contact,
               current->message);
        pos++;
        current = current->next;
    }

    printf("\n  Total pending: %d inquiry(ies).\n", q->count);
}

/* ----------------------------------------------------------
   is_queue_empty
   Returns 1 if the queue has no inquiries, 0 otherwise.
   ---------------------------------------------------------- */
int is_queue_empty(InquiryQueue *q) {
    return q->front == NULL;
}

/* ----------------------------------------------------------
   free_queue
   Frees all remaining inquiry nodes in the queue.
   Call before exiting the program.
   ---------------------------------------------------------- */
void free_queue(InquiryQueue *q) {
    InquiryNode *current = q->front;
    InquiryNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    q->front = NULL;
    q->rear  = NULL;
    q->count = 0;
}
