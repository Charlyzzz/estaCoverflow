#include "umv.h"
#include <ctype.h>

#define UMV_ACEPTA_CPU						2
#define UMV_CONTESTA_CPU_EXITO				2
#define UMV_CAE_CPU							123
#define UMV_ACEPTA_KERNEL					3
#define UMV_PROCESO_ACTIVO_CPU				25

#define UMV_MENSAJE_CPU_LEER				71
#define UMV_MENSAJE_CPU_ESCRIBIR			5

#define ERROR_NO_CREO_SEGMENTO				6
#define ERROR_FALLO_SEGMENTO				7
#define ERROR_CODIGO_INVALIDO				8

#define KERNEL_PEDIR_SEGMENTO_UMV			11
#define KERNEL_ESCRIBIR_SEGMENTO_UMV		12
#define KERNEL_DESTRUIR_SEGMENTOS_PROGRAMA	14

int main(int argc, char** argv) {

	archivo_log = log_create(argv[2], "UMV", false, LOG_LEVEL_INFO);
	testCargarConfiguracion(argv[1]);

	pthread_mutex_init(&semaforo, NULL );

	//CONEXION
	if ((socketEscucha = setup_listen(configuracion.ip, configuracion.puerto))
			< 0) {
		//ERROR
		printf("[ERROR] - EN FUNCION SOCKET()\n");
		log_error(archivo_log, "EN FUNCION SOCKET()");
	}
	if (listen(socketEscucha, 10) < 0) {
		//ERROR
		printf("[ERROR] - EN FUNCION LISTEN()\n");
	}

	//CONSOLA
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL );

	//ESCUCHA CONEXIONES
	pthread_t hiloConexiones;
	pthread_create(&hiloConexiones, NULL, (void*) escucharConexion, NULL );

	pthread_join(hiloConexiones, NULL );
	pthread_join(hiloConsola, NULL );

	return 0;

}
void consola() {

	//testCargarConfiguracion();
	//system("clear");

	int opcion = 0;
	int var1, var2;
	int segmento, offset, cantidad, pid;
	char * texto;
	char nombreDump[50];

	cabeceraConsola();

	while (opcion != -1) {
		printf("> ");
		scanf("%d", &opcion);

		pthread_mutex_lock(&semaforo);

		switch (opcion) {
		case 1:
			test();
			break;
		case 2:
			printf("\n\t INGRESE:\t\"PID, TAMAÑO\"\n");
			scanf("%d, %d", &var1, &var2);

			log_info(archivo_log,
					"[POR CONSOLA] PIDIO CRAR EL SEGMENTO DE %d DE TAMAÑO %d",
					var1, var2);
			testCreaSegmento(var1, var2);
			break;
		case 3:
			log_info(archivo_log,
					"[POR CONSOLA] PIDIO EL TAMAÑO USADO DE LA MEMORIA");
			testTamanoUsado();
			break;
		case 4:
			printf("\n\t INGRESE:\tNUMERO_SEGMENTO\n");
			scanf("%d", &var1);
			log_info(archivo_log, "[POR CONSOLA] PIDIO ELIMINAR EL SEGMENTO %d",
					var1);
			testEliminaSegmento(var1);
			break;
		case 5:
			printf("\n\t INGRESE:\tPID\n");
			scanf("%d", &var1);
			log_info(archivo_log,
					"[POR CONSOLA] PIDIO ELIMINAR SEGMENTOS DEL PROGRAMA %d",
					var1);
			testDestruirSegmentosDeUnPrograma(var1);
			break;
		case 6:
			log_info(archivo_log, "[POR CONSOLA] PIDIO COMPACTAR LA MEMORIA");
			testCompactador2();
			break;
		case 7:
			printf("\n\t INGRESE:   NUEVO_RETARDO\n");
			scanf("%d", &var1);
			configuracion.retardo = var1;
			log_info(archivo_log, "[POR CONSOLA] CAMBIO EL RETARDO A %d", var1);
			break;
		case 8:
			log_info(archivo_log, "[POR CONSOLA] CAMBIO EL ALGORITMO");
			if (algoritmoActivo == 1) {
				cambiarAlgoritmo(2);
				printf("[UMV] - SE CAMBIO EL ALGORITMO A WORST-FIT.\n");
			} else {
				cambiarAlgoritmo(1);
				printf("[UMV] - SE CAMBIO EL ALGORITMO A FIRST-FIT.\n");
			}
			break;
		case 9:
			printf("\n\t INGRESE:   PID (0 para todos los segmentos)\n");
			scanf("%d", &var1);
			printf("\n\t Guardar en archivo? INGRESE [0/1], nombre\n");
			scanf("%d, %s", &var2, &nombreDump[0]);
			log_info(archivo_log, "[POR CONSOLA] PIDIO IMPRIMIR SEGMENTOS");
			imprimeSegmentosPID(var1);
			if (var2) {
				imprimeSegmentosARCHIVO(nombreDump, var1);
			}
			break;
		case 10:
			system("clear");
			cabeceraConsola();
			break;
		case 11:
			printf("\n\t INGRESE:   SEGMENTO, OFFSET, CANTIDAD, PID \n");
			scanf("%d, %d, %d, %d", &segmento, &offset, &cantidad, &pid);

			log_info(archivo_log, "[POR CONSOLA] PIDIO LEER BYTES");

			texto = malloc(cantidad + 1);
			memcpy(texto, leerBytes(segmento, offset, cantidad, pid),
					cantidad + 1);
			*(&texto + cantidad) = '\0';

			if (texto != ((char*) 0)) {
				printf("EL TEXTO ES : ");
				puts(texto);
				log_info(archivo_log, "EL TEXTO ES: %s", texto);
			}
			free(texto);
			break;
		case 12:
			printf("\n\t INGRESE:   SEGMENTO, OFFSET, CANTIDAD, PID \n");
			scanf("%d, %d, %d, %d", &segmento, &offset, &cantidad, &pid);
			printf("DATOS\n");
			texto = malloc(cantidad + 1);
			scanf("%s", texto);
			log_info(archivo_log, "[POR CONSOLA] PIDIO ESCRIBIR BYTES");
			if (pegarBytes(texto, segmento, offset, cantidad, pid) == 1) {
				printf(" - salio: OK\n");
			} else {
				printf(" - salio: [[ERROR]]\n");
			}
			free(texto);
			break;
		case 13:
			printf("\n\t INGRESE:   OFFSET, CANTIDAD\n");
			scanf("%d, %d", &offset, &cantidad);
			printf("\n\t Guardar en archivo? INGRESE [0/1], nombre\n");
			scanf("%d, %s", &var2, &nombreDump[0]);
			leerHeavyBytes(configuracion.inicioMalloc, offset, cantidad, 0);
			if (var2) {
				leerHeavyBytesARCHIVO(configuracion.inicioMalloc, offset,
						cantidad, 0, nombreDump);
			}
			break;
		default:
			break;
		}

		pthread_mutex_unlock(&semaforo);
		retardo(configuracion.retardo);
	}
	pthread_exit(NULL );
}

