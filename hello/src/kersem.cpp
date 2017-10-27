/*
 * kernelsem.cpp
 *
 *  Created on: May 6, 2017
 *      Author: OS1
 */

#include "kersem.h"
#include "PCB.h"
#include "kernel.h"
#include "schedule.h"
#include "Thread.h"
#include <iostream.h>

extern volatile PCB* running;

struct Elemt{
	PCB* p;
	Elemt* next;
	Elemt(PCB* pp, Elemt* nxt = 0){
		p = pp; next = nxt;
	}
};

class kersemQ{
public:
	KernelSem* ks;
	kersemQ* next;
	kersemQ(KernelSem* k, kersemQ* nxt = 0) {
		ks = k; next = nxt;
	}
};

class semQ{
public:
	semQ(KernelSem* ks);
	void put(PCB* pcb);
	PCB* get() volatile;
	void dec() volatile;
protected:
private:
	KernelSem* myKersem;
	Elemt* head;
	Elemt* tail;
};

semQ::semQ(KernelSem* ks){
	myKersem = ks;
	head = 0; tail = 0;
}

void semQ::dec() volatile{
	Elemt* preth = 0;
	Elemt* tek = head;
	while(tek != 0){
		//ovde vadim
		if(tek->p->unblockWait-- == 1) {// da li je dekrementirana vrednost stigla do 0
				//oslobadjam nit, stavljam je u ready
				tek->p->state = READY;
				//cupam iz reda na semaforu, jer ako je red prazan, znaci da value treba biti 0
				myKersem->value++;
				myKersem->noefsignal++;
				//zelim da kazem svim nitima koje cekaju na moje izvrsavanje, da ako sam
				//semafor na kome sam istekao dobija jednu nit slobodnu za izvrsavanje
				tek->p->semret = 0;
				Scheduler::put(tek->p);
				//VADJENJE IZ REDA
				Elemt* toDelete = tek;
				tek = tek->next;
				if (preth != 0) {
						preth->next = tek;
						if (preth->next == 0) tail = preth;
					}
				else head=tek;
				if (!head) tail = 0;
				delete toDelete;
				//a da je maknem iz reda? hehe
		} else {
			//ovde sve ostaje u redu
			preth = tek;
			tek=tek->next;
		}
	}
}

void semQ::put(PCB* pcb){
	Elemt* newly = new Elemt(pcb);
	if (head == NULL) head = newly;
	else tail->next = newly;
	tail=newly;
}

PCB* semQ::get() volatile{
	if (!head) return 0;
	PCB* temp = head->p;
	Elemt* old = head;
	head=head->next;
	delete old;
	if (!head) tail = 0;
	return temp;
}

KernelSem::KernelSem(int init) {
	queue = new semQ(this);
	value = init;
	noefsignal = 0;
	kersemQ* temp = new kersemQ(this);
	if (!semhead) semhead = temp;
	else semtail->next = temp;
	semtail = temp;
}

KernelSem::~KernelSem() {
	signalAll();
	delete queue;
	queue = 0;
	kersemQ* tek = semhead;
	kersemQ* preth = 0;
	while(tek && tek->ks != this) {
		preth = tek; tek=tek->next;
	}
	if (!preth) semhead=tek->next;
	else preth->next = tek->next;
	if (!tek->next) semtail = preth;
}

void KernelSem::wait(Time maxTimeToWait){
	if (value--<=0) {
		running->state = BLOCKED;
		running->unblockWait = maxTimeToWait;
		queue->put((PCB *)running);
		dispatch();
	} else {
		running->semret = 1;
	}
}

void KernelSem::signal(){
	if (value++ < 0) {
		PCB* temp = queue->get();
		if (temp == 0) return;
		temp->state = READY;
		temp->semret = 1;
		Scheduler::put(temp);
	}
}

void KernelSem::signalAll(){
	int p = -value;
	for(int i=0;i<p;i++)
		signal();
}

int KernelSem::val(){
	return value;
}


kersemQ* KernelSem::semhead = 0;
kersemQ* KernelSem::semtail = 0;

void KernelSem::decrement(){
	for(kersemQ* tek = semhead; tek!=0; tek=tek->next){
		tek->ks->queue->dec();
	}
}
