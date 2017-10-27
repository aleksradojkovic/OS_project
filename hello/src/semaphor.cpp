/*
 * semaphor.cpp
 *
 *  Created on: May 6, 2017
 *      Author: OS1
 */

#include "semaphor.h"
#include "kernel.h"
#include "kersem.h"
#include "PCB.h"

extern volatile PCB* running;

Semaphore::Semaphore(int init) {
	lock();
	myImpl = new KernelSem(init);
	unlock();
	//sekcija lokovana iz razloga: Lokuje se jer pozivam new. Moze se desiti
	//da se prebacim na drugu nit i ona pozove new, a da se prethodni new nije izvrsio.
	//lock je jedino moguce resenje
}

Semaphore::~Semaphore() {
	lock();
	myImpl->signalAll();
	delete myImpl;
	unlock();
	//Ako u toku signalAll dodje dispatch, smatramo da smo unistili semafor, a moze biti jos uvek
	//niti zablokiranih na njemu. Zato je lockovan destrkutor.
}

int Semaphore::wait(Time MaxTimeToWait){
	lock();
	myImpl->wait(MaxTimeToWait);
	unlock();
	//Lockovana sekcija jer: ako nije, moze se desiti dispatch, a da samo jedna nit moze proci.
	//druga nit moze pozvati takodje wait, i onda ce i ona proci, prosle dve niti
	//a trebalo bi samo jedna da prodje. zato zelim ovo atomicno, da se zablokira odmah.
	return running->semret;
}

void Semaphore::signal(){
	lock();
	myImpl->signal();
	unlock();
	//Kao i gore, analogno.
}

int Semaphore::val() const{
	return myImpl->val();
}

