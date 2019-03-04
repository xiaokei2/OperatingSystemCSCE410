#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"
PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    shared_size = _shared_size;
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    paging_enabled = 0; //we are not ready yet

    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    //reference: www.osdever.net/tutorials/view/implementing-basic-paging
    head = NULL;
    tail = NULL;

    //setup up directory, by getting a frame from the kernel pool
    unsigned long frame = process_mem_pool->get_frames(1);
    unsigned long addr = frame * (4 KB);
    page_directory = (unsigned long*) addr; 

    //now we need to setup paging for the first 4MB (kernel space)
    frame = process_mem_pool->get_frames(1);
    addr = frame * (4 KB);
    unsigned long* page_table = (unsigned long*) addr;
    unsigned long physical_addr = 0;
    
    for(unsigned int i = 0; i < 1024; i++){
        //0b'011, 0 = kernel, 1 = read/write, 1 = present
        page_table[i] = physical_addr | 3;
        physical_addr += 4096; //each page is 4k, each page table manages 1024 pages
    }

    //fill in the page directory entry for this page
    page_directory[0] = (unsigned long) page_table | 3;

    //mark the remaining PDE as not present
    for(unsigned int i = 1; i < 1023; i++){
        page_directory[i] = 0 | 2;
    }
    
    //last entry is reserved to point to itself
    page_directory[1023] = physical_addr | 3;

    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    write_cr3((unsigned long)page_directory); //loads the page_directory to cr3
    current_page_table = this; //current table is loaded
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000); 
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    static int fault_count = 1;
    //Console::puts("..."); Console::puti(fault_count); Console::puts("th fault\n");
    //Console::puts("Handling page fault...");
    unsigned int error_code = _r->err_code;
    unsigned int bit_0 = error_code & 1;
    unsigned int bit_1 = error_code >> 1 & 1;
    unsigned int bit_2 = error_code >> 2 & 1;

    unsigned long fault_addr = read_cr2();
    //Console::puts("fault addr = "); Console::putui(fault_addr); Console::puts("\n");
    if(bit_0 == 0){
        //need to first check if PD has an entry for the corresponding page
        unsigned int page_table_nr = (fault_addr & 0xFFC00000) >> 22; //first 10 bits
        unsigned long* page_directory = (unsigned long*) 0xFFFFF000; //recursive lookup
        //the page table page exists
        if((page_directory[page_table_nr] & 1) == 1){
            unsigned long* page_table = (unsigned long*) (0xFFC00000 | (page_table_nr << 12));
            unsigned int page_nr = (fault_addr & 0x3FF000) >> 12;
            //allocate a page from proccess pool
            unsigned long frame = process_mem_pool->get_frames(1);
            unsigned long page_addr = frame * (4 KB);
            page_table[page_nr] = page_addr | 7; //1 1 1, usr | w/r | present
            fault_count++;
        }
        //need to allocate a page table for the fault address and add it to page dir
        else{
            //get a frame to store the page table
            unsigned long frame = process_mem_pool->get_frames(1);
            unsigned long page_table_addr = frame * (4 KB);
            unsigned long* page_table = (unsigned long*) page_table_addr;

            page_directory[page_table_nr] = page_table_addr | 3;

            for(unsigned int i = 0; i < 1024; i++){
                page_table[i] = 6; //doesn't matter whats in front, 1 1 0, user | w/r | invalid
            }
        }
        
    }
    //assert(false);
    //Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool* _vm_pool){
    if(head == NULL){
        head = _vm_pool;
        tail = _vm_pool;
    }else{
        tail->next = _vm_pool;
        tail = _vm_pool;
    }
}

void PageTable::free_page(unsigned long _page_no){
    bool isValid = false;
    VMPool* cur = head;
    while(cur != NULL){
        if(cur->is_legitimate(_page_no)){
            isValid = true;
            break;
        }
    }
    
    if(!isValid)
        assert("xD: Segmentation Fault! _page_no not legitimate!");

    unsigned long page_table_nr = (_page_no & 0xFFC00000) >> 22;
    unsigned long page_nr = _page_no & 0x3FF000; 

    unsigned long* page_table = (unsigned long*) (0xFFC00000 | (page_table_nr << 12));
    page_table[page_nr] = 0; //invalid

    //flush tlb; i.e. reload cr3
    write_cr3((unsigned long)page_directory);
}
