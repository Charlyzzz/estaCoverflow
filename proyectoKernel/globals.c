#include "globals.h"
#include "semaforos.h"
#include "entradaSalida.h"
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>


char*
copy_on_string(char* string, char* otherString, va_list arguments)
{
  char* aux = string_from_vformat(string, arguments);
  char* copy = string_duplicate(otherString);
  string_append(&aux, "\n");
  string_append(&copy, aux);
  free(aux);
  return copy;
}

t_pcb*
unserializePCB(void* data)
{
  t_pcb* pcb = malloc(sizeof *pcb);
  memcpy(pcb, data, sizeof *pcb);
  return pcb;
}

void
removeFromExecutingList(t_nodo_proceso_ejecutando* process)
{
  bool
  checkByProcessID(void* nodo)
  {
    return ((t_nodo_proceso_ejecutando*) nodo)->proceso.pcb.pid
        == process->proceso.pcb.pid;
  }
  pthread_mutex_lock(&mutex_listaEjecutando);
  list_remove_by_condition(listaEjecutando, checkByProcessID);
  pthread_mutex_unlock(&mutex_listaEjecutando);
  actualizarEstado();
}

void
addCPUToCPUFreeList(t_cpu* cpu)
{
  t_cpu* copia = malloc(sizeof *copia);
  memcpy(copia, cpu, sizeof *copia);
  pthread_mutex_lock(&mutex_listaCPU);
  queue_push(listaCPULibres, copia);
  pthread_mutex_unlock(&mutex_listaCPU);
  sem_post(&sem_listaCpu);
  actualizarEstado();
}

t_nodo_proceso*
copyProcessNode(t_nodo_proceso* proceso)
{
  t_nodo_proceso* copia = malloc(sizeof *copia);
  memcpy(copia, proceso, sizeof *copia);
  return copia;
}

void
addToReadyQueue(t_nodo_proceso* proceso)
{
  t_nodo_proceso* copia = copyProcessNode(proceso);
  pthread_mutex_lock(&mutex_listaListos);
  log_info(log_kernel,"Moviendo proceso %d a la lista de Listos",proceso->pcb.pid);
  queue_push(listaListos, copia);
  pthread_mutex_unlock(&mutex_listaListos);
  sem_post(&sem_listaListos);
  actualizarEstado();
}

void
removeProcess(t_nodo_proceso_ejecutando* procesoEj)
{
  t_nodo_proceso* copia = copyProcessNode(&(procesoEj->proceso));
  pthread_mutex_lock(&mutex_listaTerminados);
  queue_push(listaTerminados, copia);
  log_info(log_kernel,"Moviendo proceso %d a la lista de Terminados",procesoEj->proceso.pcb.pid);
  pthread_mutex_unlock(&mutex_listaTerminados);
  sem_post(&sem_listaTerminados);
  sem_post(&sem_multiprog);
  actualizarEstado();
}

void
pcp_exitProcess(t_nodo_proceso_ejecutando* proceso)
{
  removeProcess(proceso);
  actualizarEstado();
  addCPUToCPUFreeList(&(proceso->cpu));
  removeFromExecutingList(proceso);
}

void
stopProcessing(t_nodo_proceso_ejecutando* procesoEjecutando)
{
  removeFromExecutingList(procesoEjecutando);
  addCPUToCPUFreeList(&(procesoEjecutando->cpu));
}

void
removeCPUFromKernel(int socketCPU, fd_set* socketsPCP)
{
  bool
  buscarPorSocketCPU(void* cpu)
  {
    return (((t_cpu*) cpu)->socket) == socketCPU;
  }
  log_info(log_kernel,"Se desconecto una CPU");
  pthread_mutex_lock(&mutex_listaCPU);
  close(socketCPU);
  FD_CLR(socketCPU, socketsPCP);
  t_cpu* cpu = list_remove_by_condition(listaCPULibres->elements,
      buscarPorSocketCPU);
  if (cpu != NULL )
    {
      sem_wait(&sem_listaCpu);
      free(cpu);
    }
  pthread_mutex_unlock(&mutex_listaCPU);
  actualizarEstado();
}

