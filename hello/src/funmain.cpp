#include <stdio.h>
#include "thread.h"
#include <iostream.h>
#include "PCB.h"
#include "kernel.h"

int userMain(int argc, char* argv[]);

extern volatile PCB* running;

class Exec: public Thread{
public:
	Exec(int argc, char** argv){ eargc = argc; eargv = argv;}
	~Exec() { waitToComplete(); }
	int get_ret() const { return n; }
protected:
	void run() {
		n = userMain(eargc, eargv);
	}
private:
	int n, eargc;
	char** eargv;
};

int main(int argc, char* argv[]){
	inic();

	lock();
	PCB* p = new PCB(4096, 2, 0);
	unlock();

	running = p;
	p->state = RUNNING;

	//ova sekcija mora da bude lokovana jer tajmer je vec poceo, tick moze biti ovde.. ovo
	//ce doprineti sigurnosti
	lock();
	Exec* e = new Exec(argc, argv);
	e->start();
	unlock();

	e->waitToComplete();

	int rt = e->get_ret();

	delete e;

	restore();
	return rt;
}
