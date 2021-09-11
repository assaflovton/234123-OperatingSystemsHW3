#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;
int queue_size=0;

void enqueue(void *x)
{
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->value = x;
    new_node->next = NULL;
    struct timeval arrival_t;
    gettimeofday(&arrival_t, NULL);
    new_node->arrival = arrival_t.tv_sec + (arrival_t.tv_usec)/1000000.0;
    if (tail == NULL)
    {
        head = new_node;
    }
    else
    {
        tail->next = new_node;
    }
    tail = new_node;
    queue_size++;
    
}

void* dequeue(unsigned long* dispatch, unsigned long *arrival)
{
    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        void *result = head->value;
        node_t *temp = head;
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        queue_size--;
        
        struct timeval dispatch_t;
        gettimeofday(&dispatch_t, NULL);
        *dispatch = ((dispatch_t.tv_sec + (dispatch_t.tv_usec)/1000000.0)) - (temp->arrival);
        *arrival = temp->arrival;
        free(temp);
        return result;
    }
}

// void printQueue()
// {
//     node_t cur = head;
//     printf("\nthis is queue:");
//     while (cur!=NULL)
//     {
//       printf("%d->", (int)cur->object);
//       cur = cur->next;
//     }
// }