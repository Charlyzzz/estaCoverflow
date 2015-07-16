#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "globals.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <poll.h>
#include <errno.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <string.h>
#include <signal.h>
#include "semaforos.h"
#include "entradaSalida.h"
#include "accionesParaCPU.h"

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

void cpuFinishesQuantum(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_pcb* tmp = unserializePCB(paquete->data);
	procesoEjecutando->proceso.pcb = *tmp;
	stopProcessing(procesoEjecutando);
	addToReadyQueue(&(procesoEjecutando->proceso));
	free(procesoEjecutando);
	free(tmp);
}

void signalRequest(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_semaforo* semaforo = dictionary_get(dictionarySemaphores, paquete->data);
	semaforo_signal(semaforo);
}

void sendWaitAnswer(int cpuSocket, int codOp)
{
	char* mensajeComun = strdup("o");
	t_datosEnviar* mensaje = pedirPaquete(mensajeComun, codOp,
			string_length(mensajeComun) + 1);
	common_send(cpuSocket, mensaje, NULL );
	destruirPaquete(mensaje);
}

t_pcb* onWaitGetProcessFromCPU(int socket)
{
	t_datosEnviar* respuesta = common_receive(socket, NULL );
	if (respuesta->codigo_Operacion == CPU_PCP_PROCESS_WAITING)
	{
		t_pcb* nuevo = unserializePCB(respuesta->data);
		destruirPaquete(respuesta);
		return nuevo;
	}
	destruirPaquete(respuesta);
	return NULL ;
}

void onWaitUpdatePCBAndCPU(t_pcb* nuevo,
		t_nodo_proceso_ejecutando* procesoEjecutando, t_semaforo* semaforo)
{
	procesoEjecutando->proceso.pcb = *nuevo;

	stopProcessing(procesoEjecutando);
}

void waitRequest(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_semaforo* semaforo = dictionary_get(dictionarySemaphores, paquete->data);
	if (semaforo_wait(semaforo, procesoEjecutando))
	{
		sendWaitAnswer(procesoEjecutando->cpu.socket, PCP_CPU_WAIT_NO_OK);
		t_pcb* nuevo = onWaitGetProcessFromCPU(procesoEjecutando->cpu.socket);
		onWaitUpdatePCBAndCPU(nuevo, procesoEjecutando, semaforo);
		blockProcess(semaforo->bloqueados, procesoEjecutando);
		free(nuevo);
	}
	else
	{
		sendWaitAnswer(procesoEjecutando->cpu.socket, PCP_CPU_WAIT_OK);
	}
}

void cpuFinishesProcess(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
  log_info(log_kernel,"Se termino de correr el proceso PID %d",procesoEjecutando->proceso.pcb.pid);
	t_pcb* pcb = unserializePCB(paquete->data);
	procesoEjecutando->proceso.pcb = *pcb;
	pcp_exitProcess(procesoEjecutando);
	free(procesoEjecutando);
	free(pcb);
}

void sharedVariableRequest(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{

	t_varCompartida* varBuscada = dictionary_get(dictionarySharedVariables,
			(char*) paquete->data);
	t_datosEnviar* mensaje = pedirPaquete(&varBuscada->valor, PCP_CPU_OK,
			sizeof(uint32_t));
	common_send(procesoEjecutando->cpu.socket, mensaje, NULL );
	destruirPaquete(mensaje);
}

void sharedVariableAssign(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{

	uint32_t valorVarCompartida;
	char* nombreVarCompartida = malloc(paquete->data_size - sizeof(uint32_t));
	memcpy(nombreVarCompartida, paquete->data,
			paquete->data_size - sizeof(uint32_t));
	memcpy(&valorVarCompartida,
			paquete->data + paquete->data_size - sizeof(uint32_t),
			sizeof(uint32_t));
	t_varCompartida* varBuscada = dictionary_get(dictionarySharedVariables,
			nombreVarCompartida);
	varBuscada->valor = valorVarCompartida;
	free(nombreVarCompartida);
	actualizarEstado();
}

void printText(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_datosEnviar* mensaje = pedirPaquete(paquete->data,
			PROGRAMA_PAQUETE_PARA_IMPRIMIR, paquete->data_size);
	common_send(procesoEjecutando->proceso.soquet_prog, mensaje, NULL );
	destruirPaquete(mensaje);
}

void printVariable(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	char* traduccion = string_itoa(*((int*) paquete->data));
	t_datosEnviar* mensaje = pedirPaquete(traduccion,
			PROGRAMA_PAQUETE_PARA_IMPRIMIR, string_length(traduccion) + 1);
	common_send(procesoEjecutando->proceso.soquet_prog, mensaje, NULL );
	destruirPaquete(mensaje);
	free(traduccion);
}

void inOut(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_pcb* pcb = unserializePCB(paquete->data);
	t_io* datos = unserializeIO(paquete->data + (sizeof *pcb),
			paquete->data_size - (sizeof *pcb));
	procesoEjecutando->proceso.pcb = *pcb;
	stopProcessing(procesoEjecutando);
	doInOut(procesoEjecutando, datos);
	free(procesoEjecutando);
	free(datos);
	free(pcb);
}

void exceptionCPU(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{

	t_pcb* pcb = unserializePCB(paquete->data);
	char* textoError = malloc(paquete->data_size - sizeof(t_pcb));
	memcpy(textoError, paquete->data + sizeof(t_pcb),
			paquete->data_size - sizeof(t_pcb));


	log_warning(log_kernel, "Excepcion en el proceso %d. Los detalles de la excepcion: %s",procesoEjecutando->cpu.pid,textoError);
	procesoEjecutando->proceso.pcb = *pcb;
	t_datosEnviar* mensaje = pedirPaquete(textoError,
			PROGRAMA_PAQUETE_PARA_IMPRIMIR, string_length(textoError) + 1);
	common_send(procesoEjecutando->proceso.soquet_prog, mensaje, NULL );
	destruirPaquete(mensaje);
	pcp_exitProcess(procesoEjecutando);
	free(procesoEjecutando);
	free(textoError);
}

void cpuDisconnects(t_nodo_proceso_ejecutando* procesoEjecutando,
		fd_set* socketsCPU, t_datosEnviar* paquete)
{
	t_pcb* tmp = unserializePCB(paquete->data);
	log_info(log_kernel,
			"Se desconecta por señal SIGUSR1 la CPU %d.",
			procesoEjecutando->cpu.pid);
	procesoEjecutando->proceso.pcb = *tmp;
	removeFromExecutingList(procesoEjecutando);
	removeCPUFromKernel(procesoEjecutando->cpu.socket,socketsCPU);
	addToReadyQueue(&(procesoEjecutando->proceso));
	free(procesoEjecutando);
	free(tmp);
}

void cpuSIGINT(t_nodo_proceso_ejecutando* procesoEjecutando, fd_set* socketsCPU, t_datosEnviar* paquete) {
	t_pcb* tmp = unserializePCB(paquete->data);
	log_info(log_kernel,
	                        "Se desconecta por señal SIGINT la CPU %d.",
	                        procesoEjecutando->cpu.pid);
	procesoEjecutando->proceso.pcb = *tmp;
		removeFromExecutingList(procesoEjecutando);
		removeCPUFromKernel(procesoEjecutando->cpu.socket,socketsCPU);
		removeProcess(procesoEjecutando);
		free(procesoEjecutando);
		free(tmp);
}
