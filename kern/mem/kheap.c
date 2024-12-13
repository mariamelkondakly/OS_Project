#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


#include<kern/conc/sleeplock.h>
struct sleeplock kernel_lock;

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
//struct spinlock kernel_lock;
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit) {
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

    start = daStart;
    hard_limit = daLimit;
    init_sleeplock(&kernel_lock, "Kernel Lock");

    uint32 edited_initSizeToAllocate = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
    Break = start + edited_initSizeToAllocate;

    if (Break > hard_limit || initSizeToAllocate == 0) {
        panic("Initial size exceeds heap limit or size is 0!");
        return -1;

    }

    uint32 last_successful_va = start;

    for (uint32 i = start; i < Break; i += PAGE_SIZE) {
        struct FrameInfo *ptr = NULL;


        int ret = allocate_frame(&ptr);
        if (ret == E_NO_MEM || ptr == NULL) {

            panic("NO MEMORY while allocating frames!");



            for (uint32 k = start; k < last_successful_va; k += PAGE_SIZE) {
                unmap_frame(ptr_page_directory, k);
                free_frame(get_frame_info(ptr_page_directory, k, NULL));
            }
            return -1;
        }


        ret = map_frame(ptr_page_directory, ptr, i, PERM_AVAILABLE | PERM_WRITEABLE);
        if (ret == E_NO_MEM) {
            panic("NO MEMORY while mapping frames!");

            for (uint32 j = start; j < last_successful_va; j += PAGE_SIZE) {
                unmap_frame(ptr_page_directory, j);
                free_frame(get_frame_info(ptr_page_directory,j, NULL));
            }


            free_frame(ptr);
            return -1;
        }


        last_successful_va = i + PAGE_SIZE;
    }


    initialize_dynamic_allocator(start, Break - start);

 return 0;

}


