/*
 * accionesParaCPU.h
 *
 *  Created on: Jul 15, 2014
 *      Author: root
 */

#ifndef ACCIONESPARACPU_H_
#define ACCIONESPARACPU_H_

#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "globals.h"
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <poll.h>
#include <errno.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <signal.h>
#include "semaforos.h"

void cpuFinishesQuantum(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void signalRequest(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void waitRequest(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void cpuFinishesProcess(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void sharedVariableRequest(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void sharedVariableAssign(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void printText(t_nodo_proceso_ejecutando*, fd_set*, t_datosEnviar*);
void printVariable(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void exceptionCPU(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void cpuDisconnects(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);
void inOut(t_nodo_proceso_ejecutando*, fd_set*, t_datosEnviar*);
void cpuSIGINT(t_nodo_proceso_ejecutando* procesoEjecutando, fd_set* socketsCPU, t_datosEnviar* paquete);

#endif /* ACCIONESPARACPU_H_ */
