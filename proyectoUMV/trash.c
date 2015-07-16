/*estructuraDeSegmento worstFit(estructuraDeSegmento datosSegmento)
 {
 list_sort(listaDeSegmentos,(void*) auxiliarParaOrdenarListaDeSegmentos);
 estructuraDeSegmento* nodo;
 estructuraDeSegmento* nodoSiguiente;
 int i=0;
 int maxTamano=0;
 int tamanoLista=list_size(listaDeSegmentos);
 if (tamanoLista==0) //primer segmento de la umv
 {
 datosSegmento.direccionReal = configuracion.inicioMalloc;
 datosSegmento.direccionVirtual=1;
 }
 if (tamanoLista>0)
 {
 void* primerNodoPosicion;
 int posicionDeGuardado=0;
 estructuraDeSegmento* primerNodo=list_get(listaDeSegmentos,0);
 primerNodoPosicion= primerNodo->direccionReal;
 int tamanoEntre1erNodoEInicio=primerNodoPosicion-1;
 if (tamanoEntre1erNodoEInicio>=datosSegmento.tamanoSegmento)
 {
 maxTamano=tamanoEntre1erNodoEInicio;
 posicionDeGuardado=1;
 }
 for (i=0;i+1<tamanoLista;i++) //recorro la lista hasta una posicion menos porque si llego al ultimo, al obtener el siguiente me va a dar error
 {
 nodo=list_get(listaDeSegmentos,i);
 nodoSiguiente=list_get(listaDeSegmentos,i+1);
 int tamanoEntreSegmentos= nodoSiguiente->direccionReal - (nodo->direccionReal+nodo->tamanoSegmento);
 if (tamanoEntreSegmentos>=maxTamano && tamanoEntreSegmentos>=datosSegmento.tamanoSegmento)
 {
 maxTamano=tamanoEntreSegmentos;
 posicionDeGuardado=nodo->direccionReal+nodo->tamanoSegmento;
 }
 }
 nodo=list_get(listaDeSegmentos,tamanoLista-1);//ultimo segmento
 int tamanoEntreSegmentoYFinal=(configuracion.inicioMalloc+configuracion.cantidadBytesMalloc)-(nodo->direccionReal+nodo->tamanoSegmento)+1;
 if (datosSegmento.tamanoSegmento<=tamanoEntreSegmentoYFinal && tamanoEntreSegmentoYFinal>=maxTamano)
 {
 maxTamano=tamanoEntreSegmentoYFinal;
 posicionDeGuardado=nodo->direccionReal+nodo->tamanoSegmento;
 }
 // t odo datosSegmento.direccionVirtual=posicionDeGuardado;
 datosSegmento.direccionReal=configuracion.inicioMalloc+posicionDeGuardado;
 }
 list_add(listaDeSegmentos, segmento_create(datosSegmento));
 return datosSegmento;
 }*/

/* PARECIERAN NO SERVIR PARA NADA....
 int hayEspacioMemoria() {			----- SE REEMPLAZO POR int tamanoUsadoDelMalloc()
 int i;
 estructuraDeSegmento* nodoActual;
 int espacioMemoriaDisponible = 0;
 int tamanoLista = list_size(listaDeSegmentos);
 for (i = 0; i < tamanoLista; i++) {
 nodoActual = list_get(listaDeSegmentos, i);
 espacioMemoriaDisponible = espacioMemoriaDisponible
 + nodoActual->tamanoSegmento;
 }
 return espacioMemoriaDisponible;
 }
 ------- ESTAS DOS NO SE PARA QUE LAS CREE...
 estructuraDeSegmento *segmentoSiguienteEliminandoAnteriores(int posicion) {
 estructuraDeSegmento *nodo = list_get(listaDeSegmentos, posicion);

 while (nodo->processID != -1) {
 list_remove(listaDeSegmentos, posicion);
 if (nodo) {
 nodo = list_get(listaDeSegmentos, posicion + 1);
 }
 }
 return nodo;

 }

 estructuraDeSegmento *segmentoUltimoNodo() {
 int tamanoLista = list_size(listaDeSegmentos), i = 1;
 estructuraDeSegmento *nodo = list_get(listaDeSegmentos, tamanoLista - 1);

 while (nodo->processID != -1) {
 if (nodo) {
 list_remove(listaDeSegmentos, tamanoLista - i);
 }
 i++;
 nodo = list_get(listaDeSegmentos, tamanoLista - i);
 }

 return nodo;
 }

 */
