#include "../headers/list.h" 

 
List* create_list() {
    List* new_list = (List*)malloc(sizeof(List));
    new_list->size = 0;
    new_list->head = NULL;
    return new_list;
}
 
void push_to_list(List* list, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;
    list->size++;
}
 
void* pop_from_list(List* list) {
    if (list->size == 0) {
        return NULL;
    }
    Node* node_to_remove = list->head;
    void* data = node_to_remove->data;
    list->head = node_to_remove->next;
    free(node_to_remove);
    list->size--;
    return data;
}

void* get_from_list(List* list, size_t i) {
    if (list->size == 0 || i > list->size) {
        return NULL;
    }
    int poz = 0;
    Node* current_node = list->head;
    while ( poz < i) {
	poz++;
        current_node = current_node->next;
    }
    return current_node->data;
}

void clear_list(List* list) {
	if(list){
		Node* current_node = list->head;
		while (current_node != NULL) {
			Node* next_node = current_node->next;
			if(current_node->data)
				free(current_node->data);
			free(current_node);
			current_node = next_node;
		}
	}
    //free(list);
}

bool is_empty(List* list){
	return list->size == 0;
}