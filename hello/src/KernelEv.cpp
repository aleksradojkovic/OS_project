/*
 * KernelEv.cpp
 *
 *  Created on: May 19, 2017
 *      Author: OS1
 */

#include "KernelEv.h"
#include "PCB.h"
#include "Schedule.h"
#include "kernel.h"
#include "Thread.h"
#include <dos.h>
#include "IVTEntry.h"
#include <iostream.h>
#include "kersem.h"


//suvisno da se komentarise
void KernelEv::signal() {
	if (sem->val() == -1) sem->signal();
}


//Blokiram se. Po defaultu. Uvek su se iskljucivo samo blokirati ovim waitom, nikada necu proci
//cak i kada event poziva signal, ja cu proveriti da li sam manji od nule (da sam blokiran)
//i ako jesam signal ce biti izvrsen. Ako sam nula, nece se desiti signal.
void KernelEv::wait() {
	if (sem->val() == 0) sem->wait(0);
}

//U ovom delu, treba da setujem enkapsulirane stvari: ulaz (pokazivac na IVTEntry, i
//semafor, pomocu koga cu ovo cudo realizovati. Takodje, ulaz treba biti svestan da treba
//da me signalizira kada mu se desi prekid. zato sam se sam dodao na ulazu. Sekcija nije lokovana
//jer ovo nece niko koristiti. Koristice se iskljucivo event koji pri pozivu ovog konstruktora
//ima lock flag ukljucen.
KernelEv::KernelEv(IVTNo ivtNo) {
	ulaz = IVTEntry::IV[ivtNo];
	ulaz->dodaj(this);
	sem = new KernelSem(0);
}


//Prilikom poziva ovog destruktora, izvrsavacu se u ugnjezdenom destruktoru objekta klase event
//pa su flas I na nuli. Treba da se brisem iz liste IVTEntry-ja, ne treba mi vise nista
//signalizirati.
KernelEv::~KernelEv() {
	// TODO Auto-generated destructor stub
	ulaz->brisi(this);
	delete sem;
}
