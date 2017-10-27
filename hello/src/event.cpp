/*
 * event.cpp
 *
 *  Created on: May 13, 2017
 *      Author: OS1
 */

#include "event.h"
#include "kernel.h"
#include "KernelEv.h"

extern volatile PCB* running;


//jedino sto valja promeniti jeste myImpl. Zelim kernelovu implementaciju za ovo.
Event::Event (IVTNo ivtNo){
	lock();
	myImpl = new KernelEv(ivtNo);
	unlock();
	//Ovde treba obezbediti cuvanje stare prekidne rutine
}


//Ovo se desava jednom. samo. lok sekcija jer pozivam delete i signal. Kernel ovo radi
Event::~Event (){
	lock();
	myImpl->signal();
	delete myImpl;
	unlock();
	//lockovana sekcije jer brisem neke stvari.
}


//Kernel ovo radi.
void Event::wait(){
	lock();
	myImpl->wait();
	unlock();

}

//Kernel ovo radi.
void Event::signal(){
	lock();
	myImpl->signal();
	unlock();
}


//Sekcije su lokovane 2 puta. Nema problema sa tim, nece se nista lose desiti. razlika izmedju
//tih lockova je par instrukcija, a to je reda 10^-8s. Razlika nije vidljiva
