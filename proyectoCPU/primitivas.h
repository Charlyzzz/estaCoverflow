/* primitivas.h
 *
 *  Created on: Jun 15, 2014
 *      Author: root
 */

#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include "estructuras.h"
#include <stdint.h>
#include "decoracionDePaquetesCPU.h"
#include "stack.h"
#include "common_execution.h"
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <conitos-estaCoverflow/conitos_generic.h>
#include <commons/string.h>
#include <parser/metadata_program.h>


t_puntero definir_variable(t_nombre_variable nombre);
t_puntero obtenerPosicionVariable(t_nombre_variable nombre);
t_valor_variable desreferenciar(t_puntero direccion);
void asignar(t_puntero direccion, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,
		t_valor_variable valor);
void irALabel(t_nombre_etiqueta etiqueta);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta,t_puntero dondeRetornar);
void finalizar();
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* Texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void p_signal(t_nombre_semaforo identificador_semaforo);
void p_wait(t_nombre_semaforo identificador_semaforo);














#endif /* PRIMITIVAS_H_ */
