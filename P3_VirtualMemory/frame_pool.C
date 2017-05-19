#include "frame_pool.H"
#include "console.H"
#define frame_size 4096
// cited :http://stackoverflow.com/questions/1225998/what-is-a-bitmap-in-c

// static 
unsigned long begin_frame;
unsigned long size;



enum { BITS_PER_WORD = 32};
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

void set_bit(unsigned int *words, int n) { 
    words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void clear_bit(unsigned int *words, int n) {
    words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n)); 
}

int get_bit(unsigned int *words, int n) {
    unsigned int bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0; 
}
bool range(int n)
{
	if((n>begin_frame+size)||(n<begin_frame)) {return false;}
return true;
}
/*
	512-1024 are the actual frame numbers for the real memory address. 4096*(512-1024) will return the real address of that frame where it starts. you may
*/
FramePool::FramePool(unsigned long _base_frame_no, unsigned long _nframes,unsigned long _info_frame_no):begin_frame(_base_frame_no),size(_nframes),frame_number_store(_info_frame_no)
{
	if(frame_number_store==0) // invoke the kernel process, there should be 2MB/4KB 512 frames.
{
	//Console::puts("The begin frame\n");
	//Console::puti(size);
	bitmap=(unsigned int*)(begin_frame*frame_size);//512*4096=2MB
	inacess_map=bitmap+250; // store the inacess map after the bitmap 64;

	
        //512/32=16 //a bit as a page or frame. 32 bits as int. we have 16 int dex for kernel process 
	for(int i=0; i<16;i++)
		{bitmap[i]=0; // intialize the bitmap
		inacess_map[i]=0;
		}
		mark_inaccessible(512,1);
}
	else{
		bitmap=(unsigned int*)(frame_number_store*frame_size);// start at 4MB
		inacess_map=bitmap+1200; // store bitmap after 896
		for(int i=0;i<224;i++)
			{bitmap[i]=0;
			 inacess_map[i]=0;}
		mark_inaccessible(frame_number_store,1);
			
	}
}
unsigned long FramePool::get_frame()
{       //Console::puts("Get frame : size\n");
	//Console::puti(size);

	for(int i=0; i<size;i++) // for each int. we have 4 bytes and 32 bits total
{		        //if(get_bit(inacess_map,i)==1){ Console::puts(" Inacess  DETECTED\n"); Console::puti(i);}
			if(get_bit(bitmap,i)==0&&get_bit(inacess_map,i)==0) // check bitmap is free and available accessbile or not.
			{	
		//		Console::puts("nested loop i:\n");
			   set_bit(bitmap,i);
			return (begin_frame +i); //found the specifc frame and return num of frame.

			}
		

}
 return  0xffffffff;
}


unsigned long FramePool::release_frame(unsigned long _frame_no)
{
	/*unsigned long row =_frame_no/32; // find the specifc location of the bit, so we can release and free the offest as zero
*/
	//if(mark_inaccessible(_frame_no)==1||!range(_frame_no))
	if(!range(_frame_no))
	return  0xffffffff;
	clear_bit(bitmap,_frame_no);
	return 0; // function succeed and return 0
}

unsigned long FramePool::mark_inaccessible(unsigned long _base_frame_no, unsigned long _nframes)
{

		
		unsigned long start =(_base_frame_no-begin_frame);
		if(!range(start+_nframes)) return  0xffffffff;

	//	cout<<" the row "<< row<<endl; 
		//Console::puts("the row \n");
		//Console::puti(row);
	//	Console::puts("the offset \n");
		//Console::puti(bit_index);
	  //      set_bit(inacess_map,23);
		for(unsigned int i=0;i<_nframes;i++)
			set_bit(inacess_map,start+i);			
		for(unsigned int i=0;i<_nframes;i++)
			if(get_bit(inacess_map,start+i))
				//{Console::puts(" Bit already set \n");}

		/*if (get_bit(inacess_map,23)==1)
		     {Console::puts(" Bit already set \n");}*/
			
		

		return 0;
}



