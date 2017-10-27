/*
 * IVTEntry.cpp
 *
 *  Created on: May 21, 2017
 *      Author: OS1
 */

#include "IVTEntry.h"
#include <dos.h>
#include "KernelEv.h"
#include "kernel.h"
#include <iostream.h>
#include "kersem.h"

//char ima 1B, dovoljno za smestanje 0-255
typedef unsigned char IVTNo;

//ovako izgleda definicija interrupt-a
typedef void interrupt (*pInterrupt)(...);

//Inicijalizacija elementa u listi.
EvList::EvList(KernelEv* eev, EvList* nxt){
	ev = eev; next = nxt;
}

//pozivanje stare prekidne rutine, ako treba. Realizovana kao void, treba o tome
//eventualno voditi racuna
void IVTEntry::callOldRoutine(){
	old();
}

IVTEntry::IVTEntry(IVTNo No, pInterrupt fp) {
	lock();
	ulaz = No; //identifikacija ulaza.
	head = 0; tail = 0; //jer na pocetku, nemam uopste niceg sto ceka na ovaj dogadjaj :)
	old = getvect(No); //pamtim staru prekidnu rutinu, jer zelim da je pozivam
	setvect(No, fp); //postavljam novu prekidnu rutinu
	IV[No] = this; //dodajem u staticki niz pokazivaca da imam pristup ovim predjasnjim inf.
	//sacuvana stara prekidna rutina, postavljena nova.
	unlock();
	//sekcija ne reaguje na prekide jer: ovo se radi u makrou prepareentry, dok jos i ne postoji
	//nijedna nit. Moze biti nelokovana ali radi bezbednosti lokovacemo. Ne utice na performanse.
}

//signaliziram svakom semaforu da sam se desio. Treba voditi racuna koja je vrednost semafora.
//primetimo da semafor ne moze biti pozitivan. potrebno je izvrsiti na objektu Event-a metodu
//void Event::wait() i nit se blokira. Pozivom signala, ona se odblokira, a to znaci da nit na
//na wait-u se obavezno blokira. Znaci da vrednost semafora moze biti -1 i 0 (1 blokirana ili
//nijedna. Potrebno je signalizirati samo one semafore koji cekaju na signal, tj
//int kersem::value < 0. U suprotnom, ne cekaju na moj dogadjaj.
//sekcija je lokovana jer se koriste semafori vec, koji su lokovani
void IVTEntry::signal(){
	lock();
	for(EvList* tek = head; tek!=0; tek=tek->next){
		tek->ev->signal();
	}
	unlock();
}

//ova funkcija je namenjena da dodam u listu semafore koji cekaju prekid, i signaliziranjem
//cu im reci da se desio prekid. Ova lista ne moze da se suzi, samo moze da se u nju dodaje
//Posto dodaj moze da se radi u jednoj niti, u drugoj niti signal, zelim ovu sekciju lokovanu.
//razlozi: vrlo retko se poziva, malo, a moze izazvati probleme. ovde ima 4 naredbe, tako da je
//maksimalno vreme izvrsavanja reda 10^-8s sto ne moze uticati na performanse.
//Ovo se poziva u vecini slucajeva < 10 puta.
void IVTEntry::dodaj(KernelEv* eev){
	EvList* temp = new EvList(eev);
	if (head == 0) head = temp;
	else tail->next = temp;
	tail = temp;
}

//U ovom delu, takodje pozeljno da bude lokovan jer se menjaju prekidne rutine, i desava se jednom
//i treba sada izbeci bilo kakve prekide. Postavljamo staru prekidnu rutinu, i brisemo listu
//jer ne zelimo da imamo memoriju iza nas. Resetujemo pokazivac u globalnoj listi IVTEntry-ja.
//stanje je kao i pre, kao da se nista nije izdesavalo
IVTEntry::~IVTEntry() {
	lock();
	if (old) old();
	setvect(ulaz, old);
	//brisanje liste
	IV[ulaz] = 0;
	while(head){
		EvList* stari = head;
		head=head->next;
		delete stari;
	}
	unlock();
}

//Zelim da se osiguram, zelim nule na pocetku
IVTEntry* IVTEntry::IV[256] = {0};


void IVTEntry::brisi(KernelEv* pok){
	EvList* tek = head;
	EvList* preth = 0;
	while(tek && tek->ev != pok) { //sve dok tek postoji i dok njegov objekat je razlicit od pok
		preth = tek;
		tek=tek->next;
	}
	//za svaki slucaj radicu proveru da li tek nije slucajno NULL. Teoretski ne bi trebalo
	//jer sam siguran da ova metoda se poziva samo iz objekta koji su vec ovde dodati.
	//to je jedna instrukcija i traje kratko, brzo i izvrsava se ~1 put.
	if (tek){
		EvList* toDelete = tek;
		tek=tek->next;
		if (preth == 0) {
			head = tek;
			if (head == 0) tail = 0;
		} else {
			preth->next = tek;
			if (tek == 0) tail = preth;
		}
		delete toDelete;
	}
}
