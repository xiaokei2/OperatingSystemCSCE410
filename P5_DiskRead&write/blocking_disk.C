#include "blocking_disk.H"



void BlockingDisk::wait_until_ready()
{
	//Console::puts("waittttttttttttttttttttttttttttttttttttttt readddddddddddddddddddddd\n");
	while(!is_ready())
	{
		// the thread gives up the CPU and wait until the operation is complete ;
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
		SYSTEM_SCHEDULER->yield();
	}
}

