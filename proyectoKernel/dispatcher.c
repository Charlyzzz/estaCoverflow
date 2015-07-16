#include "globals.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

t_nodo_proceso*
getProcessToDispatch()
{

  sem_wait(&sem_listaListos);
  pthread_mutex_lock(&mutex_listaListos);
  t_nodo_proceso* processToDispatch = queue_pop(listaListos);
  pthread_mutex_unlock(&mutex_listaListos);
  actualizarEstado();
  return processToDispatch;
}

t_cpu*
getCPUToProcess()
{
  sem_wait(&sem_listaCpu);
  pthread_mutex_lock(&mutex_listaCPU);
  t_cpu *cpu = queue_pop(listaCPULibres);
  pthread_mutex_unlock(&mutex_listaCPU);
  actualizarEstado();
  return cpu;

}

void
addProcessToExecutingList(t_nodo_proceso_ejecutando* proceso)
{
  pthread_mutex_lock(&mutex_listaEjecutando);
  list_add(listaEjecutando, proceso);
  pthread_mutex_unlock(&mutex_listaEjecutando);
  actualizarEstado();
}

void
dispatch(t_cpu* cpu, t_nodo_proceso* proceso)
{
  t_datosEnviar* mensajeCPU = pedirPaquete(&(proceso->pcb),
      PCP_CPU_PROGRAM_TO_EXECUTE, sizeof(t_pcb));
  common_send(cpu->socket, mensajeCPU, NULL );
  log_info(log_kernel,"Dispatching process %d to CPU %d.",
      proceso->pcb.pid, cpu->pid);
  destruirPaquete(mensajeCPU);
}

void
addToExecutingList(t_cpu* cpu, t_nodo_proceso* proceso)
{
  t_nodo_proceso_ejecutando* nuevo = malloc(sizeof *nuevo);
  nuevo->cpu = *cpu;
  nuevo->proceso = *proceso;
  addProcessToExecutingList(nuevo);
}

void *
threadDispatcher(void *sinUso)
{
  log_info(log_kernel,"[Dispatcher] Dispatcher is ON Bitches");
  while (1)
    {
      actualizarEstado();
      t_nodo_proceso* processToDispatch = getProcessToDispatch();
      t_cpu *cpu = getCPUToProcess();
      dispatch(cpu, processToDispatch);
      addToExecutingList(cpu, processToDispatch);
      free(cpu);
      free(processToDispatch);
    }
  return NULL ;
}
