/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    
    next = NULL;
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    n_allocated_regions = 0;

    page_table->register_pool(this);
    
    //setup local table to keep track of allocated regions
    //this may become problematic when regions are released, lets see what happens for now
    allocated_regions = (Regions*) base_address; //we will use the first 4KB of this VMPool to store
    
    //mark the first 4 KB as allocated
    allocated_regions[n_allocated_regions].region_base = _base_address;
    allocated_regions[n_allocated_regions].region_len = 4096; //4 KB

    n_allocated_regions++;

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned int region_length = ((_size / 4096) + 1) * 4096; //multiples of page size

    unsigned long region_begin = allocated_regions[n_allocated_regions - 1].region_base
        + allocated_regions[n_allocated_regions - 1].region_len;

    allocated_regions[n_allocated_regions].region_base = region_begin;
    allocated_regions[n_allocated_regions].region_len = region_length;
    
    n_allocated_regions++;

    return region_begin;
}

void VMPool::release(unsigned long _start_address) {
    if(!is_legitimate(_start_address)){
        assert("xD: Segmentation Fault!\n");
    }

    for(unsigned int i = 0 ; i < n_allocated_regions; i ++){
        unsigned long region_begin = allocated_regions[i].region_base;
        unsigned long region_end = region_begin + allocated_regions[i].region_len - 1;//inclusive

        if(_start_address >= region_begin && _start_address <= region_end){
            //use this to mark released region, bad, but since we were told to not to optimize
            allocated_regions[i].region_base = 0;
            allocated_regions[i].region_len =0;
            break;
        } 

    }    
    
    //ask page_table manager to deallocate free this page
    page_table->free_page(_start_address); 

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    //scan through allocated regions to see if it lies in between any
    for(unsigned int i = 0; i < n_allocated_regions; i++){
        unsigned long region_begin = allocated_regions[i].region_base;
        unsigned long region_end = region_begin + allocated_regions[i].region_len - 1;//inclusive

        if(_address >= region_begin && _address <= region_end)
            return true;
    }

    return false; //havn't found any
    Console::puts("Checked whether address is part of an allocated region.\n");
}

