/* decoracionDePaquetesCPU.c
 *
 *  Created on: Jun 19, 2014
 *      Author: root
 */

#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <conitos-estaCoverflow/conitos_generic.h>
#include <commons/string.h>
#include "decoracionDePaquetesCPU.h"
#include "estructuras.h"

/*
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 */

t_datosEnviar* enviar_y_esperar_respuesta_con_paquete(int socket,
		t_datosEnviar* paquete, int codOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)()) {
	common_send(socket, paquete, executeIfConnectionError);
	t_datosEnviar* nuevo_paquete = common_receive(socket,
			executeIfConnectionError);
	if (nuevo_paquete->codigo_Operacion == codOpProblematico)
		executeIfProblem();
	return nuevo_paquete;
}

t_datosEnviar* enviar_y_esperar_respuesta(int socket, void* datos,
		int codigoOperacion, t_size size, int codigoOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)()) {
	t_datosEnviar* paquete = pedirPaquete((void*) datos, codigoOperacion, size);
	t_datosEnviar* nuevo_paquete = enviar_y_esperar_respuesta_con_paquete(
			socket, paquete, codigoOpProblematico, executeIfConnectionError,
			executeIfProblem);
	free(paquete);
	return nuevo_paquete;
}

/*
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 */

t_datosEnviar* paquete_escribirUMV(t_puntero base, int32_t offset,
		t_size data_size, void* data) {
	t_direccion* direccion = conitos_malloc(sizeof *direccion);
	direccion->base = base;
	direccion->offset = offset;
	direccion->tamanio = data_size;
	t_datosEnviar* paquete = pedirPaquete(direccion, UMV_MENSAJE_CPU_ESCRIBIR,
			sizeof(t_direccion));
	aniadirAlPaquete(paquete,data,data_size);
	free(direccion);
	return paquete;
}

t_datosEnviar* paquete_leerUMV(t_puntero base, int32_t offset, t_size tamanio) {
	t_direccion *data = conitos_malloc(sizeof*data);
	data->base = base;
	data->offset = offset;
	data->tamanio = tamanio;
	t_datosEnviar* paquete = pedirPaquete(data, UMV_MENSAJE_CPU_LEER,
			sizeof(*data));
	free(data);
	return paquete;
}

t_datosEnviar* paquete_asignarCompartida(t_nombre_compartida variable,
		t_valor_variable valor) {
	t_datosEnviar* paquete = pedirPaquete((void*) variable,
			CPU_PCP_ASIGNAR_VARIABLE_COMPARTIDA, string_length(variable) + 1);
	aniadirAlPaquete(paquete, &valor, sizeof(t_valor_variable));
	return paquete;
}

/*
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 */
