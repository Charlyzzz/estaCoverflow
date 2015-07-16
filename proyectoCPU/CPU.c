#include "CPU.h"
#define HANDSHAKE_UMV 2
#define CPU_PCP_SIGINT 92

AnSISOP_funciones funciones =
		{ .AnSISOP_asignar = asignar, .AnSISOP_definirVariable =
				definir_variable, .AnSISOP_dereferenciar = desreferenciar,
				.AnSISOP_finalizar = finalizar, .AnSISOP_imprimir = imprimir,
				.AnSISOP_imprimirTexto = imprimirTexto, .AnSISOP_irAlLabel =
						irALabel, .AnSISOP_llamarConRetorno = llamarConRetorno,
				.AnSISOP_llamarSinRetorno = llamarSinRetorno,
				.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
				.AnSISOP_retornar = retornar, .AnSISOP_obtenerValorCompartida =
						obtenerValorCompartida,
				.AnSISOP_asignarValorCompartida = asignarValorCompartida,
				.AnSISOP_entradaSalida = entradaSalida, };

AnSISOP_kernel funciones_kernel = { .AnSISOP_wait = p_wait, .AnSISOP_signal =
		p_signal };

char *diccionarioEtiquetas;
uint32_t retardo_asignado;
t_dictionary* diccionarioDeVariables;
t_pcb* currentPCB;
int pcpSocket;
int umvSocket;
t_log* logger;
state estado;
t_config *configuracion_CPU;
t_dictionary* dictionaryCommand;
bool desconexion = false;
bool proceso_activo = false;
sem_t semaforoSIGINT;

typedef struct {
	uint32_t quantum;
	uint32_t retardo;
} t_datos_kernel;

void createConfig(char* configPath) {
	configuracion_CPU = config_create(configPath);
}

void senialUsuario(int signal) {
	if (proceso_activo) {
		estado = KILL_CPU;
		//desconexion = true;
	} else {
		terminarCPUSinPCB(NULL, 0);
	}
}

