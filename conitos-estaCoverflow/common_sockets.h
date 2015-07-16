/*
 * common_sockets.h
 *
 *  Created on: May 11, 2014
 *      Author: root
 */

#ifndef COMMON_SOCKETS_H_
#define COMMON_SOCKETS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef int t_codigoOperacion;
typedef int t_dataSize;

typedef struct {
	t_codigoOperacion codigo_Operacion;
	t_dataSize data_size;
	void* data;

} t_datosEnviar;

#define packet_header (sizeof(t_codigoOperacion) + sizeof(t_dataSize))

void destruirPaquete(t_datosEnviar*);
t_datosEnviar* pedirPaquete(void* data,t_codigoOperacion codigoOp, t_dataSize size);
struct addrinfo* common_setup(char *IP, char* Port);
int connect_to(char *IP, char* Port);
void common_send(int socket, t_datosEnviar* paquete,void (*executeIfError)());
t_datosEnviar* common_receive(int fd, void (*executeIfError)());
int setup_listen(char* IP,char* Port);
void aniadirAlPaquete(t_datosEnviar* paquete, void* datosExtra, t_dataSize sizeExtra);

#endif /* COMMON_SOCKETS_H_ */
