#ifndef CPU_H_

#define CPU_H_
#include <unistd.h>
#include <parser/parser.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "primitivas.h"
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <signal.h>
#include <commons/log.h>
#include "globals.h"
#include "decoracionDePaquetesCPU.h"
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include "estructuras.h"
#include "common_execution.h"
#include <commons/config.h>
#include "terminateActions.h"
#include <unistd.h>

void ejecutarInstruccion();
void avisar_proceso_activo();
void terminarCPU();
void concluyo_instante_quantum();
void traer_segmento_etiquetas();
void desconectarme();
void* hot_plug(void*);
void imprimirDiccionario(char*,uint32_t*);


#endif /* CPU_H_ */
