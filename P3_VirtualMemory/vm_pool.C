

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "page_table.H"
#include "vm_pool.H"
#include "console.H"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class PageTable */
VMPool::VMPool(unsigned long _base_address, unsigned long _size, FramePool* _frame_pool, PageTable * _page_table):base_address(_base_address),size(_size),frame_pool(_frame_pool),page_table(_page_table),n_regions(0)
{

	
	page_table->register_vmpool(this);
	max_regions = PageTable::PAGE_SIZE/sizeof(region);

	region_list = (region * )(PageTable::PAGE_SIZE* (frame_pool->get_frame()));
	page_table->register_vmpool(this);
	
}

 unsigned long VMPool::allocate(unsigned long _size)
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */
{
	
	unsigned long start_addr = 0; // starting address;
	start_addr = region_list[n_regions-1].base_address + region_list[n_regions-1].size;

	region_list[n_regions].base_address = start_addr;
	region_list[n_regions++].size=_size;
	if(n_regions> max_regions)
		Console::puts("no enough memory");
	return start_addr;
}
   void VMPool::release(unsigned long _start_address)
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */
{
	for(unsigned int i=0;i<n_regions;i++)
		if(region_list[i].base_address == _start_address)
			for(unsigned int k=0;k< region_list[i].size/PageTable::PAGE_SIZE;k++)
			{
				page_table->free_page(_start_address);
				_start_address+=PageTable::PAGE_SIZE;
			}

	page_table->load();
	
}

   BOOLEAN VMPool:: is_legitimate(unsigned long _address)
   /* Returns FALSE if the address is not valid. An address is not valid
    * if it is not part of a region that is currently allocated. */
{
	if((_address >(base_address+size))||(_address< base_address))
		return false;
	return true;
}