void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
		 * 				you should allocate pages and map them into the kernel virtual address space,
		 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
		 * numOfPages = 0: just return the current position of the segment break
		 *
		 * NOTES:
		 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
		 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
		 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");

	//cprintf("2.0 sbrk entered \n \n");
	//cprintf("number of pages: %d \n \n", numOfPages);

	uint32 neededSize=numOfPages*PAGE_SIZE;
	int sizeAvailable=hard_limit-Break;

	if (Break + neededSize > hard_limit) {
	    cprintf("Requested size exceeds heap limit.\n");
	    return (void*)-1;
	}

	//cprintf("Current Break: %d, Requested Pages: %d, Needed Size: %d\n", Break, numOfPages, neededSize);
	//cprintf("Available Space: %d, Hard Limit: %d\n", hard_limit - Break, hard_limit);

	if(numOfPages==0){
		//cprintf("number of pages is 0, original break is returned. \n");
		//cprintf("2.1 sbrk return with numofpages=0 \n \n");
		return (void*)Break;
	}
	if (numOfPages > 0 && numOfPages > (hard_limit - Break) / PAGE_SIZE) {
	    cprintf("Heap overflow: Too many pages requested.\n");
	    return (void*)-1;
	}

	else if(numOfPages>0 ){
		if( neededSize<sizeAvailable){
		//if there is space for the new allocations
		uint32 prevBreak=Break;
	    uint32 last_successful_va = Break;
		for (uint32 i = Break; i < Break+neededSize; i += PAGE_SIZE){

			struct FrameInfo *ptr=NULL;

			int x=allocate_frame(&ptr); //allocation of the new frame
			if (x == E_NO_MEM || ptr == NULL) {
			    cprintf("Frame allocation failed at address: %d\n", i);
			    for (uint32 k = prevBreak; k < i; k += PAGE_SIZE) {
			        unmap_frame(ptr_page_directory, k);
			        free_frame(get_frame_info(ptr_page_directory, k, NULL));
			    }
			    return (void*)-1;
			}

			int y = map_frame(ptr_page_directory,ptr,i,PERM_AVAILABLE|PERM_WRITEABLE); //mapping the allocated frame
			if (x == E_NO_MEM || ptr == NULL) {

						panic("NO MEMORY while allocating frames!");
						for (uint32 k = Break; k < Break+neededSize; k += PAGE_SIZE) {
							unmap_frame(ptr_page_directory, k);
							free_frame(get_frame_info(ptr_page_directory, k, NULL));
						}
						return (void*)-1;
					}
		}
		Break+=neededSize;
		//cprintf("Current Break: %d,\n", Break);

        //struct BlockElement* lastFreeBlock = LIST_LAST(&freeBlocksList);
        //cprintf("old last free block size: %d \n \n",get_block_size(LIST_LAST(&freeBlocksList)));
        //lastFreeBlock = LIST_LAST(&freeBlocksList);
        //cprintf("NEW last free block size: %d \n \n",get_block_size(lastFreeBlock));

		return (void*)prevBreak;

		}
		else{ //if the number of pages is less than 0

			//cprintf("2.4 sbrk return at 129 \n \n");
			return (void*)-1 ;
		}
	}
	//cprintf("2.5 returned with null at line 132\n \n");
	return (void*)-1;

}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

     //cprintf("The Size equals %d \n",size);

	acquire_sleeplock(&kernel_lock);
	if(!isKHeapPlacementStrategyFIRSTFIT()){
		release_sleeplock(&kernel_lock);
		return NULL;
	}
	 if (size == 0 || size > (KERNEL_HEAP_MAX - KERNEL_HEAP_START)) {
			        cprintf("Invalid size for kmalloc: %u\n", size);
			        release_sleeplock(&kernel_lock);
			        return NULL;
			    }

	          //Block Allocator:
			  if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
			  		  //cprintf("ms1 alloc \n");
			  		  void * ptr =alloc_block_FF(size);
			  		  if(ptr==NULL){
			  			release_sleeplock(&kernel_lock);
		  			  return NULL;
			  		  }
			  		release_sleeplock(&kernel_lock);
			  		  return ptr;
			  	  }
			  //Page Allocator:
			    uint32 first_va_found = hard_limit+PAGE_SIZE;
			    int no_Of_required_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

			    int pagesCounter = 0;

			    // Find contiguous free pages
			    while (first_va_found < KERNEL_HEAP_MAX) {
			        uint32 *page_table;
			        struct FrameInfo *frame = get_frame_info(ptr_page_directory, first_va_found, &page_table);

			        if (frame == NULL) {
			            pagesCounter++;
			            if (pagesCounter == no_Of_required_pages) {
			                first_va_found -= (no_Of_required_pages - 1) * PAGE_SIZE;
			                break;
			            }
			        } else {
			            pagesCounter = 0;
			        }

			        first_va_found += PAGE_SIZE;
			    }

			    if (pagesCounter < no_Of_required_pages) {
			        cprintf("Not enough contiguous space in kernel heap\n");
			        release_sleeplock(&kernel_lock);
			        return NULL;
			    }

                 // Allocating frames
			    for (int i = 0; i < no_Of_required_pages; i++) {
			        struct FrameInfo *frame_info = NULL;
			        int ret = allocate_frame(&frame_info);

			        if (ret == E_NO_MEM || frame_info == NULL) {
			            cprintf("No memory for frame allocation\n");

			             // Unmapping and freeing
			            for (uint32 k = first_va_found; k < first_va_found + i * PAGE_SIZE; k += PAGE_SIZE) {
			                unmap_frame(ptr_page_directory, k);
			                free_frame(get_frame_info(ptr_page_directory, k, NULL));
			            }
			            release_sleeplock(&kernel_lock);
			            return NULL;
			        }

			        // Map frames to the virtual addresses
			        ret = map_frame(ptr_page_directory, frame_info, first_va_found + i * PAGE_SIZE, PERM_AVAILABLE | PERM_WRITEABLE);
			        if (ret == E_NO_MEM) {
			            cprintf("No memory for frame mapping\n");

			            // Unmapping and freeing
			            for (uint32 j = first_va_found; j < first_va_found + i * PAGE_SIZE; j += PAGE_SIZE) {
			                unmap_frame(ptr_page_directory, j);
			                free_frame(get_frame_info(ptr_page_directory, j, NULL));
			            }


			            free_frame(frame_info);
			            release_sleeplock(&kernel_lock);
			            return NULL;
			        }
			    }
			    // putting the allocated page into the array with its size and VA
			    struct allocated_together str;
			   	str.size=size;
			   	str.VA=(void*)first_va_found;
			   	for(int i=0;i<ARR_SIZE;i++){
			   		if(pages_together[i].VA==NULL)
			   		{
			   			pages_together[i] = str;
			   			break;
			   		}
			   	}

			   //cprintf("list done \n");
			   //cprintf("add returned from kmalloc  %d \n" ,(void*)first_va_found);
			   	release_sleeplock(&kernel_lock);
			    return (void*)first_va_found;

}



void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	 //cprintf("address wanted to be freed %d \n" ,virtual_address);
     //cprintf("hard_limit+PAGE_SIZE %d \n",hard_limit+PAGE_SIZE);
     //cprintf("KERNEL_HEAP_MAX %d \n",KERNEL_HEAP_MAX);

     //Block Allocator:
	acquire_sleeplock(&kernel_lock);
	if((uint32)virtual_address>=KERNEL_HEAP_START && (uint32)virtual_address<=Break){
				free_block(virtual_address);
			}
	  //Page Allocator:
			else if((uint32)virtual_address>=hard_limit+PAGE_SIZE && (uint32)virtual_address<=KERNEL_HEAP_MAX){
				// searching in the array for the VA of the page to its size
				struct allocated_together* my_pages = NULL;
				for(int i=0;i<ARR_SIZE;i++){
					if(pages_together[i].VA!=NULL && pages_together[i].VA==virtual_address)
						{
						  my_pages =&pages_together[i];
						  pages_together[i].VA = NULL;
						  break;
						 }
					}
				if(my_pages!= NULL) {
					//cprintf("PAGE SIZE IS %d \n",my_pages->size);
					struct FrameInfo * frame_ptr =NULL;

				for(int i=0;i<ROUNDUP(my_pages->size,PAGE_SIZE)/PAGE_SIZE;i++){
				uint32 *ptr_page_table=NULL;
				frame_ptr= get_frame_info(ptr_page_directory,(uint32)virtual_address+ i*PAGE_SIZE,&ptr_page_table);

				if(frame_ptr!=NULL){
				free_frame(frame_ptr);
				unmap_frame(ptr_page_directory, (uint32)virtual_address + i*PAGE_SIZE);

				}
				}
				//cprintf("Exit loop \n");
				}
			}else{
				panic("Invalid Address \n");
				release_sleeplock(&kernel_lock);
				return;
			}
	release_sleeplock(&kernel_lock);
	//cprintf("kfree done \n");
}


