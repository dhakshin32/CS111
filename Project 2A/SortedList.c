/*
NAME: Dhakshin Suriakannu
EMAIL: bruindhakshin@g.ucla.edu
ID: 605280083
*/

#include "SortedList.h"
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	if (list == NULL || element == NULL) {
        	return;
	}

	if (list->next == list) {
        
        if (opt_yield & INSERT_YIELD) {
          	sched_yield();
        }

		list->next = element;
		list->prev = element;
		element->prev = list;
		element->next = list;
        return;
	}
	
	SortedListElement_t* ptr = list->next;
	SortedListElement_t* pre = list;
	
   	while (ptr != list && strcmp(element->key, ptr->key) >= 0 ){
			pre = ptr;
        	ptr = ptr->next;
    } 

    if (opt_yield & INSERT_YIELD) {
        sched_yield();
	}
	
	ptr->prev=element;
	pre->next=element;
	element->next = ptr;
	element->prev = pre;
    
}

int SortedList_delete(SortedListElement_t *element) {
	if (element == NULL) {
        	return 1;
	}

    if (opt_yield & DELETE_YIELD) {
        sched_yield();
    }

    if (element->next != NULL) {
		if (element->next->prev == element) {
            element->next->prev = element->prev;
        } else {
            return 1;
        }
    }

    if (element->prev != NULL) {
        if (element->prev->next == element) {
           	element->prev->next = element->next;
        } else {
			return 1;
        }
	}
	return 0;

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    	if (list == NULL||key==NULL||list->next==NULL) {
        	return NULL;
    	}

    	SortedListElement_t* ptr = list->next;

		do{
			if (opt_yield & LOOKUP_YIELD) {
          		sched_yield();
        	}
        	if (strcmp(key, ptr->key) == 0) {
            		return ptr;
        	}
       	 	ptr = ptr->next;
		}while(ptr != list);
    	
    	return NULL;
}

int SortedList_length(SortedList_t *list) {
    	if (list == NULL) {
        	return -1;
    	}

    	int num = 0;
    	SortedListElement_t* ptr = list->next;

		while(ptr!=list){
			if (opt_yield & LOOKUP_YIELD) {
          		sched_yield();
        	}
			num++;
			ptr = ptr->next;
		}

    	return num;
}