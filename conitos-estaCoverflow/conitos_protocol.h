/*
 * conitos_protocol.h
 *
 *  Created on: Jun 7, 2014
 *      Author: root
 */

#ifndef CONITOS_PROTOCOL_H_
#define CONITOS_PROTOCOL_H_

//=========================================
/*
 *
 *  GENERIC PROTOCOL w/ALL
 *
 */
//========================================

#define END_IT_ALL 0
#define HANDSHAKE 1
#define ID_PROCESO_PROGRAMA 0
#define ID_UMV 1
#define ID_KERNEL 2
#define ID_CPU 3
#define NO_HAY_ERROR_POSIBLE 1000

//=========================================
/*
 *
 *  GENERIC PROTOCOL w/PLP
 *
 */
//========================================

#define PLP_PROGRAMA_CONFIRMAR_CONECCION 1
#define PLP_PROGRAMA_NO_SE_PUDO_CREAR 2
#define PLP_PROGRAMA_ENCOLADO 3



//=========================================
/*
 *
 *  CPU PROTOCOL w/PCP
 *
 */
//=========================================

#define CPU_PCP_FIN_QUANTUM 1
#define CPU_PCP_PEDIR_VARIABLE_COMPARTIDA 2
#define CPU_PCP_ASIGNAR_VARIABLE_COMPARTIDA 3
#define CPU_PCP_FIN_PROCESO 4
#define CPU_PCP_IMPRIMIR_VARIABLE 5
#define CPU_PCP_IMPRIMIR_TEXTO 6
#define CPU_PCP_ENTRADA_SALIDA 7
#define CPU_PCP_SIGNAL 8
#define CPU_PCP_WAIT 9
#define CPU_PCP_EXCEPTION 10
#define CPU_PCP_PROCESS_WAITING 11
#define CPU_PCP_DISCONNECTION 12


//=========================================
/*
 *
 *  KERNEL PROTOCOL w/PROGRAMA
 *
 */
//=========================================

#define PROGRAMA_PAQUETE_PARA_IMPRIMIR 1
#define PROGRAMA_CERRAR 2
#define ENVIAR_SCRIPT 3

//=========================================
/*
 *
 *  PLP PROTOCOL w/UMV
 *
 */
//=========================================

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

//=========================================
/*
 *
 *  UMV PROTOCOL and interface
 *
 */
//=========================================

#define UMV_SEGMENTATION_FAULT 1
#define UMV_LEER_BYTES 2
#define UMV_ESCRIBIR_BYTES 3

//=========================================
/*
 *
 *  PCP PROTOCOL w/CPU
 *
 */
//=========================================
#define PCP_CPU_DEFAULT 2
#define PCP_CPU_WAIT_OK 1
#define PCP_CPU_ERROR_EN_SYSCALL 3
#define PCP_CPU_WAIT_NO_OK 4
#define PCP_CPU_PROGRAM_TO_EXECUTE 6
#define PCP_CPU_OK 7
#define PCP_CPU_PROGRAM_TO_EXECUTE 26

//=========================================
/*
 *
 *  CPU PROTOCOL w/UMV
 *
 */
//=========================================
#define DESCONECTARSE 123
#define DESCONECTARSE_POR_QUANTUM 132
#define TERMINO_INSTANTE_QUANTUM 65



#endif /* CONITOS_PROTOCOL_H_ */
