/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

#define HEAD_OF_SEQUENCE 2
#define ALLOCATED        0
#define FREE             3

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

//define static pointers here
ContFramePool* ContFramePool::head = NULL;
ContFramePool* ContFramePool::tail = NULL;

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    //2 bits per frame; so it will be FRAME_SIZE * 4
    //makes sure that bitmap fits in a single frame
    assert(_n_frames <= FRAME_SIZE * 4);

    base_frame_no = _base_frame_no;
    n_frames      = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;
    n_free_frames = n_frames;
    //if info_frame_no == 0, then we default it to the first frame
    //else use the provided frame. note: looks like it could be outside 
    //the frame pool
    if(info_frame_no == 0)
        bitmap = (BitField*) (base_frame_no * FRAME_SIZE);
    else
        bitmap = (BitField*) (info_frame_no * FRAME_SIZE);
    
    // number of frames must entirely fill the bitmap
    for(unsigned long i = 0; i*4 < n_frames; i++)
        bitmap[i].initialize(); //initialize sets all bits to 1
        
    //set bitmap assigned to info frame
    if(info_frame_no == 0){
        //set head of seq
        bitmap[0].set(0, HEAD_OF_SEQUENCE);
        for(int i = 1; i < _n_info_frames; i++){
            unsigned int bitmap_index = (i - base_frame_no) / 4;
            unsigned int position = (i - base_frame_no) % 4;
            bitmap[bitmap_index].set(position, ALLOCATED);
        }
        n_free_frames -= _n_info_frames;
    }

    //now chain the pools together
    if(head == NULL){
        head = this;
        tail = this;
        this->next = NULL;
    }else{
        tail->next = this;
        this->next = NULL;
    }

    Console::puts("Frame pool initialized!\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    //find the first frame that is free, start linear search from base frame
    //note that the bitmap always start from 0, but the base may not 
    unsigned int n_contiguous_free = 0;
        
    for(unsigned int i = base_frame_no; i < base_frame_no + n_frames; i++){
        unsigned int bitmap_index = (i - base_frame_no) / 4;
        unsigned int position = (i - base_frame_no) % 4;

        if(bitmap[bitmap_index].get(position) == FREE)
            n_contiguous_free++;
        else
            n_contiguous_free = 0;

        assert(n_contiguous_free <= _n_frames); //something went wrong if this is ever the case
        
        //check to see if we found it
        if(n_contiguous_free == _n_frames){
            unsigned int head = i - n_contiguous_free + 1;
            mark_inaccessible(head, n_contiguous_free);
      
            return head;
        }
    }
    //since we havn't found one, return 0
    return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    //assert that what we are marking is within range
    assert((_base_frame_no >= base_frame_no) && (_base_frame_no + _n_frames <= base_frame_no + n_frames));
    
    //mark head of seq
    unsigned int bitmap_index = (_base_frame_no - base_frame_no) / 4;
    unsigned int position = (_base_frame_no - base_frame_no) % 4;
    bitmap[bitmap_index].set(position, HEAD_OF_SEQUENCE);
    //mark the remaining as allocated
    for(int i = _base_frame_no + 1; i < _base_frame_no + _n_frames; i++){
        bitmap_index = (i - base_frame_no) / 4;
        position = (i - base_frame_no) % 4;
        bitmap[bitmap_index].set(position, ALLOCATED);
    }
    n_free_frames -= _n_frames;
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    //locate the pool that contains the frame number
    ContFramePool* pool_ptr = ContFramePool::head;
    while(pool_ptr != NULL){
        if(pool_ptr->is_belong(_first_frame_no)){
            //found the pool that owns it
            pool_ptr->local_release_frames(_first_frame_no);
            return;
        }
        pool_ptr = pool_ptr->next;        
    }
    //Console::puts("Frame not found in any pool!");
}

inline bool ContFramePool::is_belong(unsigned long _first_frame_no){
    return (_first_frame_no >= base_frame_no && _first_frame_no < base_frame_no + n_frames);
}

void ContFramePool::local_release_frames(unsigned long _first_frame_no){
    
    unsigned int i = _first_frame_no;
    unsigned int bitmap_index = (i - base_frame_no) / 4;
    unsigned int position = (i - base_frame_no) % 4;
    i++;
    //free head_of_seq
    bitmap[bitmap_index].set(position, FREE);
    n_free_frames++;
    while(true){
        bitmap_index = (i - base_frame_no) / 4;
        position = (i - base_frame_no) % 4;
        //if we see a free block or if we see head of sequence, then we are done
        //also need to make sure we are not freeing out of boundary
        if(bitmap[bitmap_index].get(position) == FREE || bitmap[bitmap_index].get(position) == HEAD_OF_SEQUENCE
                || i >= base_frame_no + n_frames){
            return;
        }else{
            bitmap[bitmap_index].set(position, FREE);
            n_free_frames++;
            i++;
        }
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    //n_per_frame is the number of frames one frame can manage
    unsigned int n_per_frame = FRAME_SIZE * 4; //each byte can manage four frames
    unsigned int n_required = _n_frames / n_per_frame + (_n_frames % n_per_frame > 0 ? 1 : 0);
    return n_required;
}

unsigned long ContFramePool::n_free_frames_left(){
    return n_free_frames;
}
