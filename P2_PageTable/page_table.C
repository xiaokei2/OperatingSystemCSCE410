#include "page_table.H"
// reference : http://www.osdever.net/tutorials/view/implementing-basic-paging


PageTable * PageTable :: current_page_table;
unsigned int PageTable::paging_enabled;
FramePool*	 PageTable::kernel_mem_pool;
FramePool*	 PageTable::process_mem_pool;
unsigned long	PageTable::shared_size;



void PageTable::init_paging(FramePool * _kernel_mem_pool,FramePool * _process_mem_pool,const unsigned long _shared_size)
{
	paging_enabled = 0;
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	return;
}


// the first level in the paging contains page directory 
PageTable::PageTable()
{
	page_directory = (unsigned long *)(kernel_mem_pool->get_frame()* 4096);
	page_table = (unsigned long *)(kernel_mem_pool->get_frame() * 4096);
	
	unsigned long addr=0;// holds the physical address of where a page is 
	// one entry in the page directory
	for(unsigned int i =0; i<1024;i++)
	{
		page_table[i]=addr|3; // set to read/write 3->011
		addr+=4096; // move to next table;
	}
	
	page_directory[0] = (unsigned long) page_table; // attribute set to : supervisor level, read/write, present(011 in binary)
	page_directory[0] = page_directory[0]|3;

	for(unsigned int i=0; i< ENTRIES_PER_PAGE;i++)
	{
		page_directory[i]=0|2;
	}
}	

void PageTable::load()
{
	/*
		load page table into processor context 
	*/
	write_cr3((unsigned long int )page_directory); // put that page directory address into CR3
	current_page_table=this;
}

void PageTable::enable_paging()
{
	write_cr0(read_cr0()|0x8000000);// set the paging bit in CR0 to 1
	paging_enabled=1;
	
}


void PageTable::handle_fault(REGS* _r)
{
	 if(_r->err_code!=0)
{
	Console::puts("Error Protection fault\n");
}
	 else {
	 unsigned long addr= read_cr2();
	 unsigned long *page_table;
// 	 get page index
	 unsigned long page_dir_index = addr >> 22;
	 unsigned long * page_directory = current_page_table->page_directory;
	 unsigned long  page_table_index= (addr>>12 )& 0x3FF;
// 	get the table address 
	 //unsigned long * page_table = (unsigned long *) page_directory[page_directory_index];


	// page table present in page directory. 31~20 bits DIR
	if(((page_directory[addr>>22] ) & 0x1 ) == 1){
		page_table=(unsigned long *)(page_directory[page_dir_index]&0xFFFFF000);
	}

	else{
	 // allocate a frame for the page table
		page_table=(unsigned long *)(kernel_mem_pool->get_frame()<<12);
		for(unsigned int i=0;i<1024;i++)
			page_table[i]=0; 

	// attribute set to : supervisor level. 011 read/write
		page_directory[page_dir_index]=(unsigned long) page_table|3;
	}
	// if there is a page_table already, just map it.
	unsigned long frame = process_mem_pool->get_frame();
	if(frame == 0) Console::puts("No avalible frame ");

	//attribute set to :supervisory level, 011
	page_table[page_table_index] = (frame<<12)|3;
	

	}
}
