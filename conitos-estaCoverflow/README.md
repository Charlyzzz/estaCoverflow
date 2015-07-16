conitos-estaCoverflow - general library
==============================
#### Como instalar?
* Entrar en `cd conitos-estaCoverflow`
* Correr este comando `sudo make install`

#### Como desinstalar?
* Run: `sudo make clean uninstall`

#### Como linkear?
* Click derecho sobre el proyecto -> `Properties`
* Vas `C/C++ Build -> Settings -> Tool Settings -> GCC C++ Linker -> Libraries`
* Donde dice `Libraries (-l)` presiona add y agrega `conitos-estaCoverflow`

### common_sockets

```c
#include <conitos-estaCoverflow/common_sockets.h>

t_datosEnviar* pedirPaquete(void* data,t_codigoOperacion codigoOp, t_dataSize size);
void destruirPaquete(t_datosEnviar*);

struct addrinfo* common_setup(char *IP, char* Port);

int connect_to(char *IP, char* Port);
void common_send(int socket, t_datosEnviar* paquete,void (*executeIfError)());
t_datosEnviar* common_receive(int fd, void (*executeIfError)());
int setup_listen(char* Port);

```

* Nota: destruirPaquete hace un free de los datos dentro del paquete, ojo al utilizarlo, mucha veces ustedes NO quieren hacer free de esos datos. Muchas veces conviene directamente hacer 

```c
free(paquete);
```
Analizen bien esto!
#### Pueden correr los test que se encuentran en la carpeta Test/socketTest
* Se compilan yendo a la carpeta del Test correspondiente y utilizando `cd Debug` y luego `sudo make all`
* Son tests que se corresponden el uno con el otro, por lo tanto tienen que instalar ambos, y luego correrlos en orden de una manera similar a esta:

```bash
cd Test/socketTest/clientTest/Debug
make all

cd Test/socketTest/serverTest/Debug
make all

## y en dos terminales diferentes y en este mismo orden
./serverTest
./clientTest

```
