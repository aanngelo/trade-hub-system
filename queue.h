#ifndef QUEUE_H
#define QUEUE_H

/* ============================================================
   queue.h — Trade Hub System
   Queue: buyer inquiry system declarations
   ============================================================ */

/* ----------------------------------------------------------
   InquiryNode — a single buyer inquiry in the queue
   ---------------------------------------------------------- */
typedef struct InquiryNode {
    int  item_id;               /* which listing this is for  */
    char buyer_name[50];        /* name of the buyer          */
    char buyer_contact[50];     /* messenger / phone          */
    char message[200];          /* buyer's message to seller  */
    struct InquiryNode *next;   /* pointer to next in queue   */
} InquiryNode;

/* ----------------------------------------------------------
   InquiryQueue — FIFO queue for all buyer inquiries
   front = next to be served
   rear  = last person in line
   ---------------------------------------------------------- */
typedef struct InquiryQueue {
    InquiryNode *front;
    InquiryNode *rear;
    int          count;
} InquiryQueue;

/* --- Queue function declarations --- */
void         init_queue(InquiryQueue *q);
int          enqueue_inquiry(InquiryQueue *q, int item_id,
                             const char *buyer_name,
                             const char *buyer_contact,
                             const char *message);
InquiryNode *dequeue_inquiry(InquiryQueue *q);
InquiryNode *peek_inquiry(InquiryQueue *q);
void         display_inquiries(InquiryQueue *q);
int          is_queue_empty(InquiryQueue *q);
void         free_queue(InquiryQueue *q);

#endif /* QUEUE_H */
