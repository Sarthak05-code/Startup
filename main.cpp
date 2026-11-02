#include <stdio.h>
#include <stdlib.h>

// Node structure
struct Node {
    int data;
    struct Node* next;
};

// Function to create a new node
struct Node* createNode(int data)
{
    struct Node* newNode
        = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a node at the beginning
void insertAtBeginning(struct Node** head, int data)
{
    struct Node* newNode = createNode(data);
    if (*head == NULL) {
        *head = newNode;
        newNode->next = *head;
    }
    else {
        struct Node* temp = *head;
        while (temp->next != *head) {
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
        *head = newNode;
    }
}

// Function to insert a node at the end
void insertAtEnd(struct Node** head, int data)
{
    struct Node* newNode = createNode(data);
    if (*head == NULL) {
        *head = newNode;
        newNode->next = *head;
    }
    else {
        struct Node* temp = *head;
        while (temp->next != *head) {
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
    }
}

// Function to insert a node at a specific position
void insertAtPosition(struct Node** head, int data,
                      int position)
{
    struct Node* newNode = createNode(data);
    if (*head == NULL && position == 0) {
        *head = newNode;
        newNode->next = *head;
    }
    else if (position == 0) {
        insertAtBeginning(head, data);
    }
    else {
        struct Node* temp = *head;
        int i = 0;
        while (i < position - 1) {
            temp = temp->next;
            i++;
        }
        newNode->next = temp->next;
        temp->next = newNode;
    }
}

// Function to delete a node from the beginning
void deleteFromBeginning(struct Node** head)
{
    if (*head == NULL) {
        return;
    }
    else if ((*head)->next == *head) {
        free(*head);
        *head = NULL;
    }
    else {
        struct Node* temp = *head;
        while (temp->next != *head) {
            temp = temp->next;
        }
        temp->next = (*head)->next;
        struct Node* toDelete = *head;
        *head = (*head)->next;
        free(toDelete);
    }
}

// Function to delete a node from the end
void deleteFromEnd(struct Node** head)
{
    if (*head == NULL) {
        return;
    }
    else if ((*head)->next == *head) {
        free(*head);
        *head = NULL;
    }
    else {
        struct Node* secondLast = *head;
        while (secondLast->next->next != *head) {
            secondLast = secondLast->next;
        }
        struct Node* last = secondLast->next;
        secondLast->next = *head;
        free(last);
    }
}

// Function to delete a node from a specific position
void deleteAtPosition(struct Node** head, int position)
{
    if (*head == NULL) {
        return;
    }
    else if (position == 0) {
        deleteFromBeginning(head);
    }
    else {
        struct Node* temp = *head;
        int i = 0;
        while (i < position - 1) {
            temp = temp->next;
            i++;
        }
        struct Node* toDelete = temp->next;
        temp->next = temp->next->next;
        free(toDelete);
    }
}

// Function to traverse and print the circular linked list
void traverse(struct Node* head)
{
    if (head == NULL) {
        return;
    }
    struct Node* temp = head;
    do {
        printf("%d -> ", temp->data);
        temp = temp->next;
    } while (temp != head);
    printf("HEAD\n");
}

// Function to search for a node with a given key
int search(struct Node* head, int key)
{
    if (head == NULL) {
        return 0;
    }
    struct Node* temp = head;
    do {
        if (temp->data == key) {
            return 1; // Key found
        }
        temp = temp->next;
    } while (temp != head);
    return 0; // Key not found
}

// Driver program
int main()
{
    struct Node* head = NULL;

    // Insertion
    insertAtEnd(&head, 10);
    insertAtEnd(&head, 20);
    insertAtBeginning(&head, 5);
    insertAtPosition(&head, 15, 2);

    // Traversal
    printf("Circular Linked List: ");
    traverse(head);

    // Deletion
    deleteFromEnd(&head);
    deleteAtPosition(&head, 1);

    // Traversal after deletion
    printf("Circular Linked List after deletion: ");
    traverse(head);

    // Searching
    int key = 10;
    if (search(head, key)) {
        printf("Element %d is found in the linked list.\n",
               key);
    }
    else {
        printf(
            "Element %d is not found in the linked list.\n",
            key);
    }

    return 0;
}