/* stack.c
 *
 *  Created on: Jun 15, 2014
 *      Author: root
 */

#include "stack.h"



/************************************************************************************************************************************************************************************************************
 *
 * Se que hay continuations que capaz convenian para este sistema que esten aca, y no que sean continuations, pero lo quiero
 * dejar lo mas puro posible, ya suficiente el umv_segmentation_fault
 *
 ************************************************************************************************************************************************************************************************************
 */

#define tamanio_registro_variable (sizeof(t_valor_variable) + sizeof(t_nombre_variable))

stack* create_stack(int stackStart, int size) {
	stack* stack = conitos_malloc(sizeof(stack));
	stack->base = stackStart;
	stack->stack_pointer = stackStart;
	stack->contextSize = 0;
	return stack;
}

void stack_push(stack* stack, void* data, t_size data_size, int umvSocket,
		int32_t offset, void (*executeIfError)(),
		void (*executeIfStackOverflow)()) {

	t_datosEnviar* datos = paquete_escribirUMV(stack->base,
			stack->stack_pointer - stack->base + offset, data_size, data);

	t_datosEnviar* respuesta = enviar_y_esperar_respuesta_con_paquete(umvSocket,
			datos, UMV_SEGMENTATION_FAULT, executeIfError,
			executeIfStackOverflow);
	destruirPaquete(datos);
	destruirPaquete(respuesta);
	stack->contextSize++;
}

void* stack_pop(stack* stack, int32_t offset, t_size tamanio, int umvSocket,
		void (*executeIfError)(), void (*executeIfSegmentationFault)()) {
	t_datosEnviar* datos = paquete_leerUMV(stack->base,
			stack->stack_pointer - stack->base + offset, tamanio);
	t_datosEnviar* respuesta = enviar_y_esperar_respuesta_con_paquete(umvSocket,
			datos, UMV_SEGMENTATION_FAULT, executeIfError,
			executeIfSegmentationFault);
	destruirPaquete(datos);
	void* retorno = respuesta->data;
	free(respuesta);
	return retorno;
}

void stack_destroy(stack* stack) {
	free(stack);
}

int stack_offsetFromContext(stack* stack) {
	return (stack->contextSize) * tamanio_registro_variable;
}

t_puntero stack_lastVariablePosition(stack* stack) {
	return stack->stack_pointer + stack_offsetFromContext(stack)
			- tamanio_registro_variable;
}

void stack_changeContext(stack* stack, int extraBytes) {
	currentPCB->currentStack.stack_pointer =
			currentPCB->currentStack.stack_pointer
					+ (stack_offsetFromContext(stack) - (extraBytes));
	currentPCB->currentStack.contextSize = 0;
}

void stack_resetContext(stack* stack,t_puntero* stackPointer){
	stack->contextSize = ((stack->stack_pointer - sizeof(t_pc) - sizeof(t_puntero)) - *stackPointer) / 5;
	stack->stack_pointer = *stackPointer;
}
