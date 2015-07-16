/*
 * globals.h
 *
 *  Created on: Jul 12, 2014
 *      Author: root
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <conitos-estaCoverflow/common_sockets.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <semaphore.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include "estructuras.h"
#include <string.h>


extern t_configuracion configuration;
extern int socketUMV;
//listas
extern t_list *listaNuevos;
extern t_queue *listaListos;
extern t_queue *listaTerminados;
extern t_list *listaEjecutando;
extern t_queue *listaCPULibres;
//diccionarios
extern t_dictionary *dictionaryIO;
extern t_dictionary *dictionarySemaphores;
extern t_dictionary *dictionarySharedVariables;
//semaforos
extern sem_t sem_listaCpu;
extern sem_t sem_listaNuevos;
extern sem_t sem_listaListos;
extern sem_t sem_semaforos;
extern sem_t sem_multiprog;
extern sem_t sem_listaTerminados;
extern int pcp_fdMax;
extern pthread_mutex_t mutex_imprimirEstado;
extern pthread_mutex_t mutex_listaEjecutando;
extern pthread_mutex_t mutex_listaCPU;
extern pthread_mutex_t mutex_listaListos;
extern pthread_mutex_t mutex_semaforos;	//FIXME no deberia ser un array? Fijarse en kernel.c
extern pthread_mutex_t mutex_listaTerminados;
extern pthread_mutex_t mutex_listaNuevos;
extern t_dictionary* cpu_command_dictionary;
extern t_log* log_kernel;
extern bool debugOn;

/*
 * ======================================================================================================
 * ======================================================================================================
 * ======================================================================================================
 */
//TODO estaria genial poder parear los mutex con lo que estan bloqueando sinceramente...
void debugTrackPCP(char*, ...);
void debugTrackPLP(char*, ...);
void debugTrackKERNEL(char*, ...);
void stopProcessing(t_nodo_proceso_ejecutando* procesoEjecutando);
void pcp_exitProcess(t_nodo_proceso_ejecutando* proceso);
void addCPUToCPUFreeList(t_cpu* cpu);
void addToReadyQueue(t_nodo_proceso* proceso);
void removeFromExecutingList(t_nodo_proceso_ejecutando* process);
t_pcb* unserializePCB(void* data);
t_nodo_proceso* copyProcessNode(t_nodo_proceso* proceso);
void removeCPUFromKernel(int socketCPU, fd_set* socketsPCP);
void listarListos();
void listarTerminados();
void listarNuevos();
void listarEjecutando();
void listarSemaforos();
void listarCPUs();
void listarIO();
void shutdownKernel();
void actualizarEstado();
void removeProcess(t_nodo_proceso_ejecutando*);
#endif /* GLOBALS_H_ */
