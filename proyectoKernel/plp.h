/*
 * plp.h
 *
 *  Created on: 17/07/2014
 *      Author: utnso
 */

#ifndef PLP_H_
#define PLP_H_

#include "estructuras.h"
#include "globals.h"
#include <stdio.h>
#include <stdbool.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <parser/metadata_program.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>


void * threadPLP();
void encolarPCB(t_pcb *, int, int);
int generarPid(int*);
bool algoritmoSJF(void *, void *);
void* hiloDestruccion();
void* hiloMultiprogramacion();
void atenderProgramaEntrante(t_datosEnviar* paquete, int socketCliente,
		int * PIDActual);

int pedirSegmento(int * error,int tamanio, int pid);


#endif /* PLP_H_ */
