#ifndef QUEUES_H
#define QUEUES_H

#define MAKE_QUEUE(name, T) \
int queue_##name##_get(const Queue* q, unsigned int index, T* out_value) { \
if (index >= q->length) return 0; \
QueueNode* n = q->root; \
while (index--) { \
n = n->next; \
} \
*out_value = *(T*)n->data; \
return 1; \
} \
T* queue_##name##_get_ptr(const Queue* q, unsigned int index) { \
if (index >= q->length) return NULL; \
QueueNode* n = q->root; \
while (index--) { \
n = n->next; \
} \
return (T*)n->data; \
} \
int queue_##name##_push(Queue* q, T value) { \
QueueNode* n = malloc(sizeof(QueueNode)); \
if (!n) return 0; \
T* stored = malloc(sizeof(T)); \
if (!stored) { \
free(n); \
return 0; \
} \
*stored = value; \
n->data = stored; \
n->next = NULL; \
if (q->tail) { \
q->tail->next = n; \
} else { \
q->root = n; \
} \
q->tail = n; \
q->length++; \
return 1; \
} \
int queue_##name##_pop(Queue* q, T* out_value) { \
if (!q->root) return 0; \
QueueNode* n = q->root; \
q->root = n->next; \
if (q->root == NULL) q->tail = NULL; \
*out_value = *(T*)n->data; \
free(n->data); \
free(n); \
q->length--; \
return 1; \
}

typedef struct QueueNode {
   struct QueueNode* next;
   void* data;
} QueueNode;

typedef struct Queue {
   QueueNode* root;
   QueueNode* tail;
   unsigned int length;
} Queue;

void queue_init(Queue* q) {
   q->root = NULL;
   q->tail = NULL;
   q->length = 0;
}

void queue_free(Queue* q) {
   QueueNode* node = q->root;

   while (node) {
      QueueNode* next = node->next;

      free(node->data);
      free(node);

      node = next;
   }

   q->root = NULL;
   q->tail = NULL;
   q->length = 0;
}

MAKE_QUEUE(u, unsigned int)
#endif //QUEUES_H
