#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit) {
    start = daStart;
    hard_limit = daLimit;

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
	cprintf("2.0 sbrk entered \n \n");

	cprintf("number of pages: %d \n \n", numOfPages);

	uint32 neededSize=numOfPages*PAGE_SIZE;
	int sizeAvailable=hard_limit-Break;

	if (Break + neededSize > hard_limit) {
	    cprintf("Requested size exceeds heap limit.\n");
	    return (void*)-1;
	}

	cprintf("Current Break: %d, Requested Pages: %d, Needed Size: %d\n", Break, numOfPages, neededSize);
	cprintf("Available Space: %d, Hard Limit: %d\n", hard_limit - Break, hard_limit);

	if(numOfPages==0){
		cprintf("number of pages is 0, original break is returned. \n");
		cprintf("2.1 sbrk return with numofpages=0 \n \n");
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
		    cprintf("allocating frame at iteration %d\n", i);

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
		    cprintf("DONE \n");

		}
		Break+=neededSize;
		cprintf("Current Break: %d,\n", Break);

//		struct BlockElement* lastFreeBlock = LIST_LAST(&freeBlocksList);
//	    cprintf("old last free block size: %d \n \n",get_block_size(LIST_LAST(&freeBlocksList)));

//			    lastFreeBlock = LIST_LAST(&freeBlocksList);
//			    cprintf("NEW last free block size: %d \n \n",get_block_size(lastFreeBlock));
		return (void*)prevBreak;

		}
		else{ //if the number of pages is less than 0
			cprintf("2.4 sbrk return at 129 \n \n");
			return (void*)-1 ;
		}
	}
	cprintf("2.5 returned with null at line 132\n \n");
	return (void*)-1;

//	//MS2: COMMENT THIS LINE BEFORE START CODING==========
//	//return (void*)-1 ;
//	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{

	 //TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	  // Write your code here, remove the panic and write your code
	  //kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	  // use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	  //page size = 4KB
	 if (size == 0 || size > (KERNEL_HEAP_MAX - KERNEL_HEAP_START)) {
		        cprintf("Invalid size for kmalloc: %u\n", size);
		        return NULL;
		    }
//		  if(size < DYN_ALLOC_MAX_BLOCK_SIZE){
//		  		  cprintf("ms1 alloc \n");
//		  		  void * ptr =alloc_block_FF(size);
//		  		  if(ptr==NULL)
//		  			  return NULL;
//		  		  return ptr;
//		  	  }

		    uint32 first_va_found = hard_limit+PAGE_SIZE;;
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
		        return NULL;
		    }

		    // Map frames to the virtual addresses
		    for (int i = 0; i < no_Of_required_pages; i++) {
		        struct FrameInfo *frame_info = NULL;
		        int ret = allocate_frame(&frame_info);

		        if (ret == E_NO_MEM || frame_info == NULL) {
		            cprintf("No memory for frame allocation\n");

		            // Cleanup already allocated frames
		            for (uint32 cleanup_va = first_va_found; cleanup_va < first_va_found + i * PAGE_SIZE; cleanup_va += PAGE_SIZE) {
		                unmap_frame(ptr_page_directory, cleanup_va);
		                free_frame(get_frame_info(ptr_page_directory, cleanup_va, NULL));
		            }
		            return NULL;
		        }

		        ret = map_frame(ptr_page_directory, frame_info, first_va_found + i * PAGE_SIZE, PERM_AVAILABLE | PERM_WRITEABLE);
		        if (ret == E_NO_MEM) {
		            cprintf("No memory for frame mapping\n");

		            // Cleanup already allocated frames
		            for (uint32 cleanup_va = first_va_found; cleanup_va < first_va_found + i * PAGE_SIZE; cleanup_va += PAGE_SIZE) {
		                unmap_frame(ptr_page_directory, cleanup_va);
		                free_frame(get_frame_info(ptr_page_directory, cleanup_va, NULL));
		            }

		            // Free the current frame
		            free_frame(frame_info);
		            return NULL;
		        }
		    }

		    return (void*)first_va_found;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
    //TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
    // Write your code here, remove the panic and write your code
//    panic("kheap_physical_address() is not implemented yet...!!");

    //return the physical address corresponding to given virtual_address
    //refer to the project presentation and documentation for details

    //EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

    uint32 dirIndex = PDX(virtual_address);
    uint32 pageTableIndex = PTX(virtual_address);
    uint32 offset = virtual_address & 0xFFF;
    //uint32 dirEntry=ptr_page_directory[dirIndex];
    uint32 *ptr=NULL;
    get_page_table(ptr_page_directory,virtual_address,&ptr);
    uint32 frameNO=ptr[pageTableIndex];
    cprintf("offset: %d \n \n", offset);

    cprintf("Virtual address : %d \n \n", virtual_address);

    cprintf("Physical address : %d \n \n",frameNO + offset);

    return frameNO + offset;

}
unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
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
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