void cabeceraConsola() {
	printf(
			"\e[1m ----------------------------------------------------------------\e[0m\n");
	printf(
			"                                                                          \n");
	printf(
			"\e[38;5;166m         /\\                                            /\\               \n");
	printf(
			"        /  \\                                          /  \\              \n");
	printf(
			"       /    \\    \e[0m \e[1m   ¡Bienvenido a la consola   \e[0m  \e[38;5;166m   /    \\             \n");
	printf(
			"    __\e[0m\e[97m/\\____/\\\e[0m\e[38;5;166m__                                  __\e[0m\e[97m/\\____/\\\e[0m\e[38;5;166m__        \n");
	printf(
			"   / /\e[0m\e[97m\\______/\e[0m\e[38;5;166m\\ \\ \e[0m \e[1m de la UMV de Conitos Team! \e[0m \e[38;5;166m / /\e[0m\e[97m\\______/\e[0m\e[38;5;166m\\ \\     \n");
	printf(
			"  /  \\________/  \\                              /  \\________/  \\      \n");
	printf(
			"  \\______________/                              \\______________/      \e[0m\n\n");

	printf(
			"\e[1m ----------------------------------------------------------------\e[0m\n");

	printf("\e[1mSeleccione la opcion que desea ejecutar:\e[0m\n");
	printf("\e[38;5;166m1)\e[0m TEST COMPLETO\n");
	printf("\e[38;5;166m2)\e[0m CREAR SEGMENTO\n");
	printf("\e[38;5;166m3)\e[0m TAMAÑO USADO DEL MALLOC\n");
	printf("\e[38;5;166m4)\e[0m ELIMINAR SEGMENTO\n");
	printf("\e[38;5;166m5)\e[0m ELIMINAR SEGMENTO DE UN PROGRAMA\n");
	printf("\e[38;5;166m6)\e[0m COMPACTAR\n");
	printf("\e[38;5;166m7)\e[0m RETARDO\n");
	printf("\e[38;5;166m8)\e[0m CAMBIAR ALGORITMO\n");
	printf("\e[38;5;166m9)\e[0m IMPRIME ESTADO ACTUAL DE LOS SEGMENTOS\n");
	printf("\e[38;5;166m10)\e[0m LIMPIAR PANTALLA\n");
	printf("\e[38;5;166m11)\e[0m LEER BYTES\n");
	printf("\e[38;5;166m12)\e[0m PEGAR BYTES\n");
	printf("\e[38;5;166m13)\e[0m LEER HEAVY-BYTES\n\n");
}

void leerHeavyBytesARCHIVO(char * inicio, int offset, int cantidad,
		int reconoceBarra0, char nombreDump[50]) {

	int i = 0;
	int barra = reconoceBarra0;
	FILE *nombrearchivo = fopen(nombreDump, "w");
	fprintf(nombrearchivo, "<<INICIO>>LECTURA-HEAVY\n");

	for (i = 0;
			offset >= 0 && i < cantidad && (offset + i) <= configuracion.cantidadBytesMalloc;
			) {
		char * posicion = inicio + offset + i;
		if (*posicion == '\0' && barra == 1)
			i = cantidad;
		if (isalpha(*posicion)) {
			//leo char
			fprintf(nombrearchivo, "%c", *posicion);
			i = i + 1;
		} else if (iscntrl(*posicion)) {
			//leo char especiales
			fprintf(nombrearchivo, "%c", *posicion);
			i = i + 1;
		} else if (isdigit(*posicion)) {
			//leo char digito
			fprintf(nombrearchivo, "%c", *posicion);
			i = i + 1;
		} else if (ispunct(*posicion)) {
			//leo char puntuacion
			fprintf(nombrearchivo, "%c", *posicion);
			i = i + 1;
		} else if (isspace(*posicion)) {
			//leo char espacio
			fprintf(nombrearchivo, "%c", *posicion);
			i = i + 1;
		} else {
			//leo int
			fprintf(nombrearchivo, "%d", *((int *) posicion));
			i = i + 4;
		}
	}
	fprintf(nombrearchivo, "\n<<FIN>>LECTURA-HEAVY\n");

}

void leerHeavyBytes(char * inicio, int offset, int cantidad, int reconoceBarra0) {

	int i = 0;
	int barra = reconoceBarra0;
	printf("<<INICIO>>LECTURA-HEAVY\n");

	for (i = 0;
			offset >= 0 && i < cantidad && (offset + i) <= configuracion.cantidadBytesMalloc;
			) {
		char * posicion = inicio + offset + i;
		if (*posicion == '\0' && barra == 1)
			i = cantidad;
		if (isalpha(*posicion)) {
			//leo char
			printf("%c", *posicion);
			i = i + 1;
		} else if (iscntrl(*posicion)) {
			//leo char especiales
			printf("%c", *posicion);
			i = i + 1;
		} else if (isdigit(*posicion)) {
			//leo char digito
			printf("%c", *posicion);
			i = i + 1;
		} else if (ispunct(*posicion)) {
			//leo char puntuacion
			printf("%c", *posicion);
			i = i + 1;
		} else if (isspace(*posicion)) {
			//leo char espacio
			printf("%c", *posicion);
			i = i + 1;
		} else {
			//leo int
			printf("%d", *((int *) posicion));
			i = i + 4;
		}
	}
	printf("\n<<FIN>>LECTURA-HEAVY\n");

}

void * leerBytes(int base, int offset, int cantidad, int PID) {
	int i;
	int existe = 0;
	estructuraDeSegmento *segmento = NULL;

	//printf("[UMV] - LEYENDO BYTES DE MEMORIA...\n");
	//log_info(archivo_log, "LEYENDO BYTES DE MEMORIA...");

	for (i = 0; i < list_size(listaDeSegmentos) && existe == 0; i++) {
		segmento = list_get(listaDeSegmentos, i);
		if (segmento->direccionVirtual == base && segmento->processID == PID) {
			existe = 1;
		}
	}
	////printf("\nEXISTE: %d, SEGMENTO: %d \n", existe, segmento->idSegmento);
	if (existe == 1) {
		if (offset >= 0 && segmento->tamanoSegmento >= (cantidad + offset)) {
			void *buffer = malloc(cantidad);
			memcpy(buffer, segmento->direccionReal + offset, cantidad);
			//*(&buffer + cantidad) = '\0';
			return buffer;
			//return (segmento->direccionReal + offset);
		}
	}
	return ((char*) 0);
}

int pegarBytes(void * bytesToCpy, int base, int offset, int cantidad, int PID) {
	int i;
	int existe = 0;
	estructuraDeSegmento *segmento = NULL;

	//printf("[UMV] - PEGANDO BYTES EN MEMORIA...\n");
	//log_info(archivo_log, "PEGANDO BYTES EN MEMORIA...");

	for (i = 0; i < list_size(listaDeSegmentos) && existe == 0; i++) {
		segmento = list_get(listaDeSegmentos, i);
		if (segmento->direccionVirtual == base && segmento->processID == PID) {
			existe = 1;
		}
	}
	if (existe == 1) {
		if (segmento->tamanoSegmento >= (cantidad + offset)) {
			memcpy(segmento->direccionReal + offset, bytesToCpy, cantidad);
			return 1;
		}
	}
	return 0;
}

