/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  Console::puts("Constructed Scheduler.\n");
  //q sits on the stack, no further action needed
}

void Scheduler::yield() {
    //need to disable interrupts here to ensure mutual exclusion
    //basically enter critical section here, so another interrupt
    //triggered yield will not cause race condition
//    Machine::disable_interrupts();
    Thread* t = q.pop();
    assert(t != 0);
    Thread::dispatch_to(t);
    Console::puts("thread yielded\n");
    //re enable interrupt: leave crtical section
//    Machine::enable_interrupts();
}

void Scheduler::resume(Thread * _thread) {
    bool ret = q.push(_thread);
    assert(ret);
    Console::puts("thread resumed\n");
}

void Scheduler::add(Thread * _thread) {
    resume(_thread);
    Console::puts("thread added\n");
}

void Scheduler::terminate(Thread * _thread) {
    //free current thread to avoid memory leak
    delete _thread;
    //if the thread is running, then it is probably not in the queue anyways
    yield();
}