unsigned int kheap_physical_address(unsigned int virtual_address)
{
    //TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
    // Write your code here, remove the panic and write your code
    //  panic("kheap_physical_address() is not implemented yet...!!");

    //return the physical address corresponding to given virtual_address
    //refer to the project presentation and documentation for details

    //EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

    //cprintf("start of physical_address \n");
	//acquire_spinlock(&kernel_lock);
       if (virtual_address < KERNEL_HEAP_START || virtual_address >= KERNEL_HEAP_MAX){
    	   //release_spinlock(&kernel_lock);
          return 0;
       }

       uint32 *page_table = NULL;
       get_page_table(ptr_page_directory, virtual_address, &page_table);

       if (page_table == NULL)
       {
    	   //release_spinlock(&kernel_lock);
        return 0;
       }

      uint32 page_entry = page_table[PTX(virtual_address)];

     // Check if the page is present
     if (!(page_entry & PERM_PRESENT))
    {
    	 //release_spinlock(&kernel_lock);
     return 0; // Page is not mapped
    }

    uint32 frame = page_entry & 0xFFFFF000;
    uint32 offset = virtual_address & 0x00000FFF;
    //release_spinlock(&kernel_lock);
    //cprintf("End of physical_address \n");
    return frame|offset;

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
// Write your code here, remove the panic and write your code
//panic("kheap_virtual_address() is not implemented yet...!!");

//return the virtual address corresponding to given physical_address
//refer to the project presentation and documentation for details

//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//acquire_spinlock(&kernel_lock);

    struct FrameInfo* ptr_frame_info = NULL;
    ptr_frame_info = to_frame_info(physical_address);

    if (ptr_frame_info->references == 0) {
    	  //release_spinlock(&kernel_lock);
       return 0;
    }

    uint32 offset = physical_address & 0xFFF;
    uint32 dirIndex = ptr_frame_info->DirIndex << 22;
    uint32 pageTableIndex = ptr_frame_info->PageIndex << 12;

    uint32 VA = dirIndex|pageTableIndex|offset;

   if (VA < KERNEL_HEAP_START || VA >= KERNEL_HEAP_MAX){
	   //release_spinlock(&kernel_lock);
     return 0;
   }
   if( VA > hard_limit && VA < (hard_limit + PAGE_SIZE)){
	   //release_spinlock(&kernel_lock);
     return 0;
   }
   //release_spinlock(&kernel_lock);
    return VA;

}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//panic("krealloc() is not implemented yet...!!");
	if(virtual_address==NULL){
	  return kmalloc(new_size);
	}
	if(new_size==0){
		kfree(virtual_address);
	}
	else{
	//if its page allocated
	if((uint32)virtual_address>=hard_limit+PAGE_SIZE && (uint32)virtual_address<=KERNEL_HEAP_MAX){
		struct allocated_together* my_pages = NULL;
		int oldSize;
		for(int i=0;i<ARR_SIZE;i++){//looping over the list to find the old size
			if(pages_together[i].VA!=NULL && pages_together[i].VA==virtual_address){
				oldSize=pages_together[i].size;
				  break;
			  }
		}
		if(new_size<oldSize){//shrinking
			if(new_size>=DYN_ALLOC_MAX_BLOCK_SIZE){//still page allocated
			kfree(virtual_address);
			return kmalloc(new_size);

		 }
			else{//will be block allocated
				kfree(virtual_address);
				return alloc_block_FF(new_size);
			}

		   }
		else if(new_size>oldSize){//maximizing
			kfree(virtual_address);
			return kmalloc(new_size);
		}
	      }
	if((uint32)virtual_address>=KERNEL_HEAP_START && (uint32)virtual_address<=Break){//if its block allocated
		int oldSize = get_block_size(virtual_address);
		if(new_size<oldSize){//minimizing,then it still will be a block
			return realloc_block_FF(virtual_address,new_size);
		}
		else{//maximizing
			if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){//still will be a block
				return realloc_block_FF(virtual_address,new_size);
			}
			else {//will be page allocated
			return kmalloc(new_size);
			 }
	       }
		 }
	}
        return NULL;
	}
