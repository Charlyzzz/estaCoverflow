/* primitivas.c
 *
 *  Created on: Jun 15, 2014
 *      Author: root
 */

#include "primitivas.h"
#include "common_execution.h"
#include <ctype.h>
#include "globals.h"

/* ========================================================================================
 * ============================Uso Comun en todo el .c=====================================
 * ========================================================================================
 */

t_pc buscarEtiqueta(t_nombre_etiqueta etiqueta) {
	t_pc new_programCounter = metadata_buscar_etiqueta(etiqueta,
			diccionarioEtiquetas, currentPCB->tamanioIndiceEtiquetas);
	return new_programCounter; // --?
}

void resetContextAndSetStackPointerTakingCareOfTheExtraBytesThatShouldNotBeThere(
		t_nombre_etiqueta etiqueta, int extraBytes) {
	stack_changeContext(&(currentPCB->currentStack), extraBytes);
//	currentPCB->programCounter = buscarEtiqueta(etiqueta);
	irALabel(etiqueta);
	clean_variable_dictionary();
	diccionarioDeVariables = dictionary_create();
}

void change_context_common_push(stack* stack) {
	//--------------------
	//Stackout current stack pointer
	//--------------------
	stack_push(stack, &stack->stack_pointer, sizeof(t_puntero), umvSocket,
			stack_offsetFromContext(stack), ejecutarSiSePerdioConexion,
			ejecutarSiHuboStackOverflow);

	//--------------------
	//Stackout PC
	//--------------------

	stack_push(stack, &currentPCB->programCounter, sizeof(t_pc), umvSocket,
			stack_offsetFromContext(stack) - 1, ejecutarSiSePerdioConexion,
			ejecutarSiHuboStackOverflow);
}
//string[fin] == '\n' || string[fin] == '\0' || string[fin] == '\t'
char* removeNewLine(char* string) {
	int fin = strlen(string);
	while (!isalnum(string[fin])) {
		fin--;
	}
	fin++;
	return string_substring_until(string, fin);
}

/* ========================================================================================
 * ========================================================================================
 * ========================================================================================
 */

t_puntero definir_variable(t_nombre_variable nombre) {
	if (estadoPermisivo(estado)) {

	char* key = charToString(nombre);
	stack_push(&(currentPCB->currentStack), &nombre, sizeof(t_nombre_variable),
			umvSocket, stack_offsetFromContext(&(currentPCB->currentStack)),
			ejecutarSiSePerdioConexion, ejecutarSiHuboStackOverflow);
	t_puntero* i = conitos_malloc(sizeof *i);
	*i = stack_lastVariablePosition(&(currentPCB->currentStack));
	dictionary_put(diccionarioDeVariables, key, i);
	free(key);
	return stack_lastVariablePosition(&(currentPCB->currentStack)) + 1;
	}
	return 0;
}

t_puntero obtenerPosicionVariable(t_nombre_variable nombre) {
	if (estadoPermisivo(estado)) {
		char* key = charToString(nombre);
		t_puntero* posicionTotal = dictionary_get(diccionarioDeVariables, key);
		free(key);
		if (posicionTotal != NULL) {
			t_puntero posicion = *posicionTotal + 1; //+1 para excluir el caracter
			return posicion;
		}
	}
	return -1;
}

t_valor_variable desreferenciar(t_puntero direccion) {
	if (estadoPermisivo(estado)) {
		t_datosEnviar* paquete = paquete_leerUMV(
				(currentPCB->currentStack.base),
				direccion - currentPCB->currentStack.base,
				sizeof(t_valor_variable));
		t_datosEnviar* respuesta = enviar_y_esperar_respuesta_con_paquete(
				umvSocket, paquete, ERROR_FALLO_SEGMENTO,
				ejecutarSiSePerdioConexion, ejecutarSiHuboSegmentationFault);
		destruirPaquete(paquete);
		t_valor_variable retorno = *((t_valor_variable*) respuesta->data);
		destruirPaquete(respuesta);
		return retorno;
	}
	return 0;
}

