/* paquetes.h
 *
 *  Created on: Jun 18, 2014
 *      Author: root
 */

#ifndef PAQUETES_H_
#include <conitos-estaCoverflow/common_sockets.h>
#include <parser/parser.h>
#define PAQUETES_H_


typedef struct direccion_stack {
	t_puntero base;
	int32_t offset;
	t_size tamanio;
} t_direccion;

typedef struct paquete_pop_stack {
	t_direccion direccion;
} t_datos_leer;

t_datosEnviar* enviar_y_esperar_respuesta(int socket, void* datos,
		int codigoOperacion, t_size size, int codigoOpProblematico,
		void (*executeIfError)(), void (*executeIfProblem)());
void* desempaquetarDatos(t_datosEnviar* paquete,
		int codigoOperacionProblematico, void (*executeIfProblem)());
t_datosEnviar* paquete_asignarCompartida(t_nombre_compartida variable,
		t_valor_variable valor);
t_datosEnviar* enviar_y_esperar_respuesta_con_paquete(int socket,
		t_datosEnviar* paquete, int codOpProblematico, void (*executeIfError)(),
		void (*executeIfProblem)());
t_datosEnviar* paquete_leerUMV(t_puntero base, int32_t offset, t_size tamanio);
t_datosEnviar* paquete_escribirUMV(t_puntero base, int32_t offset,
		t_size data_size, void* data);

#endif /* PAQUETES_H_ */
