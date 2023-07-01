#include <stdio.h>
#include <stdlib.h>

// Structure for a node in the double linked list
struct Node {
    int data;
    struct Node* prev;
    struct Node* next;
};

// Function to create a new node
struct Node* createNode(int data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// Function to add a node at the beginning of the list
void addNode(struct Node** head, int data) {
    struct Node* newNode = createNode(data);
    newNode->next = (*head);
    if (*head != NULL) {
        (*head)->prev = newNode;
    }
    (*head) = newNode;
}

// Function to delete a node from the list
void deleteNode(struct Node** head, struct Node* delNode) {
    if (*head == NULL || delNode == NULL) {
        return;
    }
    if (*head == delNode) {
        *head = delNode->next;
    }
    if (delNode->next != NULL) {
        delNode->next->prev = delNode->prev;
    }
    if (delNode->prev != NULL) {
        delNode->prev->next = delNode->next;
    }
    free(delNode);
}

// Function to find a node with a given data value
struct Node* findNode(struct Node* head, int data) {
    struct Node* current = head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL; // Node not found
}

// Function to display the contents of the list
void displayList(struct Node* head) {
    struct Node* current = head;
    printf("List: ");
    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}

int main() {
    struct Node* head = NULL;
    addNode(&head, 3);
    addNode(&head, 7);
    addNode(&head, 12);
    addNode(&head, 5);

    displayList(head);

    struct Node* nodeToFind = findNode(head, 7);
    if (nodeToFind != NULL) {
        printf("Node found: %d\n", nodeToFind->data);
    } else {
        printf("Node not found.\n");
    }

    deleteNode(&head, head);
    displayList(head);
    
    return 0;
}
