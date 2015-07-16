/*
 * generic.c
 *
 *  Created on: May 11, 2014
 *      Author: root
 */

#include <stdlib.h>
#include "conitos_generic.h"

void* conitos_malloc(int size) {
	void* return_memory = NULL;
	if (size != 0) {
		// necesito la memoria si o si, so..
		while (return_memory == NULL) {
			return_memory = malloc(size);
		}
	}
	return return_memory;
}
