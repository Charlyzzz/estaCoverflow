/* common_execution.c
 *
 *  Created on: Jul 8, 2014
 *      Author: root
 */

#include "common_execution.h"

bool estadoPermisivo(state estado) {
	return (estado == NORMAL || estado == KILL_CPU);
}

void terminarProceso(void* datosExtra, t_size sizeExtra) {
	char* translate = string_itoa(estado);
	t_action accion = dictionary_get(dictionaryCommand,translate);
	accion(datosExtra,sizeExtra);
	clean_variable_dictionary();
	proceso_activo = false;
	free(translate);
}

bool isDisconnectionState(state estado) {
	return (estado == KILL_CPU);
}

void changeState(state toState){
	if (!isDisconnectionState(estado)) {
		estado = toState;
	}
}

void ejecutarSiSePerdioConexion() {

}

void ejecutarSiHuboSegmentationFault() {
	char* mensaje = strdup("SEGMENTATION FAULT");
	changeState(SEG_FAULT);
	terminarProceso(mensaje,strlen(mensaje) + 1);
	free(mensaje);
}

void clean_variable_dictionary() {
	dictionary_destroy_and_destroy_elements(diccionarioDeVariables, free);
}

void recreate_variable_dictionary() {
		uint32_t offset, tamanio;
		diccionarioDeVariables = dictionary_create();
		tamanio = currentPCB->currentStack.contextSize * (sizeof(t_valor_variable) + sizeof(t_nombre_variable));
		offset = currentPCB->currentStack.stack_pointer
				- currentPCB->currentStack.base; //TODO verificar que esto este bien
		t_datosEnviar* aux = solicitar_algo_UMV(currentPCB->currentStack.base, offset, tamanio); //FIXME ver esto
		llenar_diccionario(aux->data);
		destruirPaquete(aux);
}


char* charToString(char element) {
	char* new = conitos_malloc(2);
	*new = element;
	*(new + 1) = '\0';
	return new;
}

void ejecutarSiHuboStackOverflow() {
	char* mensaje = strdup("STACK OVERFLOW");
	changeState(STK_OVERFLOW);
	terminarProceso(mensaje,strlen(mensaje) + 1);
	free(mensaje);
}

void noHacerNada() {

}

t_datosEnviar* solicitar_algo_UMV(t_puntero base, int32_t offset, uint32_t tamanio) {
	t_datosEnviar* paquete = paquete_leerUMV(base, offset, tamanio);
	common_send(umvSocket, paquete, NULL);
	destruirPaquete(paquete);
	paquete = common_receive(umvSocket, NULL);
	if (paquete->codigo_Operacion != UMV_MENSAJE_CPU_LEER) {
		log_debug(logger,"Hubo un error de lectura, cierro la ejecución del programa");
		ejecutarSiHuboSegmentationFault();
	} else {
		return paquete;
	}
	destruirPaquete(paquete);
	return NULL;
}

void llenar_diccionario(char* informacion_diccvar){
	int aux, offset;
		aux = 0;
		char nombre_variable;
		char* key;
		t_valor_variable* value;
		while (aux < currentPCB->currentStack.contextSize) {
			offset = aux * (sizeof(t_valor_variable) + sizeof(t_nombre_variable));
			(aux)++;
			memcpy(&nombre_variable,informacion_diccvar + offset,sizeof(char));
			key = charToString(nombre_variable);
			value = conitos_malloc(sizeof *value);
			(*value) = currentPCB->currentStack.stack_pointer + offset;
			dictionary_put(diccionarioDeVariables, key, value);
			free(key);
		}


}
