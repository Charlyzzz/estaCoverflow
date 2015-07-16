#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <stdint.h>
#include <stdbool.h>
#include "globals.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <poll.h>
#include <sys/time.h>
#include <errno.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include "accionesParaCPU.h"
#include <netinet/in.h>
#include "dispatcher.h"
#include "entradaSalida.h"

void
sendHandshake(int cpuSocket)
{
  uint32_t* i = malloc(sizeof(*i));
  *i = configuration.quantum;
  t_datosEnviar* handshakeOK = pedirPaquete(i, PCP_CPU_OK, sizeof(uint32_t));
  *i = configuration.retardo;
  aniadirAlPaquete(handshakeOK, i, sizeof(uint32_t));
  common_send(cpuSocket, handshakeOK, NULL );
  destruirPaquete(handshakeOK);
  free(i);
}

//NOW DO THE HARLEMSHAKE
void
doHandshakeCPU(fd_set* socketsPCP, int cpuSocket)
{
  t_datosEnviar* respuesta = common_receive(cpuSocket, NULL );
  if (respuesta == NULL )
    {

      destruirPaquete(respuesta);
      return;
    }
  if (respuesta->codigo_Operacion == HANDSHAKE)
    {
      t_cpu* nueva_cpu = malloc(sizeof *nueva_cpu);
      nueva_cpu->pid = *((t_pid*) respuesta->data);
      nueva_cpu->socket = cpuSocket;
      if (cpuSocket > pcp_fdMax)
        pcp_fdMax = cpuSocket;
      FD_SET(cpuSocket, socketsPCP);
      log_info(log_kernel,
          "Se conecto la CPU %d en el socket %d",
          nueva_cpu->pid,cpuSocket);
      addCPUToCPUFreeList(nueva_cpu);
      sendHandshake(cpuSocket);
      free(nueva_cpu);
    }
  else
    {
      log_error(log_kernel,
          "No se realizo correctamente el handshake de la CPU");
      destruirPaquete(respuesta);
      return;
    }
  destruirPaquete(respuesta);
}

void
newConnectionHandlerCPU(fd_set* socketsPCP, int listeningSocket)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  int nuevaCPU = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);

  doHandshakeCPU(socketsPCP, nuevaCPU);
}

void
handlerCPU(fd_set* socketsPCP, int socketCPU)
{
  t_datosEnviar* paquete = common_receive(socketCPU, NULL );
  char *key = string_itoa(paquete->codigo_Operacion);
  t_cpu_action accion = dictionary_get(cpu_command_dictionary, key);
  free(key);

  bool
  buscarPorSocketCPU(void* proceso)
  {
    return (((t_nodo_proceso_ejecutando*) proceso)->cpu.socket) == socketCPU;
  }

  if (list_size(listaEjecutando) > 0)
    {
      pthread_mutex_lock(&mutex_listaEjecutando);
      t_nodo_proceso_ejecutando* procesoEjecutando = list_find(listaEjecutando,
          buscarPorSocketCPU);
      pthread_mutex_unlock(&mutex_listaEjecutando);
      accion(procesoEjecutando, socketsPCP, paquete);
    }
  else
    {
      if (paquete->codigo_Operacion == CPU_PCP_DISCONNECTION)
        {
          removeCPUFromKernel(socketCPU, socketsPCP);
          actualizarEstado();
        }
    }
  destruirPaquete(paquete);
}

t_nodo_hiloIO*
createIONode(int index)
{
  t_nodo_hiloIO *hilo = malloc(sizeof(t_nodo_hiloIO));
  hilo->dataHilo.retardo = atoi(configuration.valorHIO[index]);
  sem_init(&hilo->dataHilo.sem_io, 0, 0);
  pthread_mutex_init(&hilo->dataHilo.mutex_io, NULL );
  hilo->dataHilo.bloqueados = queue_create();
  hilo->dataHilo.nombre = strdup(configuration.idHIO[index]);
  return hilo;
}

void
launchIOThreads()
{
  int i = 0;
  while (configuration.idHIO[i] != '\0')
    {
      t_nodo_hiloIO *hilo = createIONode(i);
      pthread_create(&hilo->hiloID, NULL, &inOutThread,
          (void*) &hilo->dataHilo);
      dictionary_put(dictionaryIO, configuration.idHIO[i], hilo);
      log_info(log_kernel,
          "Lanzando en hiloIO %d que pertenece al dispositivo %s", i,
          hilo->dataHilo.nombre);
      i++;
    }
}

void*
threadPCP(void* cosa)
{
  log_info(log_kernel, "Se inicio el PCP");
  fd_set arraySocketsPCP;
  fd_set read;
  int indice, listeningSocketCPU;
  pthread_t hilo_dispatcher;
  pthread_create(&hilo_dispatcher, NULL, *threadDispatcher, NULL );
  launchIOThreads();

  listeningSocketCPU = setup_listen(configuration.ipKernel,
      configuration.puertoCPU);

  FD_ZERO(&arraySocketsPCP);
  FD_ZERO(&read);
  FD_SET(listeningSocketCPU, &arraySocketsPCP);
  pcp_fdMax = listeningSocketCPU;
  log_info(log_kernel, "El PCP escucha en el socket %d", listeningSocketCPU);
  listen(listeningSocketCPU, 1000);
  while (1)
    {
      read = arraySocketsPCP;
      if (select(pcp_fdMax + 1, &read, NULL, NULL, NULL ) < 0)
        {
          log_error(log_kernel, "Select error %d, exiting kernel", errno);
          close(listeningSocketCPU);
          shutdownKernel(EXIT_FAILURE);
          log_error(log_kernel, "Apagando Kernel");
        }
      for (indice = 0; indice <= pcp_fdMax; indice++)
        {
          if (FD_ISSET(indice,&read))
            {
              if (indice == listeningSocketCPU)
                {
                  newConnectionHandlerCPU(&arraySocketsPCP, listeningSocketCPU);
                }
              else
                {

                  handlerCPU(&arraySocketsPCP, indice);
                }
            }
        }
      FD_ZERO(&read);
    }
  pthread_join(hilo_dispatcher, NULL );
  return 0;
}

