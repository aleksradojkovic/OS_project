/*
 * PCB.cpp
 *
 *  Created on: May 4, 2017
 *      Author: OS1
 */

#include "PCB.h"
#include <dos.h>
#include "kernel.h"
#include "Thread.h"
#include "Semaphor.h"
#include <iostream.h>
#include <Schedule.h>

extern volatile PCB* running;
extern short SignalBlock[16];
extern short GlobalMask[16];

PerSig::PerSig(SignalId idd, PerSig* nxt):id(idd), next(nxt) {
}

PerSig::~PerSig() {

}

void PCB::PerformSignal(SignalId id) volatile{
	if (!signali[id]) return;
	if (!GlobalMask[id] || !LocalMask[id]) return;
	if (state == PAUSED) { Scheduler::put((PCB*)this); state = READY; }
	//ne zelim da registrujem signale koji ne postoje, odnosno nakon sto je zatrazen ForceQuit
	//sigurno znam da je ForceQuit poslednji
	PerSig* temp = new PerSig(id,0);
	if (SignalBlock[id] && LocSignalBlock[id]) temp->exec = 1;
	else temp->exec = 0;

	if (sighead) sigtail->next = temp;
	else sighead = temp;
	sigtail=temp;
}

void PCB::wrapper(){
	running->MyThread->run();
	if (running->myFather->MyThread != 0) running->myFather->PerformSignal(1);
	//ovo jer ako je usermain kreirao, on nema svog oca.
	if (running->MyThread) running->PerformSignal(2);
	//ako je running = userMain.. on nema svoju nit. Ne mogu samim tim ni da pozovem taj signal
	//a ni korisnik ne bi trebalo da ima pristupa toj niti
	dispatch();
	//ovde radim dispatch da bi se ti signali videli. Nakon ovoga sam siguran da ako su
	//gornji uslovi ispostovani, da ce se ove niti izvrsiti
	lock();
	running->isFinished = 1;
	int a = -running->Waiting->val(); //ovoliko jos blokiranih niti
	for(int i=0;i<a;i++){
		running->Waiting->signal();
		lock(); //delocked je ovde jer je ovaj waiting semafor.
	}
	dispatch();
	unlock();
}



void PCB::create_stack(void (*fp)()){
		stack = new unsigned[size];
		stack[size-1] = 0x200;
		stack[size-2] = FP_SEG(fp);
		stack[size-3] = FP_OFF(fp);
		sp = FP_OFF(stack+size-12);
		ss = FP_SEG(stack+size-12);
}

PCB::PCB(unsigned sz, int TSLC, PCB* Father) {
	Waiting = new Semaphore(0);
	lock(); //ovo gore bi trebalo da bude kernelSem
	for(int i=0;i<16;i++) {
		signali[i] = 0;
		LocalMask[i] = 1;
		LocSignalBlock[i] = 1;
	}
	signali[0] = forcekill;
	sighead = sigtail = 0;
	size = sz;
	id = ++uID;
	isFinished = 0; //0 - Nije zavrsio, 1 zavrsio ispravno, 2 zavrsio nasilno
	semret = 1;
	MyThread = 0;
	quant = TSLC;
	state = READY;
	if (TSLC != 0) inftimeslice = 0;
	else inftimeslice = 1;
	myFather = Father;
}

PCB::~PCB(){
	lock();
	delete [] stack;
	delete Waiting;
	unlock();
}

ID PCB::uID = -2;
