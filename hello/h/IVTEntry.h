/*
 * IVTEntry.h
 *
 *  Created on: May 21, 2017
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_

typedef unsigned char IVTNo;
typedef void interrupt (*pInterrupt)(...);

class KernelEv;

class EvList {
public:
	KernelEv* ev;
	EvList* next;
	EvList(KernelEv* eev, EvList* nxt = 0);
};

class IVTEntry {
public:
	IVTEntry(IVTNo No, pInterrupt fp);
	virtual ~IVTEntry();
	void signal();
	void callOldRoutine();
	pInterrupt old;
	static IVTEntry* IV[256];
protected:
	friend class KernelEv;
private:
	void dodaj(KernelEv* ev);
	void brisi(KernelEv* ev);
	EvList* head;
	EvList* tail;
	IVTNo ulaz;
};

#endif /* IVTENTRY_H_ */
