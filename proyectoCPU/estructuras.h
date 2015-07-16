#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <parser/parser.h>


typedef uint32_t t_pc;
typedef uint32_t t_pid;

typedef struct stack {
	t_size size;
	t_puntero base;
	t_size contextSize;
	t_puntero stack_pointer;
} stack;


struct pcb {
	t_pid pid;
	t_puntero codeSegment;
	stack currentStack;
	t_puntero codeIndex;
	t_puntero indiceEtiquetas;
	t_pc programCounter;
	t_size tamanioIndiceEtiquetas;
}typedef t_pcb;

typedef enum {NORMAL, SEG_FAULT, STK_OVERFLOW, KILL_CPU, END_PROCESS, ENTRADA_SALIDA, WAIT} state;
typedef void (*t_action)(void*,t_size);



#endif /*ESTRUCTURAS_H_*/