void
listarListos()
{
  pthread_mutex_lock(&mutex_listaListos);
  if (queue_is_empty(listaListos))
    {
      printf("\n\n\e[38;5;166m|>\e[0m COLA LISTOS -- VACIA");
    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m COLA LISTOS     Tamaño: %d",
          queue_size(listaListos));
      int i;
      t_nodo_proceso *programa;
      for (i = 0; i < queue_size(listaListos); i++)
        {
          programa = list_get(listaListos->elements, i);
          printf("\n\e[38;5;166m|\e[0m ID_PROCESO: %i", programa->pcb.pid);
        }
    }
  pthread_mutex_unlock(&mutex_listaListos);

}

void
listarTerminados()
{
  pthread_mutex_lock(&mutex_listaTerminados);
  if (queue_is_empty(listaTerminados))
    {
      printf("\n\n\e[38;5;166m|>\e[0m COLA TERMINADOS -- VACIA");
    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m COLA TERMINADOS -- Tamaño: %d",
          queue_size(listaTerminados));
      int i;

      for (i = 0; i < queue_size(listaTerminados); i++)
        {
          t_nodo_proceso *programa = list_get(listaTerminados->elements, i);
          printf(
              "\n\e[38;5;166m|\e[0m NODO: %i, ID_PROCESO: %i, SEGMENTO_CODIGO: %i, SEGMENTO_PILA: %i",
              i, programa->pcb.pid, programa->pcb.codeSegment,
              programa->pcb.currentStack.base);
        }
    }
  pthread_mutex_unlock(&mutex_listaTerminados);

}

void
listarNuevos()
{
  pthread_mutex_lock(&mutex_listaNuevos);
  if (list_is_empty(listaNuevos))
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA NUEVOS -- VACIA");
    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA NUEVOS -- Tamaño: %d",
          list_size(listaNuevos));
      int i;
      for (i = 0; i < list_size(listaNuevos); i++)
        {
          t_nodo_proceso *programa = list_get(listaNuevos, i);
          printf(
              "\n\e[38;5;166m|\e[0m NODO: %i, ID_PROCESO: %i, PESO: %i",
              i, programa->pcb.pid, programa->peso);
        }
    }
  pthread_mutex_unlock(&mutex_listaNuevos);
}

void
listarEjecutando()
{
  pthread_mutex_lock(&mutex_listaEjecutando);
  if (list_size(listaEjecutando) == 0)
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA EJECUTANDO -- VACIA");
    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA EJECUTANDO -- Tamaño: %d",
          list_size(listaEjecutando));
      int i;
      t_nodo_proceso_ejecutando *programa;
      for (i = 0; i < list_size(listaEjecutando); i++)
        {
          programa = list_get(listaEjecutando, i);
          printf(
              "\n\e[38;5;166m|\e[0m ID_PROCESO: %i | ID_CPU: %i | SOCKET_CPU: %i",
              programa->proceso.pcb.pid, programa->cpu.pid,
              programa->cpu.socket);
        }
    }
  pthread_mutex_unlock(&mutex_listaEjecutando);
}

void
listarSemaforos()
{
  void
  imprimir(char*clave, t_semaforo* semaforo)
  {
    printf(
        "\n      \e[38;5;166m|\e[0m CLAVE: %s, NOMBRE SEMAFORO: %s, VALOR SEMAFORO: %i",
        clave, semaforo->nombre, semaforo->valor);
    int i;
    t_nodo_proceso* proceso;
    if (queue_size(semaforo->bloqueados) > 0)
      {
        for (i = 0; i < queue_size(semaforo->bloqueados); i++)
          {
            proceso = list_get(semaforo->bloqueados->elements, i);
            printf("\n            \e[38;5;166m|\e[0m ID_PROCESO: %d",
                proceso->pcb.pid);
          }
      }
  }
  if (dictionary_is_empty(dictionarySemaphores))
    {
      printf("\n\n\e[38;5;166m|>\e[0m NO HAY SEMAFOROS");

    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m ESTADO DE LOS SEMAFOROS");
      dictionary_iterator(dictionarySemaphores, (void*) imprimir);

    }

}

