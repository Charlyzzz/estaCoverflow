#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <conitos-estaCoverflow/common_sockets.h>	//Manejo de sockets
#include <conitos-estaCoverflow/conitos_protocol.h>	//Manejo de sockets
#include <commons/log.h>							//COMMONS: Log
#include <commons/collections/list.h>				//COMMONS: Manejo de listas
#include <commons/config.h>							//COMMONS: Archivo de configuracion
#include <pthread.h>								//HILOS
#include <semaphore.h>								//SEMAFOROS
#ifndef UMV_H_
#define UMV_H_

typedef struct {		// Estructura para guardar los datos de configuracion
	char* puerto;
	int cantidadBytesMalloc;
	int retardo;
	char *inicioMalloc;
	char *ip;
} configuracionDeLaUMV;

typedef struct {					// Estructura para guardar el proceso activo
	int pidProcesoActivo;
} procesoActivo;

typedef struct {						// Estructura de segmentos
	int idSegmento;
	int processID; //Process ID
	int direccionVirtual; // en mi UMV
	int tamanoSegmento; // el tamaño del segmento X
	char* direccionReal; // direccion fisica de memoria
} estructuraDeSegmento;

//VARIABLES GLOBALES
t_list *listaDeSegmentos;				//Segmentos
estructuraDeSegmento* nodoDeLista;		//Lista de Segmentos
configuracionDeLaUMV configuracion;		//Registro con Configuracion
procesoActivo procesoEjecutando;		//----------------
int algoritmoActivo = 0;				//Algoritmo: 1-firstFit & 2-worstFit
int idSegmento = 0;		//ID Segmento: va creciendo para diferenciar segmentos
pthread_mutex_t semaforo;				//Semaforo para esperar
//PD: el semaforo seguro lo saco pero esta como ejemplo.
int retardoTest = 0;
fd_set descriptoresLectura;
int socketKernel, socketHilo;
int socketActivo, socketEscucha;
int optval = 1;
struct sockaddr_in dirServidor;
t_log *archivo_log;

//FUNCIONES
void consola();
void cabeceraConsola();
char * virtualAReal(int base);
void imprimeSegmentosARCHIVO(char nombreDump[50], int pid);
void leerHeavyBytes(char * inicio, int offset, int cantidad, int reconoceBarra0);
void leerHeavyBytesARCHIVO(char * inicio, int offset, int cantidad,
		int reconoceBarra0, char archivoDump[50]);
void * leerBytes(int base, int offset, int cantidad, int PID);
int pegarBytes(void * bytesToCpy, int base, int offset, int cantidad, int PID);
void leerArchivoConfiguracionUMV(char *path);
void imprimeSegmentos();
void imprimeSegmentosPID(int pid);
procesoActivo cambiarProceso(procesoActivo estado, int pidNuevo);
void cambiarAlgoritmo(int nuevoAlgoritmo);
void destruirSegmentosDeUnPrograma(int pid);
void almacenarBytes(void* base, int offset, int tamano, void* buffer);
void retardo(int milisegundos);
void actualizarSegmento(void* memoriaDestino, estructuraDeSegmento *nodoOrigen);
void eliminaVacios();
void compactador();
int tamanoUsadoDelMalloc();
void segmento_create(estructuraDeSegmento datos, t_list* lista);
bool auxiliarParaOrdenarListaDeSegmentos(estructuraDeSegmento* seg1,
		estructuraDeSegmento* seg2);
bool dirVirEstaEnUMV(int numero);
estructuraDeSegmento firstFit(estructuraDeSegmento datosSegmento);
estructuraDeSegmento worstFit(estructuraDeSegmento datosSegmento);
int crearSegmento(int pid, int tamano);
char* pedirMemoria(int cantidad);
void testCompactador2();

//TEST
void testCargarConfiguracion(char *path);
void testCreaSegmento(int numeroSegmento, int tamanoSegmento);
void testTamanoUsado();
void testEliminaSegmento(int numero);
void testDestruirSegmentosDeUnPrograma(int numero);
void testCompactador();
void test();

//HILOS
void escucharConexion();
void atenderConexion(int listener);
void esperaConexionCPU();
void *hiloKernel(void * socketParametro);
void *cadaCPU(void *socketParametro);

#endif /* UMV_H_ */
