/*
 * Thread.cpp
 *
 *  Created on: May 5, 2017
 *      Author: OS1
 */

#include "Thread.h"
#include "kernel.h"
#include "schedule.h"
#include <iostream.h>
#include "PCB.h"
#include "Semaphor.h"
#include "Threads.h"

extern volatile int zahtevana_promena_konteksta;
extern volatile PCB* running;

short GlobalMask[16];
//0 - Masked, 1 - Non Masked
short SignalBlock[16];
//0 - Blocked, 1 - NonBlocked


Threads* thead = 0;
Threads* ttail = 0;

Thread::Thread(StackSize stackSize, Time timeSlice) {
	softlock();
	lock();
	myPCB = new PCB(stackSize, timeSlice, (PCB*)running);
	lock();
	myPCB->MyThread = this;
	for(int i=0; i<16;i++){
			myPCB->LocSignalBlock[i] = running->LocSignalBlock[i];
			myPCB->LocalMask[i] = running->LocalMask[i];
			myPCB->signali[i] = running->signali[i];
			//Pretpostavka: zahteve necu kopirati. Korisnik verovatno ne zeli nezeljene efetke.
	}

	Threads* temp = new Threads(this);
	if (!thead) thead = temp;
	else ttail->next = temp;
	ttail = temp;
	unlock();
	softunlock();
}


void Thread::start(){
	lock();
	myPCB->create_stack();
	Scheduler::put(myPCB);
	unlock();
}

ID Thread::getRunningId(){
	if (running->MyThread) return running->id;
	else return 0;
}

Thread* Thread::getThreadById(ID idd){
	Threads* tek = thead;
	while(tek && tek->t->getId() != idd)
		tek=tek->next;
	return tek->t;
}

Thread::~Thread() {
	waitToComplete();
	lock();
	Threads* preth = 0;
	Threads* tek;
	while(tek){
		if (tek->t == this) {
			Threads* toDelete = tek;
			tek=tek->next;
			if (preth != 0) {
				preth->next = tek;
				if (tek == 0) ttail = tek;
			} else thead=tek;
			if (thead == 0) ttail = 0;
			delete toDelete;
			break;
		} else {
			preth = tek;
			tek=tek->next;
		}
	}
	delete myPCB;
	unlock();
}

ID Thread::getId(){
	if (myPCB) return myPCB->id;
	else return 0;
}

void Thread::waitToComplete(){
	if (this->myPCB->isFinished) return;
	myPCB->Waiting->wait(0);
}

void dispatch(){ // sinhrona promena konteksta
	lock();
	zahtevana_promena_konteksta = 1;
	timer();
	unlock();
}

void Thread::registerHandler(SignalId signal, SignalHandler handler){
	myPCB->signali[signal] = handler;
}

SignalHandler Thread::getHandler(SignalId signal){
	return myPCB->signali[signal];
}

void Thread::maskSignal(SignalId signal){
	myPCB->LocalMask[signal] = 0;
}
void Thread::maskSignalGlobally(SignalId signal){
	GlobalMask[signal] = 0;
}
void Thread::unmaskSignal(SignalId signal){
	myPCB->LocalMask[signal] = 1;
}
void Thread::unmaskSignalGlobally(SignalId signal){
	GlobalMask[signal] = 1;
}

void Thread::blockSignal(SignalId signal){
	myPCB->LocSignalBlock[signal] = 0;
}
void Thread::blockSignalGlobally(SignalId signal){
	SignalBlock[signal] = 0;
}

void Thread::unblockSignal(SignalId signal){
	myPCB->LocSignalBlock[signal] = 1;
}

void Thread::unblockSignalGlobally(SignalId signal){
	SignalBlock[signal] = 1;
}

void Thread::signal(SignalId sig) {
	if (sig == 1 || sig == 2 || myPCB->isFinished) return;
	myPCB->PerformSignal(sig);
}

void Thread::pause(){
	lock();
	running->state = PAUSED;
	dispatch();
	unlock();
}
