/*
 * entradaSalida.c
 *
 *  Created on: Jul 17, 2014
 *      Author: root
 */
#include "globals.h"
#include "entradaSalida.h"

t_io* unserializeIO(void* data, t_size size)
{
	t_io* nuevo = malloc(sizeof *nuevo);
	t_size name_size = size - sizeof(uint32_t);
	nuevo->nombre = malloc(name_size);
	memcpy(nuevo->nombre, data, name_size);
	memcpy(&(nuevo->tiempo), data + name_size, sizeof(uint32_t));
	return nuevo;
}

void blockProcessIO(t_nodo_hiloIO* dispositivoIO,
		t_nodo_proceso_bloqueadoIO* nuevo)
{
	pthread_mutex_lock(&dispositivoIO->dataHilo.mutex_io);
	queue_push(dispositivoIO->dataHilo.bloqueados, nuevo);
	pthread_mutex_unlock(&dispositivoIO->dataHilo.mutex_io);
	sem_post(&dispositivoIO->dataHilo.sem_io);
}

t_nodo_proceso_bloqueadoIO* waitForProcess(t_dataDispositivo* datos)
{
	sem_wait(&datos->sem_io);
	actualizarEstado();
	pthread_mutex_lock(&datos->mutex_io);
	t_nodo_proceso_bloqueadoIO* proceso = queue_pop(datos->bloqueados);
	pthread_mutex_unlock(&datos->mutex_io);
	return proceso;
}

void doInOut(t_nodo_proceso_ejecutando* procesoEjecutando, t_io* datos)
{
	t_nodo_proceso_bloqueadoIO* nuevo = malloc(sizeof *nuevo);
	t_nodo_hiloIO* dispositivoIO = dictionary_get(dictionaryIO, datos->nombre);
	nuevo->espera = datos->tiempo;
	nuevo->proceso = copyProcessNode(&(procesoEjecutando->proceso));
	blockProcessIO(dispositivoIO, nuevo);
	actualizarEstado();

}

void* inOutThread(void* dataHilo)
{
	int executionSimulator;

	t_dataDispositivo *datos = dataHilo;
	t_nodo_proceso_bloqueadoIO* proceso;

	while (1)
	{

		proceso = waitForProcess(datos);
		executionSimulator = (datos->retardo * proceso->espera) * 1000;
		usleep(executionSimulator);
		addToReadyQueue(proceso->proceso);
		free(proceso);
	}
	return NULL ;
}
