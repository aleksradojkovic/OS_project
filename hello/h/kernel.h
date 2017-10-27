/*
 * kernel.h
 *
 *  Created on: May 4, 2017
 *      Author: OS1
 */

#ifndef KERNEL_H_
#define KERNEL_H_

class kersemQ;

void inic();
void restore();
void interrupt timer();
void lock();
void unlock();
void softlock();
void softunlock();
void forcekill();

#endif /* KERNEL_H_ */
