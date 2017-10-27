// File: event.h
#ifndef _event_h_
#define _event_h_

#include "IVTEntry.h"

typedef unsigned char IVTNo;


#define PREPAREENTRY(numEntry, callOld)\
void interrupt inter##numEntry(...); \
IVTEntry newEntry##numEntry(numEntry, inter##numEntry); \
void interrupt inter##numEntry(...) {\
newEntry##numEntry.signal();\
if (callOld == 1)\
newEntry##numEntry.old();\
dispatch();\
}


class KernelEv;

class Event {
public:
	Event (IVTNo ivtNo);
	~Event ();
	void wait ();
protected:
	friend class KernelEv;
	void signal(); // can call KernelEv
private:
	KernelEv* myImpl;
};
#endif
