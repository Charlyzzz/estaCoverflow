#include <commons/config.h>
#include <stdio.h>
#include <conitos-estaCoverflow/common_sockets.h>
#include <conitos-estaCoverflow/conitos_protocol.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

int main(int argc, char *argv[]) {

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);
	char * direccionConfig;
	char * ipKernelPLP;
	char * puertoKernelPLP;
	char * direccionScript;
	char * programa;

	t_config * archivoConfiguracion;
	direccionConfig = getenv("ANSISOPCONFIG");
	//direccionConfig = argv[1];
	archivoConfiguracion = config_create(direccionConfig);
	ipKernelPLP = config_get_string_value(archivoConfiguracion, "IP");
	puertoKernelPLP = config_get_string_value(archivoConfiguracion, "Puerto");
	direccionScript = argv[1];

	int tamanio, descriptorArch;
	struct stat infoArchivo;

	descriptorArch = open(direccionScript, O_RDONLY); //abre el archivo
	fstat(descriptorArch, &infoArchivo); //averiguamos su informacion
	tamanio = infoArchivo.st_size;
	programa = malloc(tamanio);

	read(descriptorArch, programa, tamanio); //guardo el script en programa
	close(descriptorArch);

	int serverSocket;

	serverSocket = connect_to(ipKernelPLP, puertoKernelPLP);

	if (serverSocket == -1) {

		puts("\nPrograma finalizado ");
		puts("\n\n");
		close(serverSocket);
		return EXIT_FAILURE;

	} else {

		t_datosEnviar * paquete = pedirPaquete("H", HANDSHAKE, 2);

		common_send(serverSocket, paquete, NULL );

		destruirPaquete(paquete);

		paquete = common_receive(serverSocket, NULL );

		if (paquete->codigo_Operacion != PLP_PROGRAMA_CONFIRMAR_CONECCION) {
			puts("Handshake rechazado, cerrando el programa");
			close(serverSocket);
			return EXIT_FAILURE;
		}

		paquete = pedirPaquete(programa, ENVIAR_SCRIPT, strlen(programa) + 1);

		common_send(serverSocket, paquete, NULL );
		destruirPaquete(paquete);
		paquete = common_receive(serverSocket, NULL );

		if (paquete->codigo_Operacion == PLP_PROGRAMA_NO_SE_PUDO_CREAR) {
			puts(
					"La umv no pudo alocar los segmentos pedidos, finalizando programa");
			close(serverSocket);
			return EXIT_FAILURE;
		}
		destruirPaquete(paquete);

		puts("A la escucha");

		while (1) {

			paquete = common_receive(serverSocket, NULL );
			if (paquete->codigo_Operacion == PROGRAMA_PAQUETE_PARA_IMPRIMIR) {
				puts(paquete->data);
				destruirPaquete(paquete);
			} else {
				if (paquete->codigo_Operacion == PROGRAMA_CERRAR) {
					puts("Cerrando programa; Gracias, vuelva pronto");
					destruirPaquete(paquete);
					close(serverSocket);
					return EXIT_SUCCESS;
				}
			}

		}
	}
}
