#include "globals.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "entradaSalida.h"
#include "semaforos.h"
#include "accionesParaCPU.h"
#include <commons/config.h>
#include "pcp.h"
#include "plp.h"
#include "kernelPosta.h"
#include "estructuras.h"

t_configuracion configuration;
int socketUMV;

t_list *listaNuevos;
t_queue *listaListos;
t_queue *listaTerminados;
t_list *listaEjecutando;
t_queue *listaCPULibres;
//diccionarios
t_dictionary *dictionaryIO;
t_dictionary *dictionarySemaphores;
t_dictionary *dictionarySharedVariables;
//semaforos
sem_t sem_listaCpu;
sem_t sem_listaNuevos;
sem_t sem_listaListos;
sem_t sem_semaforos;
sem_t sem_multiprog;
sem_t sem_listaTerminados;
pthread_mutex_t mutex_imprimirEstado;
pthread_mutex_t mutex_listaEjecutando;
pthread_mutex_t mutex_listaCPU;
pthread_mutex_t mutex_listaListos;
pthread_mutex_t mutex_semaforos; //FIXME no deberia ser un array? Fijarse en kernel.c
pthread_mutex_t mutex_listaTerminados;
pthread_mutex_t mutex_listaNuevos;
int pcp_fdMax;
t_dictionary* cpu_command_dictionary;
t_log* log_kernel;
bool debugOn = false;

void
startCommunicationWithUMV()
{
  t_datosEnviar *mensaje = pedirPaquete("OK", UMV_ACEPTA_KERNEL, 3);
  socketUMV = connect_to(configuration.ipUMV, configuration.puertoUMV);
  common_send(socketUMV, mensaje, NULL );
  mensaje = common_receive(socketUMV, NULL );
  if (mensaje->codigo_Operacion != UMV_ACEPTA_KERNEL)
    {
      shutdownKernel();
      log_error(log_kernel,
          "No se pudo conectar con la UMV, cerrando el Kernel");
    }
  log_info(log_kernel, "Se conecto exitosamente con la UMV");
}

int
main(int argc, char** argv)
{

  setvbuf(stdout, NULL, _IONBF, 0);

  readConfig(argv[1]);

  log_kernel = log_create(argv[2], "KERNEL", false, LOG_LEVEL_TRACE);
  log_info(log_kernel, "Se inicio el Kernel");

  initializateCollections();

  setupSemaphores();

  fillDictionaries();

  startCommunicationWithUMV();

  pthread_t hilo_PLP;
  pthread_t hilo_PCP;

  pthread_create(&hilo_PLP, NULL, *threadPLP, NULL );
  pthread_create(&hilo_PCP, NULL, *threadPCP, NULL );

  actualizarEstado();
  t_nodo_proceso* nodoAListo;
  while (1)
    {

      sem_wait(&sem_multiprog);
      sem_wait(&sem_listaNuevos);
      pthread_mutex_lock(&mutex_listaNuevos);
      nodoAListo = list_remove(listaNuevos, 0);
      pthread_mutex_unlock(&mutex_listaNuevos);
      log_info(log_kernel, "Moviendo PID %d a la lista de Listos",
          nodoAListo->pcb.pid);
      pthread_mutex_lock(&mutex_listaListos);
      queue_push(listaListos, nodoAListo);
      pthread_mutex_unlock(&mutex_listaListos);
      sem_post(&sem_listaListos);
      actualizarEstado();
    }

  pthread_join(hilo_PLP, NULL );
  pthread_join(hilo_PCP, NULL );

  return 0;
}

void
fillDictionaries()
{

  fillSemaphoreDictionary();

  fillSharedVariableDictionary();

  fillCPUCommandDictionary();

}

#define command(codOp,function) dictionary_put(cpu_command_dictionary,string_itoa(codOp),function);

/*void fillCPUCommandDictionary()
 {
 void cpuFinishesQuantum(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void signalRequest(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void waitRequest(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void cpuFinishesProcess(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void cpuFinishesProcess(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void sharedVariableRequest(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void sharedVariableAssign(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void printText(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void printVariable(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void exceptionCPU(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void cpuDisconnects(t_nodo_proceso_ejecutando*, struct pollfd*, int*, t_datosEnviar*);
 void inOut(t_nodo_proceso_ejecutando* ,struct pollfd* , int* , t_datosEnviar* );*/
/*
 command(,cpuFinishesQuantum)
 command(,signalRequest)
 command(,waitRequest)
 command(,cpuFinishesProcess)
 command(,sharedVariableRequest)
 command(,sharedVariableAssign)
 command(,printText)
 command(,printVariable)
 command(,exceptionCPU)
 command(,cpuDisconnects)
 command(,inOut)}*/

#define CPU_PCP_SIGINT 92

