/* 
    Author: Joshua Capehart
			Based on code by:
			R. Bettati
            Modified by Dilma Da Silva
            Department of Computer Science and Engineering
            Texas A&M University
			
			A thread scheduler.

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

/*--------------------------------------------------------------------------*/
/* DEFINES 
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INCLUDES 
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "thread.H"
#include "queue.H"
/*--------------------------------------------------------------------------*/
/* SCHEDULER
/*--------------------------------------------------------------------------*/

class Scheduler {

   /* The scheduler may need private members... */

public:
   

   /* Setup the scheduler. This sets up the ready queue, for example.
      If the scheduler implements some sort of round-robin scheme, then the 
      end_of_quantum handler is installed here as well. */
   Scheduler();

   /* Called by the currently running thread in order to give up the CPU. 
      The scheduler selects the next thread from the ready queue to load onto 
      the CPU, and calls the dispatcher function defined in 'threads.h' to
      do the context switch. */
   virtual void yield();

   /* Add the given thread to the ready queue of the scheduler. This is called
      for threads that were waiting for an event to happen, or that have 
      to give up the CPU in response to a preemption. */
   virtual void resume(Thread * _thread);

   /* Make the given thread runnable by the scheduler. This function is called
	  typically after thread creation. Depending on the
      implementation, this may not entail more than simply adding the 
      thread to the ready queue (see scheduler_resume). */
   virtual void add(Thread * _thread);

   /* Remove the given thread from the scheduler in preparation for destruction
      of the thread. */
   virtual void terminate(Thread * _thread);

private:
	Queue<Thread*> q;
};
	
	

#endif
