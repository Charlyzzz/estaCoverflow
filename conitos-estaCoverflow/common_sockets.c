/*
 * common_sockets.c
 *
 *  Created on: May 7, 2014
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/error.h>
#include <errno.h>
#include "conitos_generic.h"
#include "common_sockets.h"

/*
 * NOTA: Si no tiene codigo de Operacion lo que vas a mandar, TE JODES, la mayoria lo necesita, y no voy a hacer
 * otro serializador y deserializador para eso particularmente
 */
void aniadirAlPaquete(t_datosEnviar* paquete, void* datosExtra,
		t_dataSize sizeExtra)
{
	void* dataClumpExtra = conitos_malloc(paquete->data_size + sizeExtra);
	memcpy(dataClumpExtra, paquete->data, paquete->data_size);
	memcpy(dataClumpExtra + paquete->data_size, datosExtra, sizeExtra);
	free(paquete->data);
	paquete->data_size += sizeExtra;
	paquete->data = dataClumpExtra;
}

t_datosEnviar* pedirPaquete(void* data, t_codigoOperacion codigoOp,
		t_dataSize size)
{
	t_datosEnviar* paquete = conitos_malloc(sizeof(t_datosEnviar));
	paquete->codigo_Operacion = codigoOp;
	paquete->data_size = size;
	paquete->data = conitos_malloc(size);
	memcpy(paquete->data, data, size);
	return paquete;
}

void destruirPaquete(t_datosEnviar* paquete)
{
	free(paquete->data);
	free(paquete);
}

static char* common_serializer(t_datosEnviar* unPaquete)
{
	char* stream = conitos_malloc((packet_header) + unPaquete->data_size);
	memcpy(stream, unPaquete, packet_header);
	memcpy(stream + packet_header, unPaquete->data, unPaquete->data_size);
	return stream;
}

static t_datosEnviar* common_unserializeHeader(char* buffer)
{
	t_datosEnviar* unPaquete = conitos_malloc(sizeof(t_datosEnviar));
	memcpy(&unPaquete->codigo_Operacion, buffer, sizeof(t_codigoOperacion));
	memcpy(&unPaquete->data_size, buffer + sizeof(t_codigoOperacion),
			sizeof(t_dataSize));
	return unPaquete;
}

static void common_unserializeData(char* buffer, t_datosEnviar* unPaquete)
{
	unPaquete->data = conitos_malloc(unPaquete->data_size);
	memcpy(unPaquete->data, buffer, unPaquete->data_size);
}

/**  @NAME: common_setup
 *	 @DESC: Retorna un puntero a una addrInfo totalmente lista (Si, es una lista y veo el pun)
 *	 para usar, dado una IP y un Host cualesquiera.
 *	 No deberia ser usado fuera de common_sockets.
 *	 @RETURN: Devuelve NULL si falla.
 */

struct addrinfo* common_setup(char *IP, char* Port)
{
	struct addrinfo hints;
	struct addrinfo* serverInfo;
	int16_t error;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (!strcmp(IP, "localhost"))
	{
		hints.ai_flags = AI_PASSIVE;
		error = getaddrinfo(NULL, Port, &hints, &serverInfo);
	}
	else
		error = getaddrinfo(IP, Port, &hints, &serverInfo);
	if (error)
	{
		error_show("Problema con el getaddrinfo()!: %s", gai_strerror(error));
		return NULL ;
	}
	return serverInfo;
}

/**	@NAME: connect_to
 * 	@DESC: Intenta establecer conexion con el servidor que deberia estar escuchando. Controla errores, vuelve cuando conecta
 * 	@RETURN: Devuelve EXIT_FAILURE (1) si fallo la conexion, en otro caso devuelve el socket.
 *
 */
int connect_to(char *IP, char* Port)
{
	struct addrinfo* serverInfo = common_setup(IP, Port);
	if (serverInfo == NULL )
	{
		return -1;
	}
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (serverSocket == -1)
	{
		error_show(
				"\n No se pudo disponer de un socket al llamar connect_to, error: %d, man errno o internet!",
				errno);
		return -1;
	}
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1)
	{
		puts("\n");
		error_show(
				"No se pudo conectar con el proceso que hace de servidor, error: %d",
				errno);
		close(serverSocket);
		return -1;
	}
	//char s[INET6_ADDRSTRLEN];
	//inet_ntop(serverInfo->ai_family, get_in_addr((struct sockaddr *)serverInfo->ai_addr),s, sizeof s);
	//printf("Conectando con %s\n", s);
	freeaddrinfo(serverInfo);
	return serverSocket;
}

/**
 * @NAME: common_send
 * @DESC: Intenta mandar un paquete por el socket.
 * Devuelve EXIT_SUCCESS.
 */
void common_send(int socket, t_datosEnviar* paquete, void (*executeIfError)())
{
	char* buffer;
	int totalLength;
	t_dataSize quantitySend;
	int enviando = 1;
	int offset = 0;
	buffer = common_serializer(paquete);
	totalLength = paquete->data_size + packet_header;
	while (enviando)
	{
		quantitySend = send(socket, buffer + offset, totalLength - offset, 0);
		if (quantitySend == -1)
		{
			error_show("Error al enviar, error: %d", errno);
			executeIfError();
		}
		if (quantitySend < totalLength)
		{
			totalLength = totalLength - quantitySend;
			offset = quantitySend;
			//keep sending!!
		}
		else
			enviando = 0;
	}
	free(buffer);
}

/** @NAME common_receive
 *  @DESC: Recibe paquetes a traves de fd, no implementa select ni poll. Si hay error ejecuta executeIfError
 *  @RETURN: Retorna el paquete recibido
 */
t_datosEnviar* common_receive(int fd, void (*executeIfError)())
{
	char *buffer = conitos_malloc(packet_header);
	int buffer_size = recv(fd, buffer, packet_header, MSG_WAITALL);
	if (buffer_size < 0)
	{
		error_show("Error at receiving packets! error: %d", errno);
		executeIfError();
		return NULL ;
	}
	if (buffer_size == 0)
	{
		error_show("Connection closed! at socket %d", fd);
		executeIfError();
		return NULL ;
	}
	t_datosEnviar* dataToReceive = common_unserializeHeader(buffer);
	free(buffer);

	buffer = conitos_malloc(dataToReceive->data_size);
	buffer_size = recv(fd, buffer, dataToReceive->data_size, MSG_WAITALL);
	if (buffer_size < 0)
	{
		error_show("Error at receiving packets! error: %d", errno);
		executeIfError();
		return NULL ;
	}
	if (buffer_size == 0)
	{
		error_show("Connection closed! at socket %d", fd);
		executeIfError();
		return NULL ;
	}
	common_unserializeData(buffer, dataToReceive);
	free(buffer);
	return dataToReceive;
}

int setup_listen(char* IP,char* Port)
{
	struct addrinfo* serverInfo = common_setup(IP, Port);
	if (serverInfo == NULL )
		return -1;
	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return socketEscucha;
}

