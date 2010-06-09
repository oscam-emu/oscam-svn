
#ifndef CTHREAD_H_
#define CTHREAD_H_

#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <typeinfo>
#include <cstdarg>
#include <sys/resource.h>

using namespace std;


//--------------------------------------------------------------------
// CLASS --- cMutex ---
//--------------------------------------------------------------------
class cMutex
{
private:
      pthread_mutex_t mutex;
      int locked;
public:
     cMutex(void);
     ~cMutex();
     void Lock(void);
     void Unlock(void);
};

//--------------------------------------------------------------------
// CLASS --- cThread ---
//--------------------------------------------------------------------
class cThread
{
private:
	  bool running;
	  bool terminate;
	  string description;
	  pthread_t threadPT;

	  cMutex mutex;
	  static void *startThread(cThread *Thread);
protected:
	  void Sleepms(unsigned int msec);
	  void Lock(void) { mutex.Lock(); }
	  void Unlock(void) { mutex.Unlock(); }
	  bool Terminated(void) { return terminate; };
	  virtual void Execute() = 0;
public:
	  cThread(const string Description, int Priority);
	  virtual ~cThread();
	  bool isActive(void);
	  void Stop(int waitSeconds = 0);
	  void Start(void);
};

#endif /* CTHREAD_H_ */