void
listarCPUs()
{
  pthread_mutex_lock(&mutex_listaCPU);
  if (queue_size(listaCPULibres) == 0)
    {
      printf("\e[38;5;166m|>\e[0m LISTA CPU's LIBRES -- VACIA");
    }
  else
    {
      printf("\n\e[38;5;166m|>\e[0m LISTA CPU's LIBRES -- Tamaño: %d",
          queue_size(listaCPULibres));
      int i;
      t_cpu* cpu;
      for (i = 0; i < queue_size(listaCPULibres); i++)
        {
          cpu = list_get(listaCPULibres->elements, i);
          printf("\n\e[38;5;166m|\e[0m CPU ID: %d | SOCKET: %i ", cpu->pid,
              cpu->socket);
        }
    }

  pthread_mutex_unlock(&mutex_listaCPU);

}

void
listarIO()
{
  void
  imprimirHilosIO(char*clave, t_nodo_hiloIO* hiloIO)
  {
    printf("\n  \e[38;5;166m|\e[0m NOMBRE DISPOSITIVO: %s",
        hiloIO->dataHilo.nombre);
    int i;
    t_nodo_proceso_bloqueadoIO *programa;
    pthread_mutex_lock(&hiloIO->dataHilo.mutex_io);
    for (i = 0; i < queue_size(hiloIO->dataHilo.bloqueados); i++)
      {
        programa = list_get(hiloIO->dataHilo.bloqueados->elements, i);
        printf("\n       \e[38;5;166m|\e[0m PID: %d", programa->proceso->pcb.pid);
      }
    pthread_mutex_unlock(&hiloIO->dataHilo.mutex_io);
  }
  if (dictionary_is_empty(dictionaryIO))
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA DE HILOS IO -- VACIA");
    }
  else
    {
      printf("\n\n\e[38;5;166m|>\e[0m LISTA DE HILOS IO");
      dictionary_iterator(dictionaryIO, (void*) imprimirHilosIO);
    }
}
void listarVariablesCompartidas(){
  void imprimirVariableCompartida(char* clave,t_varCompartida* var){

    printf("\n  \e[38;5;166m|\e[0m VARIABLE: %s, VALOR:%d",
            clave,var->valor);
  }
  if(dictionary_is_empty(dictionarySharedVariables)){

      printf("\n\n\e[38;5;166m|>\e[0m NO HAY VARIABLES COMPARTIDAS");

  }else{
      printf("\n\n\e[38;5;166m|>\e[0m LISTA DE VARIABLES COMPARTIDAS");
      dictionary_iterator(dictionarySharedVariables,(void*)imprimirVariableCompartida);
  }

}

void
destruirListas()
{

}

void
cerrarSemaforos()
{
  pthread_mutex_destroy(&mutex_imprimirEstado);
  pthread_mutex_destroy(&mutex_listaCPU);
  pthread_mutex_destroy(&mutex_listaTerminados);
  pthread_mutex_destroy(&mutex_semaforos);
  pthread_mutex_destroy(&mutex_listaListos);
  sem_destroy(&sem_listaNuevos);
  sem_destroy(&sem_listaListos);
  sem_destroy(&sem_semaforos);
}

void
shutdownKernel()
{
  destruirListas();
  cerrarSemaforos();
  exit(EXIT_FAILURE);
}

void
actualizarEstado()
{
  pthread_mutex_lock(&mutex_imprimirEstado);
  system("clear");
  listarCPUs();
  listarEjecutando();
  listarTerminados();
  listarListos();
  listarNuevos();
  listarIO();
  listarSemaforos();
  listarVariablesCompartidas();

  pthread_mutex_unlock(&mutex_imprimirEstado);
}
