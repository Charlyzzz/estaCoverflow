/* stack.h
 *
 *  Created on: Jun 15, 2014
 *      Author: root
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>
#include <stdlib.h>
#include "decoracionDePaquetesCPU.h"
#include <conitos-estaCoverflow/conitos_generic.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <parser/parser.h>
#include "estructuras.h"
#include "globals.h"


stack* create_stack(int, int);
void stack_push(stack* stack, void* data, t_size data_size, int umvSocket,
		int32_t offset, void (*executeIfError)(),
		void (*executeIfStackOverflow)());
void* stack_pop(stack* stack, int32_t offset, t_size tamanio, int umvSocket,
		void (*executeIfError)(), void (*executeIfSegmentationFault)());
t_puntero stack_lastVariablePosition(stack* stack);
int stack_offsetFromContext(stack* stack);
void stack_changeContext(stack* stack, int extraBytes);
void stack_resetContext(stack* stack,t_puntero* stackPointer);

#endif /* STACK_H_ */