void leerArchivoConfiguracionUMV(char *path) {
	t_config* config = config_create(path);
	if (config_has_property(config, "MEMORIA")) {
		configuracion.cantidadBytesMalloc = config_get_int_value(config,
				"MEMORIA");
	} else {
		//printf("[ERROR] - NO ME PIDIERON MEMORIA\n");
		log_error(archivo_log, "NO ME PIDIERON MEMORIA");
		//me cierro?
	}

	if (config_has_property(config, "RETARDO")) {	// 1000 es 1 segundo
		configuracion.retardo = config_get_int_value(config, "RETARDO");
	} else {
		//printf("[ERROR] - NO ME PIDIERON RETARDO\n");
		log_error(archivo_log, "NO ME PIDIERON RETARDO");
	}

	configuracion.ip = config_get_string_value(config, "IP");

	configuracion.puerto = config_get_string_value(config, "PUERTO");
	//printf("\tMEMORIA: %d\n", configuracion.cantidadBytesMalloc);
	//printf("\tRETARDO: %d\n", configuracion.retardo);
	//printf("\tIP: %s\n", configuracion.ip);
	//printf("\tPUERTO: %s\n", configuracion.puerto);
	log_info(archivo_log, "[DATOS]");
	log_info(archivo_log, "MEMORIA %d", configuracion.cantidadBytesMalloc);
	log_info(archivo_log, "RETARDO %d", configuracion.retardo);
	log_info(archivo_log, "IP %s", configuracion.ip);
	log_info(archivo_log, "PUERTO %d", configuracion.ip);

	//config_destroy(config);

}

char * virtualAReal(int base) {
	int tamanoLista = list_size(listaDeSegmentos);
	int i;
	estructuraDeSegmento *segmento;

	for (i = 0; i < tamanoLista; i++) {
		segmento = list_get(listaDeSegmentos, i);
		if (segmento->direccionVirtual == base) {
			return segmento->direccionReal;
		}
	}
	return ((void *) 0);
}

void imprimeSegmentos() {
	imprimeSegmentosPID(0);
}

