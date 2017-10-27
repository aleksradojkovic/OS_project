/*
 * kernelsem.h
 *
 *  Created on: May 6, 2017
 *      Author: OS1
 */

#ifndef KERSEM_H_
#define KERSEM_H_

typedef unsigned int Time;

class semQ;
class kersemQ;

class KernelSem {
public:
	static kersemQ* semhead;
	static kersemQ* semtail;
	static void decrement();
	KernelSem(int init);
	virtual ~KernelSem();
	void wait(Time MaxTimeToWait);
	void signal();
	void signalAll();
	int val();
protected:
	friend void interrupt timer();
	friend class semQ;
private:
	int value;
	int noefsignal;
	semQ* queue;
};

#endif /* KERSEM_H_ */
