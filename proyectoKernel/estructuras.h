/*
 * estructuras.h
 *
 *  Created on: Jul 18, 2014
 *      Author: root
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <conitos-estaCoverflow/common_sockets.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <stdint.h>
#include <pthread.h>
#include <poll.h>

typedef uint32_t t_puntero;
typedef char t_nombre_variable;
typedef uint32_t t_size;
typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef t_nombre_variable* t_nombre_compartida;
typedef t_nombre_variable* t_nombre_dispositivo;
typedef uint32_t t_pid;
typedef uint32_t t_pc;

typedef struct stack
{
	t_size size;
	t_puntero base;
	t_size contextSize;
	t_puntero stack_pointer;
} stack;

typedef struct pcb
{
	t_pid pid;
	t_puntero codeSegment;
	stack currentStack;
	t_puntero codeIndex;
	t_puntero indiceEtiquetas;
	t_pc programCounter;
	t_size tamanioIndiceEtiquetas;
} t_pcb;

typedef struct
{
	t_pcb pcb;
	int peso;
	int soquet_prog;
} t_nodo_proceso;

typedef struct
{
	int socket;
	t_pid pid;
} t_cpu;

typedef struct
{
	t_nodo_proceso proceso;
	t_cpu cpu;
} t_nodo_proceso_ejecutando;

typedef struct
{
	t_nodo_proceso* proceso;
	uint32_t espera;
} t_nodo_proceso_bloqueadoIO;

typedef void (*t_cpu_action)(t_nodo_proceso_ejecutando*, fd_set*,
		t_datosEnviar*);

typedef struct
{
	char *puertoCPU;
	char *puertoProg;
	char *puertoUMV;
	uint32_t quantum;
	uint32_t retardo;
	uint32_t grado_multiprog;
	uint32_t tamanio_pila;
	char *ipUMV;
	char *ipKernel;
	char **idHIO;
	char **valorHIO;
	char **semaforos;
	char **valor_semaforos;
	char **var_compartidas;
} t_configuracion;

typedef struct
{
	char *nombre;
	u_int32_t valor;
} t_varCompartida;

#endif /* ESTRUCTURAS_H_ */
