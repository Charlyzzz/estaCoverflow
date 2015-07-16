/*
 * semaforos.h
 *
 *  Created on: Jul 15, 2014
 *      Author: root
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_
#include <stdbool.h>
#include <stdint.h>
#include "semaforos.h"
#include <stdbool.h>
#include "globals.h"
#include <commons/collections/queue.h>
#include <pthread.h>

typedef struct {
	char *nombre;
	int32_t valor;
	t_queue *bloqueados;
} t_semaforo;

t_semaforo *semaforo_create(char*nombre, int32_t valor);
void semaforo_signal(t_semaforo* semaforo);
bool semaforo_wait(t_semaforo* semaforo, t_nodo_proceso_ejecutando* proceso);
void blockProcess(t_queue* blockedQueue,
		t_nodo_proceso_ejecutando* procesoEjecutando);

#endif /* SEMAFOROS_H_ */
