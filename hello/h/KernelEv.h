/*
 * KernelEv.h
 *
 *  Created on: May 19, 2017
 *      Author: OS1
 */

#ifndef KERNELEV_H_
#define KERNELEV_H_

class PCB;
class IVTEntry;
class KernelSem;
//Ove deklaracije su svesne da ove tri klase postoje. Ne znaju sta su, ali
//i nije bitno, bitno je samo evidencija o njihovom postojanju. cpp ce ih includovati

typedef unsigned char IVTNo;

class KernelEv {
public:
	KernelEv(IVTNo ivtNo);
	virtual ~KernelEv();
	void signal();
	void wait();
protected:
	friend class Event;
	friend class IVTEntry;
private:
	KernelSem* sem;
	IVTEntry* ulaz;
};

#endif /* KERNELEV_H_ */
