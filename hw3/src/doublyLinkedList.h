/*
 * doublyLinkedList.h
 *
 */

#ifndef DOUBLYLINKEDLIST_H_
#define DOUBLYLINKEDLIST_H_

#include <stdbool.h>

typedef struct DoublyLinkedList_t* DoublyLinkedList;

void Initialize();
void Destroy();
bool InsertHead (int key, char data);
bool InsertTail(int key, char data);
bool Delete(int key);
bool Search(int key, char* data);

#endif /* DOUBLYLINKEDLIST_H_ */
