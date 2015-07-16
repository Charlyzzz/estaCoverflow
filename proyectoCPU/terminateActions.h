
#ifndef ONENDACTIONS_H_
#define ONENDACTIONS_H_

#include "common_execution.h"
#include "globals.h"
#include <unistd.h>
#include <commons/config.h>

void terminarCPU(void* dataExtra, t_size size);
void terminaFinishedProcess(void* dataExtra, t_size extraSize);
void terminaBlockedSemaphore(void* dataExtra, t_size extraSize);
void terminaBlockedIO(void* dataExtra, t_size extraSize);
void terminaExcepcion(void* dataExtra,t_size extraSize) ;
void terminaFinQuantum(void* dataExtra, t_size extraSize);
void terminarCPUSinPCB();

#endif /* ONENDACTIONS_H_ */
