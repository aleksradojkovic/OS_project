/*
 * PCB.h
 *
 *  Created on: May 4, 2017
 *      Author: OS1
 */

#ifndef pcb_H_
#define pcb_H_

class Thread;
class Semaphore;


typedef unsigned SignalId;

typedef int ID;

typedef void (*SignalHandler)();

typedef enum {IDLE, RUNNING, READY, BLOCKED, PAUSED} State;

class PerSig {
public:
	PerSig(SignalId idd, PerSig* nxt = 0);
	virtual ~PerSig();
protected:
	friend void interrupt timer();
	friend class PCB;
private:
	SignalId id;
	volatile PerSig* next;
	short exec;
};

class PCB {
private:
	unsigned sp;
	unsigned ss;
	unsigned* stack;
	unsigned size;
	short inftimeslice;
	unsigned isFinished;
	int quant;
	Thread* MyThread;
	Semaphore* Waiting;
	volatile State state;
	ID id;
	static ID uID;
	volatile unsigned int unblockWait;
	SignalHandler signali[16];
	short LocalMask[16];
	short LocSignalBlock[16];
	volatile PerSig* sighead;
	volatile PerSig* sigtail;
	short semret;
	PCB* myFather;
protected:
	friend class KernelSem;
	friend class KernelEv;
	friend class Thread;
	friend void forcekill();
	friend class semQ;
	friend void interrupt timer();
	friend int main(int argc, char* argv[]);
	friend class PerSig;
	friend class Semaphore;
	friend void inic();
	friend class Queue;
	friend void idl();
public:
	void PerformSignal(SignalId id) volatile;
	static void wrapper();
	void create_stack(void (*fp)() = wrapper);
	PCB(unsigned size, int slice, PCB* Father);
	~PCB();
};

#endif