void senialInterrupcion(int signal) {
	/*if (proceso_activo) {
		estado = KILL_CPU;

	} else {
		terminarCPUSinPCB(NULL, 0);
	}*/
	//sem_wait(&semaforoSIGINT);
	if (proceso_activo) {
	t_datosEnviar* info1 = pedirPaquete(currentPCB, CPU_PCP_SIGINT, sizeof(t_pcb));
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
	terminarCPUSinPCB(NULL,0);
}

void setupSignals() {
	signal(SIGUSR1, senialUsuario);
	signal(SIGINT, senialInterrupcion);
	//sem_init(&semaforoSIGINT,0,1);
}

void destroyOnConnectionFailure() {
	close(umvSocket);
	close(pcpSocket);
	log_destroy(logger);
	config_destroy(configuracion_CPU);
	exit(EXIT_FAILURE);
}

void doHandshakeUMV() {
	t_datosEnviar* paquete = pedirPaquete("a", HANDSHAKE_UMV, 2);
	common_send(umvSocket, paquete, destroyOnConnectionFailure);
	destruirPaquete(paquete);
	paquete = common_receive(umvSocket, NULL );
	if (paquete->codigo_Operacion != UMV_CONTESTA_CPU_EXITO) {
		log_debug(logger, "Failure handshaking umv");
		destruirPaquete(paquete);
		destroyOnConnectionFailure();
	}
	log_debug(logger, "Successful handshake with UMV");
	destruirPaquete(paquete);
}

void connectToUMV() {
	char* IP_UMV = config_get_string_value(configuracion_CPU, "IPUMV");
	char* puertoUMV = config_get_string_value(configuracion_CPU, "PuertoUMV");
	umvSocket = connect_to(IP_UMV, puertoUMV);
	if (umvSocket == -1) {
		log_debug(logger, "Connection to UMV failed. Ending");
		destroyOnConnectionFailure();
	}
	log_debug(logger, "Connection to UMV is ready, preparing handshake...");
	doHandshakeUMV();
}

t_datos_kernel getKernelData(void* data) {
	t_datos_kernel kernelData;
	memcpy(&kernelData, data, sizeof(uint32_t));
	kernelData.quantum = *(uint32_t*) (data);
	kernelData.retardo = *(uint32_t*) (data + sizeof(uint32_t));
	return kernelData;
}

t_datos_kernel doHandshakeKernel() {
	int pidCPU = getpid();
	t_datosEnviar* paquete = pedirPaquete(&pidCPU, HANDSHAKE, sizeof(int));
	common_send(pcpSocket, paquete, destroyOnConnectionFailure);
	destruirPaquete(paquete);
	paquete = common_receive(pcpSocket, NULL );
	if (paquete->codigo_Operacion != PCP_CPU_OK) {
		log_debug(logger, "Failure handshaking kernel");
		destruirPaquete(paquete);
		destroyOnConnectionFailure();
	}
	log_debug(logger, "Successful handshake with Kernel");
	t_datos_kernel kernelData = getKernelData(paquete->data);
	destruirPaquete(paquete);
	return kernelData;

}

t_datos_kernel connectToKernel() {
	char* IPKernel = config_get_string_value(configuracion_CPU, "IPKernel");
	char* puertoKernel = config_get_string_value(configuracion_CPU,
			"PuertoKernel");
	pcpSocket = connect_to(IPKernel, puertoKernel);
	if (pcpSocket == -1) {
		log_debug(logger, "Failure connecting to Kernel");
		destroyOnConnectionFailure();
	}
	return doHandshakeKernel();
}

t_datos_kernel setupConnections() {
	connectToUMV();
	return connectToKernel();
}

t_pcb* getNewPCB() {
	t_pcb* new = conitos_malloc(sizeof(t_pcb));
	t_datosEnviar* recepcion_PCB = common_receive(pcpSocket, NULL );
	if (recepcion_PCB->codigo_Operacion != PCP_CPU_PROGRAM_TO_EXECUTE) {
		log_debug(logger, "Failure delivering PCB...");
		estado = END_PROCESS;
		return NULL ;
	}
	*new = *(t_pcb*) recepcion_PCB->data;
	destruirPaquete(recepcion_PCB);
	log_debug(logger, "PCB Received correctly. Starting to execute process %d",
			new->pid);
	return new;
}

void resetContextFrom0() {
	recreate_variable_dictionary();
	traer_segmento_etiquetas();
}

#define command(state,function) dictionary_put(dictionaryCommand,string_itoa(state),function);

void setupActionsOnTerminate() {
	dictionaryCommand = dictionary_create();
	command(STK_OVERFLOW, terminaExcepcion)
	command(SEG_FAULT, terminaExcepcion)
	command(WAIT, terminaBlockedSemaphore)
	command(ENTRADA_SALIDA, terminaBlockedIO)
	command(NORMAL, terminaFinQuantum)
	command(KILL_CPU, terminarCPU)
	command(END_PROCESS, terminaFinishedProcess)
}

int main(int argc, char **argv) {
	logger = log_create(argv[2], "CPU", true, LOG_LEVEL_DEBUG);
	log_info(logger, "Log created");
	createConfig(argv[1]);
	setupSignals();
	t_datos_kernel kernelData = setupConnections();
	setupActionsOnTerminate();
	uint32_t quantum_aux;
	estado = NORMAL;
	while (estado != KILL_CPU) {
		quantum_aux = kernelData.quantum;
		estado = NORMAL;
		currentPCB = getNewPCB();
		avisar_proceso_activo();
		resetContextFrom0();
		while (quantum_aux > 0 && estadoPermisivo(estado)) {
			quantum_aux = quantum_aux - 1;
		//	sem_wait(&semaforoSIGINT);
			ejecutarInstruccion(kernelData.retardo);
		//	sem_post(&semaforoSIGINT);
		}
		log_debug(logger, "Stopping execution of process...");
		if (estado == KILL_CPU) {
			log_debug(logger, "CPU Process is going to disconnect...");
			terminarCPU(NULL, 0);
		} else {
			if (proceso_activo) {
				terminarProceso(NULL, 0);
			}
		}
		free(currentPCB);
	}
	terminarCPU(NULL,0);
	return 0;
}

void ejecutarInstruccion(uint32_t retardo_asignado) {
	uint32_t offset_codeIndex = currentPCB->programCounter
			* sizeof(t_intructions);
	t_datosEnviar* infoCodeIndexPacket = solicitar_algo_UMV(
			currentPCB->codeIndex, offset_codeIndex, sizeof(t_intructions));
	t_intructions* infoCodeIndex= (t_intructions*) (infoCodeIndexPacket->data);
	log_debug(logger, "Searching for instruction on...");
	t_datosEnviar* instruccionPaquete = solicitar_algo_UMV(currentPCB->codeSegment,
			infoCodeIndex->start, infoCodeIndex->offset);
	char* instruccionReal = malloc(instruccionPaquete->data_size + 1);
	memcpy(instruccionReal,instruccionPaquete->data,instruccionPaquete->data_size);
	instruccionReal[instruccionPaquete->data_size] = '\0';
	log_debug(logger, "Instruction Found: %s", instruccionReal);

	(currentPCB->programCounter)++;
	analizadorLinea((char * const ) instruccionReal, &funciones,
			&funciones_kernel);

	free(instruccionReal);
	destruirPaquete(instruccionPaquete);
	destruirPaquete(infoCodeIndexPacket);
	usleep(retardo_asignado * 1000);
}

void avisar_proceso_activo() {
	proceso_activo = true;
	t_datosEnviar* paquete = pedirPaquete(&(currentPCB->pid),
			UMV_PROCESO_ACTIVO_CPU, sizeof(uint32_t));
	common_send(umvSocket, paquete, NULL );
	destruirPaquete(paquete);
	paquete = common_receive(umvSocket, NULL );
	if (paquete->codigo_Operacion != UMV_PROCESO_ACTIVO_CPU) {
		log_debug(logger, "No se pudo avisar el proceso activo");
		estado = END_PROCESS;
	}
	destruirPaquete(paquete);
}

void traer_segmento_etiquetas() {
	printf("\n%d\n", currentPCB->tamanioIndiceEtiquetas);
	if (currentPCB->tamanioIndiceEtiquetas != 0) {
		t_datosEnviar* aux = solicitar_algo_UMV(currentPCB->indiceEtiquetas, 0,
				currentPCB->tamanioIndiceEtiquetas);
		diccionarioEtiquetas = realloc(diccionarioEtiquetas,
				currentPCB->tamanioIndiceEtiquetas);
		memcpy(diccionarioEtiquetas, aux->data, currentPCB->tamanioIndiceEtiquetas);
		destruirPaquete(aux);
	}
}