void
fillCPUCommandDictionary()
{
  command(CPU_PCP_FIN_QUANTUM, cpuFinishesQuantum)
  command(CPU_PCP_SIGNAL, signalRequest)
  command(CPU_PCP_WAIT, waitRequest)
  command(CPU_PCP_FIN_PROCESO, cpuFinishesProcess)
  command(CPU_PCP_PEDIR_VARIABLE_COMPARTIDA, sharedVariableRequest)
  command(CPU_PCP_ASIGNAR_VARIABLE_COMPARTIDA, sharedVariableAssign)
  command(CPU_PCP_IMPRIMIR_TEXTO, printText)
  command(CPU_PCP_IMPRIMIR_VARIABLE, printVariable)
  command(CPU_PCP_EXCEPTION, exceptionCPU)
  command(CPU_PCP_DISCONNECTION, cpuDisconnects)
  command(CPU_PCP_ENTRADA_SALIDA, inOut)
  command(CPU_PCP_SIGINT, cpuSIGINT)

}

#define registerWithNameAndValue(functionName,type,typeText,dictionary,creator)\
				void functionName(char* name,int value) { \
						type *var = creator(name,value);\
						dictionary_put(dictionary, var->nombre, var);\
				}\

registerWithNameAndValue(registerSemaphore, t_semaforo, "semaphore",
    dictionarySemaphores, semaforo_create)
registerWithNameAndValue(registerSharedVariable, t_varCompartida,
    "shared variable", dictionarySharedVariables, createSharedVariable)

void
fillSemaphoreDictionary()
{

  int i = 0;

  while (configuration.semaforos[i] != '\0')
    {
      registerSemaphore(configuration.semaforos[i],
          atoi(configuration.valor_semaforos[i]));
      i++;
    }

}

void
fillSharedVariableDictionary()
{
  int i = 0;

  while (configuration.var_compartidas[i] != '\0')
    {
      registerSharedVariable(configuration.var_compartidas[i], 0);
      i++;
    }

}

t_varCompartida*
createSharedVariable(char* name, uint32_t initial_value)
{
  t_varCompartida* var = malloc(sizeof(t_varCompartida));
  var->nombre = strdup(name);
  var->valor = initial_value;
  return var;
}

void
setupSemaphores()
{
  sem_init(&sem_listaTerminados, 0, 0);
  pthread_mutex_init(&mutex_listaTerminados, NULL );
  sem_init(&sem_listaCpu, 0, 0);
  pthread_mutex_init(&mutex_listaCPU, NULL );
  pthread_mutex_init(&mutex_listaEjecutando, NULL );
  sem_init(&sem_listaNuevos, 0, 0);
  sem_init(&sem_listaListos, 0, 0);
  sem_init(&sem_multiprog, 0, configuration.grado_multiprog);
  sem_init(&sem_semaforos, 0, 0);
  pthread_mutex_init(&mutex_semaforos, NULL );
  pthread_mutex_init(&mutex_listaListos, NULL );
  pthread_mutex_init(&mutex_imprimirEstado, NULL );
}

void
initializateCollections()
{
  listaNuevos = list_create();
  listaListos = queue_create();
  listaTerminados = queue_create();
  listaCPULibres = queue_create();
  listaEjecutando = list_create();
  dictionaryIO = dictionary_create();
  dictionarySemaphores = dictionary_create();
  dictionarySharedVariables = dictionary_create();
  cpu_command_dictionary = dictionary_create();
}

void
readConfig(char *path)
{

  t_config *configKernel;
  configKernel = config_create(path);
  configuration.ipUMV = config_get_string_value(configKernel, "IPUMV");
  configuration.ipKernel = config_get_string_value(configKernel, "IPKERNEL");
  configuration.puertoUMV = config_get_string_value(configKernel, "PUERTO_UMV");
  /*configuration.puertoKERNEL = config_get_string_value(configKernel,
   "PUERTO_KERNEL");*/
  configuration.puertoProg = config_get_string_value(configKernel,
      "PUERTO_PROG");
  configuration.puertoCPU = config_get_string_value(configKernel, "PUERTO_CPU");
  configuration.quantum = config_get_int_value(configKernel, "QUANTUM");
  configuration.grado_multiprog = config_get_int_value(configKernel,
      "MULTIPROGRAMACION");
  configuration.retardo = config_get_int_value(configKernel, "RETARDO");
  configuration.tamanio_pila = config_get_int_value(configKernel,
      "TAMANIO_STACK");
  configuration.semaforos = config_get_array_value(configKernel, "SEMAFOROS");
  configuration.valor_semaforos = config_get_array_value(configKernel,
      "VALOR_SEMAFORO");
  configuration.valorHIO = config_get_array_value(configKernel, "HIO");
  configuration.var_compartidas = config_get_array_value(configKernel,
      "VARIABLES_GLOBALES");
  configuration.idHIO = config_get_array_value(configKernel, "ID_HIO");
  //config_destroy(configKernel); TODO esto tiene un leak mas grande que uan casa

}
