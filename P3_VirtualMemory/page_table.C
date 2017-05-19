#include "page_table.H"
// reference : http://www.osdever.net/tutorials/view/implementing-basic-paging


PageTable * PageTable :: current_page_table;
unsigned int PageTable::paging_enabled;
FramePool*	 PageTable::kernel_mem_pool;
FramePool*	 PageTable::process_mem_pool;
unsigned long	PageTable::shared_size;



void PageTable::init_paging(FramePool * _kernel_mem_pool,FramePool * _process_mem_pool,const unsigned long _shared_size)
{
	//paging_enabled = 0;
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;


}


// the first level in the paging contains page directory 
PageTable::PageTable()
{
	page_directory = (unsigned long *)(process_mem_pool->get_frame()* 4096);
	page_table = (unsigned long *)(process_mem_pool->get_frame() * 4096);
	
	unsigned long addr=0;

	for(unsigned int i =0; i<1024;i++)
	{
		page_table[i]=addr|3; // set to read/write 3->011
		addr+=4096; // move to next table;
	}
	
	page_directory[0] = (unsigned long) page_table |3; 


	
	for(unsigned int i=1; i< ENTRIES_PER_PAGE;i++)
	{
		page_directory[i]=0|2;
	}
	page_directory[1023]=(unsigned long)page_directory|3; // set last entry in page directory to point itself
	for(unsigned int i=0;i<vm_pool_size;i++)
		vm_pool_array[i]=NULL;


}	

void PageTable::load()
{
	/*
		load page table into processor context 
	*/

	current_page_table=this;
	write_cr3((unsigned long)(current_page_table->page_directory)); // put that page directory address
 //into CR3
}

void PageTable::enable_paging()
{
	write_cr0(read_cr0()|0x8000000);// set the paging bit in CR0 to 1
	paging_enabled=1;



}

void PageTable::free_page(unsigned long page_number)
{

	unsigned long page_index = (page_number >> 12) & 0x3FF;
	unsigned long *page_table= (unsigned long *)(0xFFC00000|((page_number>>22) << 12));
	//release the frame_number 
	process_mem_pool->release_frame(page_table[page_index]);

}
void PageTable::register_vmpool(VMPool * _pool)
{
	for(unsigned int i =0; i<vm_pool_size;i++)
	{
		if(vm_pool_array[i]== NULL)
		{
		// find avalible array to register the pool;
			vm_pool_array[i]=_pool;
			Console::puts("successfully registered the pool");
			break;
		}
	}
}	
void PageTable::handle_fault(REGS* _r)
{
	 unsigned long * page_dir = (unsigned long *) 0xFFFFF000;
	 if(_r->err_code!=0)
{
	Console::puts("Error Protection fault\n");
}
	 else {
	 unsigned long addr= read_cr2();
	 unsigned long *page_table;

	 unsigned long  page_dir_index = addr >> 22;

	 unsigned long  page_table_index= (addr>>12 )& 0x3FF;
	
	 VMPool **vm_pool_array= current_page_table->vm_pool_array;
	 for(unsigned int i =0; i<vm_pool_size;i++)
		if(vm_pool_array[i]!=NULL)
			if(vm_pool_array[i]->is_legitimate(addr))
			{
				Console::puts("Valid Address\n");
				break;
			}



	// page table present in page directory. 31~20 bits DIR
	if(((page_dir[page_dir_index] ) & 0x1 ) == 1){
		page_table=(unsigned long *)(0XFFC00000|page_dir_index<<12);

		page_table[page_table_index] = ((process_mem_pool->get_frame()<<12)|3);
	}

	else{
	 // allocate a frame for the page table
		page_dir[page_dir_index] =(unsigned long )((process_mem_pool->get_frame()<<12)|3);
		page_table=(unsigned long *)((0XFFC00000|page_dir_index<<12)); 

		for(unsigned int i=0;i<1024;i++)
			page_table[i]=2; 

	// attribute set to : supervisor level. 011 read/write // load page table into the directory
		page_table[page_table_index]=(unsigned long) page_table|3;
	
	// if there is a page_table already, just map it.
		unsigned long frame = process_mem_pool->get_frame();
		if(frame == 0) Console::puts("No avalible frame ");

	//attribute set to :supervisory level, 011
		page_table[page_table_index] = (frame<<12)|3;
	}	

	}
}