void imprimeSegmentosARCHIVO(char nombreDump[50], int pid) {
	int tamanoLista = list_size(listaDeSegmentos);
	int i;
	estructuraDeSegmento *segmento;
	FILE *nombrearchivo = fopen(nombreDump, "w");
	fprintf(nombrearchivo,
			"ID \t PROCESSID \t VIRTUAL \t\t TAMAÑO \t REAL \t\t\t FIN REAL\n");

	if (tamanoLista > 0) {
		for (i = 0; i < tamanoLista; i++) {
			segmento = list_get(listaDeSegmentos, i);
			if (segmento->processID == pid || pid == 0) {
				if (segmento->processID == -1) {
					fprintf(nombrearchivo, "%d \t %d \t\t\t\t %d ",
							segmento->idSegmento, segmento->processID,
							segmento->direccionVirtual);

				} else {
					fprintf(nombrearchivo, "%d \t %d \t\t %d ",
							segmento->idSegmento, segmento->processID,
							segmento->direccionVirtual);
				}
				if (segmento->direccionVirtual <= 99999)
					fprintf(nombrearchivo, "\t");
				if (segmento->direccionVirtual == 0)
					fprintf(nombrearchivo, "\t\t");
				if (segmento->tamanoSegmento < 100) {
					fprintf(nombrearchivo, "\t %d \t\t\t %p \t %p \n",
							segmento->tamanoSegmento, segmento->direccionReal,
							segmento->direccionReal + segmento->tamanoSegmento);
				} else {
					fprintf(nombrearchivo, "\t %d \t\t %p \t %p \n",
							segmento->tamanoSegmento, segmento->direccionReal,
							segmento->direccionReal + segmento->tamanoSegmento);
				}
			}
		}
	} else {
		fprintf(nombrearchivo, "[UMV] - NO HAY SEGMENTOS CREADOS\n");
	}
	fprintf(nombrearchivo, "\n\n");
	if (pid == 0) {
		fprintf(nombrearchivo, "REPORTE:\n");
		fprintf(nombrearchivo, " \t 1) TAMAÑO DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc);
		fprintf(nombrearchivo, " \t 2) TAMAÑO USADO DEL MALLOC: %d\n",
				tamanoUsadoDelMalloc());
		fprintf(nombrearchivo, " \t 3) TAMAÑO LIBRE DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc - tamanoUsadoDelMalloc());
	}

	fprintf(nombrearchivo, "\n\n");
	fclose(nombrearchivo);

}

void imprimeSegmentosPID(int pid) {
	int tamanoLista = list_size(listaDeSegmentos);
	int i;
	estructuraDeSegmento *segmento;
	printf("ID \t PROCESSID \t VIRTUAL \t TAMAÑO \t REAL \t\t FIN REAL\n");
	log_info(archivo_log,
			"ID \t PROCESSID \t VIRTUAL \t TAMAÑO \t REAL \t\t FIN REAL");

	if (tamanoLista > 0) {
		for (i = 0; i < tamanoLista; i++) {
			segmento = list_get(listaDeSegmentos, i);
			if (segmento->processID == pid || pid == 0) {
				printf("%d \t %d \t\t %d ", segmento->idSegmento,
						segmento->processID, segmento->direccionVirtual);
				if (segmento->direccionVirtual <= 99999) {
					printf("\t");
					log_info(archivo_log,
							"%d \t %d \t\t %d \t\t %d \t\t %p \t %p",
							segmento->idSegmento, segmento->processID,
							segmento->direccionVirtual,
							segmento->tamanoSegmento, segmento->direccionReal,
							segmento->direccionReal + segmento->tamanoSegmento);
				} else {
					log_info(archivo_log,
							"%d \t %d \t\t %d \t %d \t\t %p \t %p",
							segmento->idSegmento, segmento->processID,
							segmento->direccionVirtual,
							segmento->tamanoSegmento, segmento->direccionReal,
							segmento->direccionReal + segmento->tamanoSegmento);
				}
				printf("\t %d \t\t %p \t %p \n", segmento->tamanoSegmento,
						segmento->direccionReal,
						segmento->direccionReal + segmento->tamanoSegmento);
			}
		}
	} else {
		printf("[UMV] - NO HAY SEGMENTOS CREADOS\n");
		log_warning(archivo_log, "NO HAY SEGMENTOS CREADOS");
	}

	if (pid == 0) {
		printf("\nREPORTE:\n");
		log_info(archivo_log, "\nREPORTE:\n");
		printf(" \t 1) TAMAÑO DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc);
		log_info(archivo_log, " \t 1) TAMAÑO DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc);
		printf(" \t 2) TAMAÑO USADO DEL MALLOC: %d\n", tamanoUsadoDelMalloc());
		log_info(archivo_log, " \t 2) TAMAÑO USADO DEL MALLOC: %d\n",
				tamanoUsadoDelMalloc());
		printf(" \t 3) TAMAÑO LIBRE DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc - tamanoUsadoDelMalloc());
		log_info(archivo_log, " \t 3) TAMAÑO LIBRE DEL MALLOC: %d\n",
				configuracion.cantidadBytesMalloc - tamanoUsadoDelMalloc());
	}

	retardo(configuracion.retardo);

}

procesoActivo cambiarProceso(procesoActivo estado, int pidNuevo) {
	estado.pidProcesoActivo = pidNuevo;
	return estado;
}

void cambiarAlgoritmo(int nuevoAlgoritmo) {

	if (algoritmoActivo == nuevoAlgoritmo) {
		//printf("[UMV] - NO SE CAMBIO EL ALGORITMO (era el mismo).\n");
		log_warning(archivo_log, "NO SE CAMBIO EL ALGORITMO (era el mismo)");
	} else {
		if (nuevoAlgoritmo == 1) {
			algoritmoActivo = 1;
			//printf("[UMV] - SE CAMBIO EL ALGORITMO A FIRST-FIT.\n");
			log_info(archivo_log, "SE CAMBIO EL ALGORITMO A FIRST-FIT");
		} else if (nuevoAlgoritmo == 2) {
			algoritmoActivo = 2;
			//printf("[UMV] - SE CAMBIO EL ALGORITMO A WORST-FIT.\n");
			log_info(archivo_log, "SE CAMBIO EL ALGORITMO A WORST-FIT");
		} else {
			//printf("[ERROR] - ALGORITMO INEXISTENTE.\n");
			log_error(archivo_log, "ALGORITMO INEXISTENTE");
		}
	}
	retardo(configuracion.retardo);
}

void destruirSegmentosDeUnPrograma(int pid) {
	int tamanoLista = list_size(listaDeSegmentos);
	int i;
	estructuraDeSegmento *segmento;
	//printf("[UMV] - DESTRUYENDO SEGMENTOS DEL PROGRAMA PID: %d\n", pid);
	//log_info(archivo_log, "DESTRUYENDO SEGMENTOS DEL PROGRAMA PID: %d", pid);
	for (i = 0; i < tamanoLista; i++) {
		segmento = list_get(listaDeSegmentos, i);
		if (segmento->processID == pid) {
			segmento->processID = -1;
		}
	}
}

void almacenarBytes(void* base, int offset, int tamano, void* buffer) {
	void* posicionMemoria = base + offset;
	memcpy(posicionMemoria, buffer, tamano);
	//printf("[UMV] - SE ALMACENARON %d BYTES EN LA MEMORIA\n", tamano);
	log_info(archivo_log, "SE ALMACENARON %d BYTES EN LA MEMORIA", tamano);
	retardo(configuracion.retardo);
}

void retardo(int milisegundos) {
	sleep(milisegundos / 1000); // el tiempo se recibe en milisegundos y la funcion sleep
	//funciona en segundos (1 segundo son 1000 milisegundos) asi que divido por 1000
}

void actualizarSegmento(void* memoriaDestino, estructuraDeSegmento *nodoOrigen) {
	//printf("[UMV] - ACTUALIZANDO SEGMENTO\n");
	//log_info(archivo_log, "ACTUALIZANDO SEGMENTO");
	memcpy(memoriaDestino, nodoOrigen->direccionReal,
			nodoOrigen->tamanoSegmento);
	nodoOrigen->direccionReal = memoriaDestino;
}

void eliminaVacios() {
	int tamanoLista = list_size(listaDeSegmentos), i = 0;
	estructuraDeSegmento *nodo;

	//printf("[UMV] - ELIMINANDO SEGMENTOS \"VACIOS\"\n");
	log_info(archivo_log, "ELIMINANDO SEGMENTOS \"VACIOS\"");
	for (i = 0; i < tamanoLista; i++) {
		nodo = list_get(listaDeSegmentos, i);
		if (nodo->processID == -1) {
			list_remove(listaDeSegmentos, i);
			i--;
			tamanoLista--;
		}

	}

}

void compactador() {

	char *memoriaMalloc = configuracion.inicioMalloc;
	int memoriaUsada;
	int i, tamanoLista;

	eliminaVacios();
	tamanoLista = list_size(listaDeSegmentos);

	//printf("[UMV] - COMPACTANDO MEMORIA...\n");
	log_info(archivo_log, "COMPACTANDO MEMORIA...");

	if (tamanoLista > 0) {
		estructuraDeSegmento *nodo = list_get(listaDeSegmentos, 0);
		estructuraDeSegmento *nodoSiguiente;

		if (nodo->direccionReal > memoriaMalloc) {
			actualizarSegmento(memoriaMalloc, nodo);
		}
		for (i = 0; (i + 1) < tamanoLista; i++) {
			nodo = list_get(listaDeSegmentos, i);
			nodoSiguiente = list_get(listaDeSegmentos, i + 1);
			char *finSegmentoAnterior = nodo->direccionReal
					+ nodo->tamanoSegmento;

			if ((finSegmentoAnterior) < nodoSiguiente->direccionReal) {
				actualizarSegmento(finSegmentoAnterior, nodoSiguiente);
			}

		}

		// ULTIMO SEGMENTO
		nodo = list_get(listaDeSegmentos, tamanoLista - 1);
		memoriaUsada = tamanoUsadoDelMalloc();

		if (memoriaUsada < configuracion.cantidadBytesMalloc) {
			estructuraDeSegmento segmentoVacio;
			segmentoVacio.direccionReal = nodo->direccionReal
					+ nodo->tamanoSegmento;
			segmentoVacio.tamanoSegmento = configuracion.cantidadBytesMalloc
					- memoriaUsada;
			segmentoVacio.direccionVirtual = 0;
			idSegmento++;
			segmentoVacio.idSegmento = idSegmento;
			segmentoVacio.processID = -1;
			segmento_create(segmentoVacio, listaDeSegmentos);
		}

	} else {
		estructuraDeSegmento segmentoVacio;
		segmentoVacio.direccionReal = configuracion.inicioMalloc;
		segmentoVacio.tamanoSegmento = configuracion.cantidadBytesMalloc;
		segmentoVacio.direccionVirtual = 0;
		idSegmento++;
		segmentoVacio.idSegmento = idSegmento;
		segmentoVacio.processID = -1;
		segmento_create(segmentoVacio, listaDeSegmentos);
	}
}

int tamanoUsadoDelMalloc() {
	int tamanoLista = list_size(listaDeSegmentos);
	int i = 0, tamano = 0;
	estructuraDeSegmento *nodo;

	for (i = 0; i < tamanoLista; i++) {
		nodo = list_get(listaDeSegmentos, i);
		if (nodo->processID != -1) {
			tamano += nodo->tamanoSegmento;
		}
	}
	return tamano;
}

void segmento_create(estructuraDeSegmento datos, t_list* lista) {
	estructuraDeSegmento *new = malloc(sizeof(estructuraDeSegmento));
	new->processID = datos.processID;
	new->direccionVirtual = datos.direccionVirtual;
	new->tamanoSegmento = datos.tamanoSegmento;
	new->direccionReal = datos.direccionReal;
	new->idSegmento = datos.idSegmento;
	list_add(lista, new);
}

bool auxiliarParaOrdenarListaDeSegmentos(estructuraDeSegmento* seg1,
		estructuraDeSegmento* seg2) {
	return seg1->direccionReal < seg2->direccionReal;
}

bool dirVirEstaEnUMV(int numero) {
	int i;
	bool existe = false;
	estructuraDeSegmento *segmento;
	//printf("[UMV] - BUSCANDO DIRECCION VIRTUAL\n");
	//log_info(archivo_log, "BUSCANDO DIRECCION VIRTUAL");

	for (i = 0; i < list_size(listaDeSegmentos) && existe == false; i++) {
		segmento = list_get(listaDeSegmentos, i);
		if (segmento->direccionVirtual == numero) {
			existe = true;
		}
	}
	return existe;
}

estructuraDeSegmento firstFit(estructuraDeSegmento datosSegmento) {
	list_sort(listaDeSegmentos, (void*) auxiliarParaOrdenarListaDeSegmentos);
	bool bandera = false;
	int i = 0;
	int dirVir = 0;
	bool banderaExiste = false;
	estructuraDeSegmento *datosDelVacio;
	estructuraDeSegmento segmentoVacio;
	int tamanoLista = list_size(listaDeSegmentos);

	//printf("[UMV] - CREANDO SEGMENTO FIRST-FIT\n");
	log_info(archivo_log, "CREANDO SEGMENTO FIRST-FIT");
	//primer segmento en la UMV
	if (tamanoLista == 0
			&& datosSegmento.tamanoSegmento
					<= configuracion.cantidadBytesMalloc) {
		datosSegmento.direccionReal = configuracion.inicioMalloc;
		datosSegmento.idSegmento = idSegmento;

		while (dirVir == 0) {
			dirVir = rand();
		}

		datosSegmento.direccionVirtual = dirVir;
		idSegmento++;
		segmentoVacio.direccionReal = datosSegmento.direccionReal
				+ datosSegmento.tamanoSegmento;
		segmentoVacio.tamanoSegmento = configuracion.cantidadBytesMalloc
				- datosSegmento.tamanoSegmento;
		segmentoVacio.direccionVirtual = 0;
		segmentoVacio.idSegmento = idSegmento;
		segmentoVacio.processID = -1;
		segmento_create(datosSegmento, listaDeSegmentos);
		segmento_create(segmentoVacio, listaDeSegmentos);
		bandera = true;

	}
	// si hay algo en la umv entra en el for
	for (i = 0; i < tamanoLista && bandera != true; i++) {
		nodoDeLista = list_get(listaDeSegmentos, i);
		if (nodoDeLista->processID > 0) //el segmento no está vacío
				{
			bandera = false; //no debo hacer nada, entonces paso la posicion
		} else {
			//el segmento esta vacio y entra el segmento que pido
			if (nodoDeLista->tamanoSegmento >= datosSegmento.tamanoSegmento) {
				banderaExiste = true;
				idSegmento++;
				datosSegmento.direccionReal = nodoDeLista->direccionReal;
				datosSegmento.idSegmento = idSegmento;
				while (dirVir == 0 && banderaExiste == true) {
					dirVir = rand();
					banderaExiste = dirVirEstaEnUMV(dirVir);
				}
				datosSegmento.direccionVirtual = dirVir;
				segmento_create(datosSegmento, listaDeSegmentos);
				datosDelVacio = list_remove(listaDeSegmentos, i);
				segmentoVacio = *datosDelVacio;
				segmentoVacio.direccionReal = datosSegmento.direccionReal
						+ datosSegmento.tamanoSegmento;
				segmentoVacio.tamanoSegmento = segmentoVacio.tamanoSegmento
						- datosSegmento.tamanoSegmento;
				segmento_create(segmentoVacio, listaDeSegmentos);
				bandera = true;
			}
		}
	}
	return datosSegmento;
}

estructuraDeSegmento worstFit(estructuraDeSegmento datosSegmento) {
	list_sort(listaDeSegmentos, (void*) auxiliarParaOrdenarListaDeSegmentos);
	int i = 0;
	int dirVir = 0;
	bool bandera = false;
	bool banderaExiste = false;
	estructuraDeSegmento segmentoVacio;
	estructuraDeSegmento *datosDelVacioAnterior;
	estructuraDeSegmento segmentoVacioMaximo;
	int posicionDelVacioMaximo = 0;
	int tamanoLista = list_size(listaDeSegmentos);

	segmentoVacioMaximo.tamanoSegmento = 0;
	//printf("[UMV] - CREANDO SEGMENTO WORST-FIT\n");
	log_info(archivo_log, "CREANDO SEGMENTO WORST-FIT");
	//primer segmento en la UMV
	if (tamanoLista == 0
			&& datosSegmento.tamanoSegmento
					<= configuracion.cantidadBytesMalloc) {
		datosSegmento.direccionReal = configuracion.inicioMalloc;
		datosSegmento.idSegmento = idSegmento;

		while (dirVir == 0) {
			dirVir = rand();
		}

		datosSegmento.direccionVirtual = dirVir;
		idSegmento++;
		segmentoVacio.direccionReal = datosSegmento.direccionReal
				+ datosSegmento.tamanoSegmento;
		segmentoVacio.tamanoSegmento = configuracion.cantidadBytesMalloc
				- datosSegmento.tamanoSegmento;
		segmentoVacio.direccionVirtual = 0;
		segmentoVacio.idSegmento = idSegmento;
		segmentoVacio.processID = -1;
		segmento_create(datosSegmento, listaDeSegmentos);
		segmento_create(segmentoVacio, listaDeSegmentos);
		bandera = true;

	}
	// si hay algo en la umv entra en el for
	for (i = 0; i < tamanoLista && bandera != true; i++) {
		nodoDeLista = list_get(listaDeSegmentos, i);
		if (nodoDeLista->processID > 0) //el segmento no está vacío
				{
			bandera = false; //no debo hacer nada, entonces paso la posicion
		} else {
			//el segmento esta vacio y entra el segmento que pido
			if (nodoDeLista->tamanoSegmento
					> segmentoVacioMaximo.tamanoSegmento) { // toma el primero mas grande
				posicionDelVacioMaximo = i;
				segmentoVacioMaximo = *nodoDeLista;
				banderaExiste = true;
			}
		}
	}

	if (banderaExiste == true) {

		idSegmento++;
		datosSegmento.direccionReal = segmentoVacioMaximo.direccionReal;
		datosSegmento.idSegmento = idSegmento;
		while (dirVir == 0 && banderaExiste == true) {
			dirVir = rand();
			banderaExiste = dirVirEstaEnUMV(dirVir);
		}
		datosSegmento.direccionVirtual = dirVir;
		segmento_create(datosSegmento, listaDeSegmentos);
		datosDelVacioAnterior = list_remove(listaDeSegmentos,
				posicionDelVacioMaximo);
		segmentoVacio = *datosDelVacioAnterior;
		segmentoVacio.direccionReal = datosSegmento.direccionReal
				+ datosSegmento.tamanoSegmento;
		segmentoVacio.tamanoSegmento = segmentoVacio.tamanoSegmento
				- datosSegmento.tamanoSegmento;
		segmento_create(segmentoVacio, listaDeSegmentos);
		bandera = true;
	}

	return datosSegmento;
}

int crearSegmento(int pid, int tamano) {
	estructuraDeSegmento segmentoACrear;
	segmentoACrear.processID = pid;
	segmentoACrear.tamanoSegmento = tamano;

	if ((tamanoUsadoDelMalloc() + tamano)
			<= configuracion.cantidadBytesMalloc) {
		if (algoritmoActivo == 1) {
			segmentoACrear = firstFit(segmentoACrear);
		} else {
			segmentoACrear = worstFit(segmentoACrear);
		}
		//printf("[UMV] - SEGMENTO CREADO DEL PID: %d, TAMAÑO: %d\n", pid,
		//		tamano);
		log_info(archivo_log, "SEGMENTO CREADO DEL PID: %d, TAMAÑO: %d", pid,
				tamano);
		retardo(configuracion.retardo);
		return segmentoACrear.direccionVirtual;
	} else {
		//printf("[ERROR] - NO SE PUDO CREAR EL SEGMENTO DEL TAMAÑO PEDIDO.\n");
		log_error(archivo_log,
				"NO SE PUDO CREAR EL SEGMENTO DEL TAMAÑO PEDIDO");
		retardo(configuracion.retardo);
		return -1;
	}
}

char* pedirMemoria(int cantidad) {
	char *memoria;
	memoria = malloc(cantidad);
	return memoria;
}

// TEST

void testCargarConfiguracion(char *path) {
	//printf("[UMV] - LEO LAS CONFIGURACIONES.\n");
	log_info(archivo_log, "LEO LAS CONFIGURACIONES");
	leerArchivoConfiguracionUMV(path);

	//printf("[UMV] - PIDO MEMORIA DE %d.\n", configuracion.cantidadBytesMalloc);
	configuracion.inicioMalloc = pedirMemoria(
			configuracion.cantidadBytesMalloc);
	log_info(archivo_log, "PIDO MEMORIA DE %d.",
			configuracion.cantidadBytesMalloc);

	//printf("[UMV] - LA MEMORIA SOLICITADA EMPIEZA EN: %p Y TERMINA EN: %p \n",
	//		configuracion.inicioMalloc,
	//		configuracion.inicioMalloc + configuracion.cantidadBytesMalloc);
	log_info(archivo_log,
			"LA MEMORIA SOLICITADA EMPIEZA EN: %p Y TERMINA EN: %p",
			configuracion.inicioMalloc,
			configuracion.inicioMalloc + configuracion.cantidadBytesMalloc);

	//printf("[UMV] - CREO LA LISTA DE SEGMENTOS.\n");
	log_info(archivo_log, "CREO LA LISTA DE SEGMENTOS");
	listaDeSegmentos = list_create();

	//printf("[UMV] - INICIO LA VARIABLE ALGORITMO.\n");
	log_info(archivo_log, "INICIO LA VARIABLE ALGORITMO");

	cambiarAlgoritmo(1);
	retardo(configuracion.retardo);
}

void testCreaSegmento(int numeroSegmento, int tamanoSegmento) {
	//log_info(archivo_log, "CREO EL SEGMENTO %d DE TAMAÑO %d", numeroSegmento,
	//		tamanoSegmento);
	if (crearSegmento(numeroSegmento, tamanoSegmento) < 0) {
		printf("[ERROR] - NO SE CREO EL SEGMENTO %d DE TAMAÑO %d.\n",
				numeroSegmento, tamanoSegmento);
	} else {
		printf("[UMV] - CREO EL SEGMENTO %d DE TAMAÑO %d.\n", numeroSegmento,
				tamanoSegmento);
	}
}

void testTamanoUsado() {
	int a = tamanoUsadoDelMalloc();
	printf("[UMV] - TAMAÑO USADO DE LA MEMORIA %d.\n", a);
	log_info(archivo_log, "TAMAÑO USADO DE LA MEMORIA %d", a);
	retardo(configuracion.retardo);
}

void testEliminaSegmento(int numero) {
	estructuraDeSegmento *nodoAMover;

	printf("[UMV] - ELIMINO EL SEGMENTO %d.\n", numero);
	log_info(archivo_log, "ELIMINO EL SEGMENTO %d", numero);
	nodoAMover = list_remove(listaDeSegmentos, numero);
	nodoAMover->processID = -1;
	list_add(listaDeSegmentos, nodoAMover);
	retardo(configuracion.retardo);
}

void testDestruirSegmentosDeUnPrograma(int numero) {
	//log_info(archivo_log, "ELIMINO LOS SEGMENTOS DEL PROGRAMA PID: %d", numero);
	destruirSegmentosDeUnPrograma(numero);
	retardo(configuracion.retardo);
}

void testCompactador() {
	//printf("[UMV] - ESTADO DE LOS SEGMENTOS  A N T E S  DE COMPACTAR.\n");
	log_info(archivo_log, "ESTADO DE LOS SEGMENTOS  A N T E S  DE COMPACTAR");
	imprimeSegmentos();
	//printf("[UMV] - ESTADO DE LOS SEGMENTOS  D E S P U E S  DE COMPACTAR.\n");
	log_info(archivo_log,
			"ESTADO DE LOS SEGMENTOS  D E S P U E S  DE COMPACTAR");
	compactador();
	imprimeSegmentos();
}

void testCompactador2() {
	printf("[UMV] - ESTADO DE LOS SEGMENTOS  A N T E S  DE COMPACTAR.\n");
	log_info(archivo_log, "ESTADO DE LOS SEGMENTOS  A N T E S  DE COMPACTAR");
	imprimeSegmentos();
	printf("[UMV] - ESTADO DE LOS SEGMENTOS  D E S P U E S  DE COMPACTAR.\n");
	log_info(archivo_log,
			"ESTADO DE LOS SEGMENTOS  D E S P U E S  DE COMPACTAR");
	compactador();
	imprimeSegmentos();
}

void test() {
	testCreaSegmento(1, 100);
	testTamanoUsado();
	testCreaSegmento(1, 100);
	testTamanoUsado();
	testCreaSegmento(3, 100);
	testCreaSegmento(3, 100);

	testEliminaSegmento(0);
	testEliminaSegmento(1);

	testCreaSegmento(4, 50);
	testCreaSegmento(4, 70);
	testDestruirSegmentosDeUnPrograma(4);
	testCreaSegmento(5, 150);

	//printf(" === === FIN TEST === === \n");
}

void escucharConexion() {

	while (1) {
		FD_ZERO(&descriptoresLectura);
		FD_SET(socketEscucha, &descriptoresLectura);

		socketActivo = select(socketEscucha + 1, &descriptoresLectura, NULL,
				NULL, NULL );

		if (socketActivo < 0) {
			//error
			printf("[ERROR] - FUNCION SELECT().\n");
			log_error(archivo_log, "FUNCION SELECT()");
			pthread_exit(NULL );
		}
		if (FD_ISSET(socketEscucha,&descriptoresLectura)) {
			//UNA NUEVA CPU INTENTA CONECTARSE
			atenderConexion(socketEscucha); //aceptar la conexion
		}

	}
}

void atenderConexion(int listener) {
	int soket;
	pthread_t threadHilo;
	extern int socketKernel;
	extern int socketHilo;
	t_datosEnviar *mensaje = malloc(sizeof(t_datosEnviar));
	struct sockaddr_in dirClienteEntrante;
	int tam = sizeof(struct sockaddr_in);

	soket = accept(listener, (struct sockaddr *) &dirClienteEntrante,
			(void*) &tam);
	mensaje = common_receive(soket, NULL );

	if (mensaje->codigo_Operacion == UMV_ACEPTA_KERNEL) { 		// con KERNEL
		socketKernel = soket;
		common_send(socketKernel, mensaje, NULL );
		//printf("[UMV] - SE ACEPTO AL KERNEL\n");
		log_info(archivo_log, "SE ACEPTO AL KERNEL");
		pthread_create(&threadHilo, NULL, &hiloKernel, &socketKernel);
	}
	if (mensaje->codigo_Operacion == UMV_ACEPTA_CPU) {				// con CPU
		socketHilo = soket;
		mensaje->codigo_Operacion = UMV_ACEPTA_CPU;
		mensaje->data = "OK";
		mensaje->data_size = 3;
		common_send(soket, mensaje, NULL ); //CONEXION CREADA CON EXITO
		//printf("[UMV] - SE ACEPTO UN NUEVO CPU\n");
		log_info(archivo_log, "SE ACEPTO UN NUEVO CPU");
		pthread_create(&threadHilo, NULL, &cadaCPU, &socketHilo);
	}
}

void *hiloKernel(void * socketParametro) {
	int socketKernel;
	memcpy(&socketKernel, socketParametro, sizeof(int));
	retardo(configuracion.retardo);
	while (1) {
		int pid;
		int tamano;
		int dirLog;
		int segmento;
		void * data;
		t_datosEnviar *mensaje; // = malloc(sizeof(t_datosEnviar));
		mensaje = common_receive(socketKernel, NULL );
		pthread_mutex_lock(&semaforo);

		switch (mensaje->codigo_Operacion) {
		case KERNEL_ESCRIBIR_SEGMENTO_UMV:
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&tamano, mensaje->data + sizeof(int), sizeof(int));
			memcpy(&segmento, mensaje->data + 2 * sizeof(int), sizeof(int));
			data = malloc(tamano);
			memcpy(data, mensaje->data + 3 * sizeof(int), tamano);

			if (tamano == 0) {
				dirLog = -1;
				mensaje->codigo_Operacion = KERNEL_PEDIR_SEGMENTO_UMV;
				mensaje->data = malloc(sizeof(dirLog));
				memcpy(mensaje->data, &dirLog, sizeof(dirLog));
				mensaje->data_size = sizeof(dirLog);
				common_send(socketKernel, mensaje, NULL );
				break;
			} else {

				if (pegarBytes(data, segmento, 0, tamano, pid) == 1) { // tamano solo de los caractes sin el /0
					//printf(
					//		"[Del KERNEL] - ESCRIBE - SEGMENTO: %d, TAMAÑO: %d\n",
					//		segmento, tamano); //%s\n", (char *) data);

					//leerHeavyBytes(virtualAReal(segmento),0,tamano,0);

					log_info(archivo_log,
							"[Del KERNEL] - ESCRIBE - SEGMENTO: %d, TAMAÑO: %d\n",
							segmento, tamano); //%s",	(char *) data);
					mensaje = pedirPaquete(strdup("OK"),
							KERNEL_ESCRIBIR_SEGMENTO_UMV, 3);
				} else {
					//printf("[Del KERNEL] - ESCRIBE - SIN AUTORIZACION\n");
					log_error(archivo_log,
							"[Del KERNEL] - ESCRIBE - SIN AUTORIZACION");
					mensaje = pedirPaquete(strdup("SF"), ERROR_FALLO_SEGMENTO,
							3);
				}
				common_send(socketKernel, mensaje, NULL );
				free(data);
			}
			break;
		case KERNEL_PEDIR_SEGMENTO_UMV:
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&tamano, mensaje->data + sizeof(pid), sizeof(int));
			if (tamano != 0) {
				dirLog = crearSegmento(pid, tamano);
			} else {
				dirLog = -1;
				mensaje->codigo_Operacion = KERNEL_PEDIR_SEGMENTO_UMV;
				mensaje->data = malloc(sizeof(dirLog));
				memcpy(mensaje->data, &dirLog, sizeof(dirLog));
				mensaje->data_size = sizeof(dirLog);
				common_send(socketKernel, mensaje, NULL );
				break;
			}
			free(mensaje->data);

			//CREO EL PAQUETE
			mensaje->codigo_Operacion = KERNEL_PEDIR_SEGMENTO_UMV;
			mensaje->data = malloc(sizeof(dirLog));
			memcpy(mensaje->data, &dirLog, sizeof(dirLog));
			mensaje->data_size = sizeof(dirLog);

			if (dirLog >= 0) {
				//printf(
				//		"[Del KERNEL] - SEGMENTO CREADO: PID: %d, TAMAÑO: %d, DIRVIRTUAL: %d.\n",
				//		pid, tamano, dirLog);
				log_info(archivo_log,
						"[Del KERNEL] - SEGMENTO CREADO: PID: %d, TAMAÑO: %d, DIRVIRTUAL: %d.",
						pid, tamano, dirLog);
			} else {
				//COMPACTO
				//printf(
				//		"[UMV] - NO HAY ESPACIO PARA UN SEGMENTO DE %d, ME   C O M P A C T O !!.\n",
				//		tamano);
				log_warning(archivo_log,
						"NO HAY ESPACIO PARA UN SEGMENTO DE %d, ME   C O M P A C T O !!",
						tamano);
				testCompactador();

				//INTENTO CREAR NUEVAMENTE
				int dirLog = crearSegmento(pid, tamano);
				memcpy(mensaje->data, &dirLog, sizeof(dirLog));

				if (dirLog >= 0) {
					//printf(
					//		"[Del KERNEL] - SEGMENTO CREADO: PID: %d, TAMAÑO: %d, DIRVIRTUAL: %d.\n",
					//		pid, tamano, dirLog);
					log_info(archivo_log,
							"[Del KERNEL] - SEGMENTO CREADO: PID: %d, TAMAÑO: %d, DIRVIRTUAL: %d",
							pid, tamano, dirLog);

				} else {
					mensaje = pedirPaquete(strdup("NO"), ERROR_NO_CREO_SEGMENTO,
							3);
					//printf("[Del KERNEL] - SEGMENTO NO CREADO.\n");
					log_error(archivo_log, "SEGMENTO NO CREADO");
				}

			}
			common_send(socketKernel, mensaje, NULL );
			break;
		case KERNEL_DESTRUIR_SEGMENTOS_PROGRAMA:
			memcpy(&pid, mensaje->data, sizeof(int));
			testDestruirSegmentosDeUnPrograma(pid);
			//printf(
			//		"[Del KERNEL] - SE DESTRUYERON LOS SEGMENTOS DEL PROGRAMA PID: %d.\n",
			//		pid);
			log_info(archivo_log,
					"[Del KERNEL] - SE DESTRUYERON LOS SEGMENTOS DEL PROGRAMA PID: %d",
					pid);
			break;
		default:
			//printf("[Del KERNEL] - CONDIGO INVALIDO.\n");
			log_warning(archivo_log, "[Del KERNEL] - CONDIGO INVALIDO");
			break;
		}
		pthread_mutex_unlock(&semaforo);
		destruirPaquete(mensaje);
		retardo(configuracion.retardo);
	}
	return NULL ;
}

void *cadaCPU(void *socketParametro) {
	int socketCPU;
	int PIDProcesoActivo;
	int anterior = 0;
	extern fd_set descriptoresLectura;
	memcpy(&socketCPU, socketParametro, sizeof(int));
	retardo(configuracion.retardo);

	while (1) {
		int segmento, offset, tamano;
		void * data;
		t_datosEnviar *mensaje;

		mensaje = common_receive(socketCPU, NULL );
		pthread_mutex_lock(&semaforo);
		switch (mensaje->codigo_Operacion) {
		case UMV_MENSAJE_CPU_ESCRIBIR:

			memcpy(&segmento, mensaje->data, sizeof(int));
			memcpy(&offset, mensaje->data + sizeof(int), sizeof(int));
			memcpy(&tamano, mensaje->data + sizeof(int) + sizeof(int),
					sizeof(int));
			data = malloc(tamano);
			memcpy(data,
					mensaje->data + sizeof(int) + sizeof(int) + sizeof(int),
					tamano);

			if (pegarBytes(data, segmento, offset, tamano, PIDProcesoActivo)
					== 1) { // tamano solo de los caractes sin el /0
				//printf(
				//		"[Del CPU] - ESCRIBE - SEGMENTO: %d, OFFSET: %d, TAMAÑO: %d\n",
				//		segmento, offset, tamano); //");//%d\n", *((int *) data));
				//log_info(archivo_log,
				//		"[Del CPU] - ESCRIBE - SEGMENTO: %d, OFFSET: %d, TAMAÑO: %d\n",
				//		segmento, offset, tamano);
				mensaje = pedirPaquete(strdup("OK"), UMV_MENSAJE_CPU_ESCRIBIR,
						3);
			} else {
				//printf("[Del CPU] - ESCRIBE - SIN AUTORIZACION\n");
				log_error(archivo_log,
						"[Del CPU] - ESCRIBE - SIN AUTORIZACION");
				mensaje = pedirPaquete(strdup("SF"), ERROR_FALLO_SEGMENTO, 3);
			}
			common_send(socketCPU, mensaje, NULL );
			free(data);

			break;
		case UMV_MENSAJE_CPU_LEER:
			memcpy(&segmento, mensaje->data, sizeof(int));
			memcpy(&offset, mensaje->data + sizeof(int), sizeof(int));
			memcpy(&tamano, mensaje->data + sizeof(int) + sizeof(int),
					sizeof(int));
			destruirPaquete(mensaje);

			data = leerBytes(segmento, offset, tamano, PIDProcesoActivo);

			if (data == ((void*) 0)) {
				//printf("[Del CPU] - LEE - NO PUEDE LEER, SIN AUTORIZACION\n");
				log_error(archivo_log,
						"[Del CPU] - NO PUEDE LEER, SIN AUTORIZACION");
				mensaje = pedirPaquete(strdup("SF"), ERROR_FALLO_SEGMENTO, 3);
			} else {
				//printf(
				//		"[Del CPU] - LEE - SEGMENTO: %d, OFFSET: %d, TAMAÑO: %d\n",
				//		segmento, offset, tamano);
				//log_info(archivo_log,
				//		"[Del CPU] - LEE - SEGMENTO: %d, OFFSET: %d, TAMAÑO: %d\n",
				//		segmento, offset, tamano); // %s", (char *) data);
				if (tamano == 0) {
					mensaje = pedirPaquete(strdup(""), UMV_MENSAJE_CPU_LEER, 1);
				} else {
					mensaje = pedirPaquete(data, UMV_MENSAJE_CPU_LEER, tamano);
				}
			}
			common_send(socketCPU, mensaje, NULL );
			free(data);
			break;
		case UMV_PROCESO_ACTIVO_CPU:
			anterior = PIDProcesoActivo;
			memcpy(&PIDProcesoActivo, mensaje->data, sizeof(int));
			//printf("[Del CPU] - SE CAMBIO EL PROCESO ACTIVO DE %d a %d.\n",
			//		anterior, PIDProcesoActivo);
			log_info(archivo_log,
					"[Del CPU] - SE CAMBIO EL PROCESO ACTIVO DE %d a %d",
					anterior, PIDProcesoActivo);
			destruirPaquete(mensaje);

			mensaje = pedirPaquete(strdup("OK"), UMV_PROCESO_ACTIVO_CPU, 3);
			common_send(socketCPU, mensaje, NULL );
			break;
		case UMV_CAE_CPU:
			//printf("[Del CPU] - SE MURIO LA CONEXION.\n");
			log_info(archivo_log, "[Del CPU] - SE MURIO LA CONEXION");
			destruirPaquete(mensaje);
			close(socketCPU);
			FD_CLR(socketCPU, &descriptoresLectura);
			pthread_mutex_unlock(&semaforo);
			pthread_exit(NULL );
			break;
		default:
			//printf("[Del CPU] - CONDIGO INVALIDO.\n");
			log_warning(archivo_log, "[Del CPU] - CONDIGO INVALIDO");

			mensaje = pedirPaquete(strdup("FF"), ERROR_CODIGO_INVALIDO, 3);
			common_send(socketCPU, mensaje, NULL );
			break;
		}
		pthread_mutex_unlock(&semaforo);
		destruirPaquete(mensaje);
		retardo(configuracion.retardo);
	}
	return NULL ;
}
