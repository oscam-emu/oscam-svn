

#include "cthread.h"
#include "global.h"

//--------------------------------------------------------------------
// IMPLEMENTATION --- cMutex class ---
//--------------------------------------------------------------------
cMutex::cMutex(void)
{
  locked = 0;
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
  pthread_mutex_init(&mutex, &attr);
}

//-----------------------------------------
cMutex::~cMutex()
{
  pthread_mutex_destroy(&mutex);
}

//-----------------------------------------
void cMutex::Lock(void)
{
  pthread_mutex_lock(&mutex);
  locked++;
}

//-----------------------------------------
void cMutex::Unlock(void)
{
 if (!--locked)
    pthread_mutex_unlock(&mutex);
}

//--------------------------------------------------------------------
// IMPLEMENTATION --- cThread class ---
//--------------------------------------------------------------------
cThread::cThread(const string Description, const int Priority)
{
  running = false;
  terminate = false;
  threadPT = NULL;

  if (Priority) setpriority(PRIO_PROCESS, 0, Priority);
  if (!Description.empty()) description = Description;
}

//-----------------------------------------
cThread::~cThread()
{
   Stop(2); // try to stop thread in case is running, 2 sec timeout
}

//-----------------------------------------
void cThread::Stop(int waitSeconds)
{
   // if thread not runing, exit
   if (!isActive())
	{ threadPT = 0; return; }
   // signal thread to stop - exit
   terminate = true;
   // waiting parameter < 1, no wait specified, exit
   if (waitSeconds < 1)
	  return;
   // wait for thread to exit
   int timeOutCounter = waitSeconds * 10;
   while(isActive() && timeOutCounter--) {
	  Sleepms(100);
   }
   // check for time out, if not
   if (!timeOutCounter)
	 throw StandardException("Can't stop thread, timeout!");

   threadPT = 0;
}

//-----------------------------------------
void *cThread::startThread(cThread *Thread)
{
	Thread->running = true;
	Thread->Execute();
	Thread->running = false;

	return NULL;
}

//-----------------------------------------
void cThread::Start()
{
   // if thread is runing, exit
   if (isActive())
	  return;
   // crate thread
   terminate = false;
   if (pthread_create(&threadPT, NULL, (void *(*) (void *))&startThread, (void *)this) != 0)
	  throw StandardException("Can't start thread!");
   // wait max 1 sec for thread to start
   int timeOutCounter = 10;
   while(!running && timeOutCounter--)
	 {  Sleepms(100); }
   if (!timeOutCounter)
	  throw StandardException("Thread started but not running, timeout!");
}

//-----------------------------------------
bool cThread::isActive()
{
   if (running && !threadPT)
	  throw StandardException("Thread suppose to be running but thread pointer is NULL!");

   if (running && pthread_kill(threadPT, 0) != 0)
	  throw StandardException("Thread suppose to be running but I cant find thread!");

   return running;
}

//-----------------------------------------
void cThread::Sleepms(unsigned int msec)
{
	//does not interfere with signals like sleep and usleep do
	struct timespec req_ts;
	req_ts.tv_sec = msec/1000;
	req_ts.tv_nsec = (msec % 1000) * 1000000L;
	nanosleep (&req_ts, NULL);
}
//-----------------------------------------
