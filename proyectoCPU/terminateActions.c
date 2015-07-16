#include "terminateActions.h"

t_datosEnviar* empaquetarPCB(int codOp) {
	return pedirPaquete(currentPCB, codOp, sizeof(t_pcb));
}

void imprimirDiccionario(char* key, void *data) {
		uint32_t offset;
		t_puntero direccion= *(t_puntero*)data;
		offset = direccion + 1 - currentPCB->currentStack.base;
		t_datosEnviar* mensaje = solicitar_algo_UMV(currentPCB->currentStack.base, offset,
				sizeof(t_valor_variable));
		log_info(logger, " Variable:  %s \n Valor:  %d", key, *(t_valor_variable*)(mensaje->data));
		destruirPaquete(mensaje);
}

void terminaFinQuantum(void* dataExtra, t_size extraSize) {
	t_datosEnviar* paquete = empaquetarPCB(CPU_PCP_FIN_QUANTUM);
	log_info(logger, "Process finished a quantum.");
	if (currentPCB->currentStack.contextSize > 0) {
	log_info(logger, "Current variable state on context:\n ");
	dictionary_iterator(diccionarioDeVariables,imprimirDiccionario);
	} else {
		log_debug(logger,"No variables in current context.");
	}
	common_send(pcpSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaExcepcion(void* dataExtra, t_size extraSize) {
	char* error_msg = (char*) dataExtra;
	t_datosEnviar* paquete = empaquetarPCB(CPU_PCP_EXCEPTION);
	log_info(logger, "Process finished with an exception: %s", error_msg);
	aniadirAlPaquete(paquete, error_msg, extraSize);
	common_send(pcpSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaBlockedIO(void* dataExtra, t_size extraSize) {
	t_datosEnviar* paquete = empaquetarPCB(CPU_PCP_ENTRADA_SALIDA);
	aniadirAlPaquete(paquete, dataExtra, extraSize);
	common_send(pcpSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaBlockedSemaphore(void* dataExtra, t_size extraSize) {
	t_datosEnviar* paquete = empaquetarPCB(CPU_PCP_PROCESS_WAITING);
	common_send(pcpSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaFinishedProcess(void* dataExtra, t_size extraSize) {
	t_datosEnviar* paquete = empaquetarPCB(CPU_PCP_FIN_PROCESO);
	log_debug(logger, "Process %d finished executing", currentPCB->pid);
	dictionary_iterator(diccionarioDeVariables, (void*) imprimirDiccionario);
	common_send(pcpSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminarCPUSinPCB() {
	t_datosEnviar* info1 = pedirPaquete("a", CPU_PCP_DISCONNECTION, 2);
	t_datosEnviar* info = pedirPaquete("a", UMV_CAE_CPU, 2);
	common_send(umvSocket, info, NULL );
	common_send(pcpSocket, info1, NULL );
	destruirPaquete(info1);
	destruirPaquete(info);
	close(umvSocket);
	close(pcpSocket);
	config_destroy(configuracion_CPU);
	exit(0);
}

void terminarCPU(void* dataExtra, t_size size) {
	if (proceso_activo) {
		t_datosEnviar* pcbAEnviar = empaquetarPCB(CPU_PCP_DISCONNECTION);
		common_send(pcpSocket, pcbAEnviar, NULL );
		destruirPaquete(pcbAEnviar);
	} else {
           t_datosEnviar* cerrar = pedirPaquete("a",CPU_PCP_DISCONNECTION,2);
           common_send(pcpSocket,cerrar,NULL);
           destruirPaquete(cerrar);
           }
           t_datosEnviar* info = pedirPaquete("a", UMV_CAE_CPU, 2);
           common_send(umvSocket, info, NULL );
           destruirPaquete(info);
           close(umvSocket);
	       close(pcpSocket);
	       clean_variable_dictionary();
	       free(currentPCB);
	       config_destroy(configuracion_CPU);
	       exit(0);
         
}
