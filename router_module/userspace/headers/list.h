#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
    void* data;
    struct node* next;
} Node;
 
typedef struct list {
    int size;
    Node* head;
} List;


List* create_list();

void push_to_list(List* list, void* data);

void* pop_from_list(List* list);

void* get_from_list(List* list, size_t i);

void clear_list(List* list);

bool is_empty(List* list);