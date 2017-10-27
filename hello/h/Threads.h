/*
 * Threads.h
 *
 *  Created on: May 26, 2017
 *      Author: OS1
 */


struct Threads{
	Thread* t;
	Threads* next;
	Threads(Thread* tt, Threads* nxt = 0) { t = tt; next = nxt;	}
};


