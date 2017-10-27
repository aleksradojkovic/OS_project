/*
 * kernel.cpp
 *
 *  Created on: May 4, 2017
 *      Author: OS1
 */

#include "kernel.h"
#include <dos.h>
#include <iostream.h>
#include "schedule.h"
#include "PCB.h"
#include "Thread.h"
#include "kersem.h"
#include <stdlib.h>
#include "Semaphor.h"
#include "Threads.h"

class kersemQ;

extern short SignalBlock[16];
extern short GlobalMask[16];
extern Threads* thead;
extern Threads* ttail;


volatile int locked = 0;

PCB* idle;
static volatile PCB* ForceQuitPCB = 0;

volatile int zahtevana_promena_konteksta = 0;
volatile int brojac = 2;
volatile PCB* running;
volatile short lockflag = 0;

unsigned tsp, tss;
volatile PerSig* tek;
volatile PerSig* preth;

void tick();


void lock() {
 	asm {
 		cli
 	}
}

void unlock() {
	asm sti
}

void softlock() {
	lockflag = 1;
}

void softunlock() {
	lockflag = 0;
}


void interrupt timer(){	// prekidna rutina
	if (!zahtevana_promena_konteksta) {
		tick();
		KernelSem::decrement();
	}
	if (!lockflag){
		if (!zahtevana_promena_konteksta) brojac--;
		if ((brojac == 0 && !running->inftimeslice) || zahtevana_promena_konteksta) {
			asm {
				mov tsp, sp
				mov tss, ss
			}
			running->sp = tsp;
			running->ss = tss;
			if (!running->isFinished && running->state != PAUSED && running->state != BLOCKED && running->state != IDLE)
			{
				running->state = READY;
				Scheduler::put((PCB *)running);
			}

			running = Scheduler::get();
			if (running == 0) {
				running = idle;
			} else
			running->state = RUNNING;

			tsp = running->sp;
			tss = running->ss;

			asm {
				mov sp, tsp
				mov ss, tss
			}

			tek = running->sighead;
			preth = 0;

			while(tek) {
				if (tek->exec != 0 || (SignalBlock[tek->id] && running->LocSignalBlock[tek->id])) {
					//ako je tek->exec 1 (Tada je mogao da se izvrsi) || moze sada da se izvrsi trenutni signal
					if (tek->id == 0){
						ForceQuitPCB = running;

						softlock();
						running->signali[0]();
						softunlock();

						running = idle;

						tsp = running->sp;
						tss = running->ss;

						asm {
							mov sp, tsp
							mov ss, tss
						}

						delete ForceQuitPCB->stack;
						ForceQuitPCB->stack = 0;

						break;
					}
					if (running->signali[tek->id]) { // i ako postoji signal (Ovo ovde ne mora, ali zelim da budem siguran i otporan na greske. Mozda je u medjuvremenu
						//nesto promenjeno, stavljen signal na nulu ili tako nesto. Ne zelim da rizikujem.
						softlock();  //ovo setuje lockflag = 0
						asm cli // ovo dozvoljava prekide
						//dispatch(); <- PERFORMANSE
						running->signali[tek->id](); //ovo izvrsava funkciju
						asm sti // ovo zabranjuje prekide
						softunlock(); //ovo setuje lockflag = 1;
					} //ovde sam izvrsi ovu funkciju ako je mogla da se izvrsi
					//sada je brisem iz liste.
					tek = tek->next;
					if (preth == 0) { //brisem glavu
						running->sighead = tek;
						if (running->sighead == 0) running->sigtail = 0; //ako sam obrisao glavu i ona je bila ujedno i rep, rep je nula takodje
					} else {
						preth->next = tek;
						if (tek == 0) running->sigtail = preth;
					}
				} else {
					//blokirana je. Idem dalje.
					preth = tek;
					tek=tek->next;
				}
				//zavrsena obrada svih signala.
			}
			brojac = running->quant;
		}
	}
	if(!zahtevana_promena_konteksta) asm int 60h;
	zahtevana_promena_konteksta = 0;
}


unsigned oldTimerOFF, oldTimerSEG;


void idl() {
	int i;
	loop: i=1;
	for(int k=0;k<10000;k++);
	i=i+1;
	asm { jmp loop }
}

void inic(){
	lock();
	idle = new PCB(4096,2, 0);
	unlock();
	idle->create_stack(idl);
	idle->state = IDLE;

	for(int i=0;i<16;i++){
		GlobalMask[i] = 1;
		SignalBlock[i] = 1;
	}
	asm{
		cli
		push es
		push ax

		mov ax,0
		mov es,ax

		mov ax, word ptr es:0022h
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax

		mov word ptr es:0022h, seg timer
		mov word ptr es:0020h, offset timer

		mov ax, oldTimerSEG
		mov word ptr es:0182h, ax
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}


void forcekill(){
	ForceQuitPCB->isFinished = 1;
	delete ForceQuitPCB->Waiting;
	ForceQuitPCB->Waiting = 0;
	Threads* preth = 0;
	Threads* tek;
	while(tek){
		if (tek->t == ForceQuitPCB->MyThread) {
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
}

void restore(){
	asm {
		cli
	}
		delete idle;
	asm {
		push es
		push ax

		mov ax,0
		mov es,ax


		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}