void asignar(t_puntero direccion, t_valor_variable valor) {
	if (estadoPermisivo(estado)) {
		t_datosEnviar* paquete = paquete_escribirUMV(
				(currentPCB->currentStack.base),
				direccion - (currentPCB->currentStack.base),
				sizeof(t_valor_variable), &valor);
		t_datosEnviar* nuevo_paquete = enviar_y_esperar_respuesta_con_paquete(
				umvSocket, paquete, ERROR_FALLO_SEGMENTO,
				ejecutarSiSePerdioConexion, ejecutarSiHuboStackOverflow);
		destruirPaquete(paquete);
		destruirPaquete(nuevo_paquete);
	}
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {
	if (estadoPermisivo(estado)) {
		char* variable_request = removeNewLine(variable);
		t_datosEnviar* respuesta = enviar_y_esperar_respuesta(pcpSocket,
				variable_request, CPU_PCP_PEDIR_VARIABLE_COMPARTIDA,
				string_length(variable_request) + 1, NO_HAY_ERROR_POSIBLE,
				ejecutarSiSePerdioConexion, noHacerNada);
		t_valor_variable retorno = *(t_valor_variable*) respuesta->data;
		destruirPaquete(respuesta); // otra vez :D
		free(variable_request);
		return retorno;
	}
	return 0;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,
		t_valor_variable valor) {
	if (estadoPermisivo(estado)) {
		char* variable_request = removeNewLine(variable);
		t_datosEnviar* paquete = paquete_asignarCompartida(variable_request,
				valor);
		common_send(pcpSocket, paquete, ejecutarSiSePerdioConexion);
		destruirPaquete(paquete);
		return valor;
	}
	return 0;
}

void irALabel(t_nombre_etiqueta etiqueta) {
	if (estadoPermisivo(estado)) {
		char* etiquetaABuscar = removeNewLine(etiqueta);
		currentPCB->programCounter = buscarEtiqueta(etiquetaABuscar);
		free(etiquetaABuscar);
	}
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {
	if (estadoPermisivo(estado)) {
		change_context_common_push(&(currentPCB->currentStack));
		//--------------------
		//Reset contextSize and set new stack_pointer
		//--------------------
		//Tiene 2 bytes de mas asi que..
		resetContextAndSetStackPointerTakingCareOfTheExtraBytesThatShouldNotBeThere(
				etiqueta, 2);
	}

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar) {
	if (estadoPermisivo(estado)) {
		change_context_common_push(&(currentPCB->currentStack));
		//--------------------
		//Stackout return address
		//--------------------
		stack_push(&(currentPCB->currentStack), &dondeRetornar,
				sizeof(t_puntero), umvSocket,
				stack_offsetFromContext(&(currentPCB->currentStack)) - 2,
				ejecutarSiSePerdioConexion, ejecutarSiHuboStackOverflow);
		//--------------------
		//Reset contextSize and set new stack_pointer
		//--------------------
		//Tiene 3 bytes de mas asi que..
		resetContextAndSetStackPointerTakingCareOfTheExtraBytesThatShouldNotBeThere(
				etiqueta, 3);
	}
}

void finalizar() {
	if (estadoPermisivo(estado)) {
		if (currentPCB->currentStack.base
				== currentPCB->currentStack.stack_pointer) {
			changeState(END_PROCESS);
			terminarProceso(NULL, 0);
		} else {
			t_pc* programCounter = stack_pop(&(currentPCB->currentStack),
					-sizeof(t_pc), sizeof(t_pc), umvSocket,
					ejecutarSiSePerdioConexion,
					ejecutarSiHuboSegmentationFault);
			t_puntero* stack_pointer = stack_pop(&(currentPCB->currentStack),
					-sizeof(t_pc) - sizeof(t_puntero), sizeof(t_puntero),
					umvSocket, ejecutarSiSePerdioConexion,
					ejecutarSiHuboSegmentationFault);
			currentPCB->programCounter = *programCounter;
			stack_resetContext(&currentPCB->currentStack, stack_pointer);
			free(programCounter);
			free(stack_pointer);
			clean_variable_dictionary();
			recreate_variable_dictionary();
		}
	}
}

void retornar(t_valor_variable retorno) {
	if (estadoPermisivo(estado)) {
	t_puntero* direccion_de_retorno = stack_pop(&(currentPCB->currentStack),
			-sizeof(t_puntero), sizeof(t_puntero), umvSocket,
			ejecutarSiSePerdioConexion, ejecutarSiHuboSegmentationFault);
	asignar(*direccion_de_retorno, retorno);
	currentPCB->currentStack.stack_pointer -= sizeof(t_puntero);
	finalizar();
	}
}

//syscall
void imprimir(t_valor_variable valor_mostrar) {
	if (estadoPermisivo(estado)) {
	t_datosEnviar* paquete = pedirPaquete(&valor_mostrar,
	CPU_PCP_IMPRIMIR_VARIABLE, sizeof(t_valor_variable));
	common_send(pcpSocket, paquete, ejecutarSiSePerdioConexion);
	destruirPaquete(paquete);
	}
}

//syscall
void imprimirTexto(char* Texto) {
	if (estadoPermisivo(estado)) {
	char* text_to_send = removeNewLine(Texto);
	t_datosEnviar* paquete = pedirPaquete(Texto, CPU_PCP_IMPRIMIR_TEXTO,
			string_length(Texto) + 1);
	common_send(pcpSocket, paquete, ejecutarSiSePerdioConexion);
	destruirPaquete(paquete);
	free(text_to_send);
	}
}

//syscall
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	if (estadoPermisivo(estado)) {
	t_size sizeString = string_length(dispositivo) + 1;
	void* datosExtra = conitos_malloc(sizeString + sizeof(int));
	memcpy(datosExtra, dispositivo, sizeString);
	memcpy(datosExtra + sizeString, &tiempo, sizeof(int));
	//currentPCB->programCounter++;
	changeState(ENTRADA_SALIDA);
	terminarProceso(datosExtra, sizeString + sizeof(int));
	free(datosExtra);
	}
}

void p_signal(t_nombre_semaforo identificador_semaforo) {
	if (estadoPermisivo(estado)) {
	t_nombre_semaforo semaforoASignalear = removeNewLine(
			identificador_semaforo);
	t_datosEnviar* paquete = pedirPaquete(semaforoASignalear,
	CPU_PCP_SIGNAL, string_length(semaforoASignalear) + 1);
	common_send(pcpSocket, paquete, NULL);
	destruirPaquete(paquete);
	}
}

void p_wait(t_nombre_semaforo identificador_semaforo) {
	if (estadoPermisivo(estado)) {
	t_nombre_semaforo semaforoASignalear = removeNewLine(
			identificador_semaforo);
	t_datosEnviar* paquete = pedirPaquete(semaforoASignalear, CPU_PCP_WAIT,
			string_length(semaforoASignalear) + 1);
	t_datosEnviar* respuesta = enviar_y_esperar_respuesta_con_paquete(pcpSocket,
			paquete, NO_HAY_ERROR_POSIBLE, ejecutarSiSePerdioConexion,
			noHacerNada);
	destruirPaquete(paquete);
	if (respuesta->codigo_Operacion != PCP_CPU_WAIT_OK) {
		changeState(WAIT);
		terminarProceso(NULL, 0);
	}
	destruirPaquete(respuesta);
	}
}
