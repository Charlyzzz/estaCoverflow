/*
 * kernelPosta.h
 *
 *  Created on: Jul 17, 2014
 *      Author: root
 */

#ifndef KERNELPOSTA_H_
#define KERNELPOSTA_H_

void fillSemaphoreDictionary();
void fillSharedVariableDictionary();
t_varCompartida *createSharedVariable(char*, uint32_t);
void setupSemaphores();
void initializateCollections();
void readConfig(char *);
void shutdownKernel();
void cerrarSemaforos();
void startCommunicationWithUMV();
void fillDictionaries();
void fillCPUCommandDictionary();

#endif /* KERNELPOSTA_H_ */
