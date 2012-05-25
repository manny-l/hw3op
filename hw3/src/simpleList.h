/*
 * simpleList.h
 *
 *  Created on: May 18, 2012
 *      Author: root
 */

#ifndef SIMPLELIST_H_
#define SIMPLELIST_H_

headp listInit();

int listIsEmpty(headp head);

void listInsert(headp head, commandType comm,
							int key,
							int value);

#endif /* SIMPLELIST_H_ */
