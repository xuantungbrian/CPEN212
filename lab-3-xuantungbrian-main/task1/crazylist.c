#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "crazylist.h"
#include <stdio.h>

crazycons_t *enclosing_struct(uint64_t *car) {
	crazycons_t *cons = (crazycons_t *) malloc(sizeof(crazycons_t));
	return (crazycons_t *) ((void *) car - sizeof(cons->cdr));
}

uint64_t *cons(uint64_t car, uint64_t *cdr) {
    crazycons_t *cons = (crazycons_t *) malloc(sizeof(crazycons_t));
    assert(cons);
    cons->car = car;
    cons->cdr = cdr;
    assert(cons);
    return (uint64_t *) &cons->car;
}

uint64_t first(uint64_t *list) {
	return *list;
}

uint64_t *rest(uint64_t *list) {
	return enclosing_struct(list)->cdr;
	
}

uint64_t *find(uint64_t *list, uint64_t query) {
	uint64_t *list1 = list;
	int i=0;
	if (list==NULL) return NULL;
	while (1) {
		if (first(list1) == query) {
			break;
		}
		list1 = rest(list1);
		if (list1 == NULL) {
			i=1;
			break;
		}
	}
	if (i==1) return NULL;
	else return list1;
}

uint64_t *insert_sorted(uint64_t *list, uint64_t n) { 
	uint64_t *list1 = list;
	uint64_t *list2;
	while (1) {
		if (list == NULL) {
			list2 = cons(n, NULL);
			return list2;
		}
		if (first(rest(list)) >= n) {
                        list2 = cons(n, list);
                        break;
                }
		else if (rest(list1) == NULL) {
			list2 = cons(n, NULL);
			enclosing_struct(list1)->cdr  = list2;
                        break;
		}
		else if (first(rest(list1)) >= n &&  rest(list1) != NULL) {
                        list2 = cons(n, rest(list1));
                        enclosing_struct(list1)->cdr  = list2;
                        break;
                }

		list1=rest(list1);
	}
	if (list1==list) return list2;
	else return list;
}

void print_list(uint64_t *list) {
	uint64_t *list1 = list;
	while (list!=NULL) {
		printf("%" PRIu64 " " ,first(list1));
		list1 = rest(list1);
		if (list1 == NULL) break;
	}
	printf("\n");
}

