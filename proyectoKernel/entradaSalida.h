/*
 * entradaSalida.h
 *
 *  Created on: Jul 17, 2014
 *      Author: root
 */

#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

typedef struct
{
	char* nombre;
	pthread_mutex_t mutex_io;
	sem_t sem_io;
	int retardo;
	t_queue *bloqueados;
} t_dataDispositivo;

typedef struct
{
	pthread_t hiloID;
	t_dataDispositivo dataHilo;
} t_nodo_hiloIO;

typedef struct
{
	char* nombre;
	uint32_t tiempo;
} t_io;

t_io* unserializeIO(void* data, t_size size);
void doInOut(t_nodo_proceso_ejecutando* procesoEjecutando, t_io* datos);
void* inOutThread(void* dataHilo);

#endif /* ENTRADASALIDA_H_ */
