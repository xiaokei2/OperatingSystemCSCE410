/* 
    Author: Dilma DA Silva
            Department of Computer Science and Engineering
            Texas A&M University
			
			A thread scheduler.

*/

/*--------------------------------------------------------------------------*/
/* DEFINES 
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INCLUDES 
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "console.H"

extern void operator delete(void*);

extern Scheduler* SYSTEM_SCHEDULER;

extern Thread* current_thread;





Scheduler::Scheduler() 
{
  q = Queue<Thread*>();
	return;
}

void Scheduler::yield() 
{
	if(machine_interrupts_enabled())
		machine_disable_interrupts();
 //Console::puts("Yielddddddddddddddddddddddddddddddddddd");
  Thread *target = q.dequeue();
  
  Thread::dispatch_to(target);
  
}

void Scheduler::resume(Thread* _thread) 
{
	if(machine_interrupts_enabled())
		machine_disable_interrupts();
 //Console::puts("RESSSSSUMMMMMMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
	q.enqueue(_thread);
	
}

void Scheduler::add(Thread* _thread) 
{
 //Console::puts("aDDDDDDDDDDDDDDDDDDDDDDDDD");
	q.enqueue(_thread);

}

void Scheduler::terminate(Thread* _thread) 
{
	// need to releasing the CPU, releasing memory, giving control to the next thread.
	// we still need to release the thread memory
	// without dispatch the thread, we can use the yield first and delete the object later. 
	yield();
	//q.dequeue();

}
void thread_shutdown() {
        /* This function should be called when the thread returns from the thread function.
           It terminates the thread by releasing memory and any other resources held by the thread. 
           This is a bit complicated because the thread termination interacts with the scheduler.
        */
        
        SYSTEM_SCHEDULER->terminate(current_thread->CurrentThread());
	delete current_thread; // deletes the thread
	
}
