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

	int sizeNeeded=numOfPages*(int)PAGE_SIZE;
	int sizeAvailable=hard_limit-Break;

//	cprintf("size need for the pages: %d \n \n"+ sizeNeeded);
//	cprintf("where the break is: %d \n\n"+ (int)Break);
//	cprintf("where the hard_limit is: %d \n\n"+ hard_limit);
//	cprintf("The size between the break and hard_limit (sizeAvailable) %d \n\n"+ sizeAvailable);

//cprintf("number of pages: %d \n \n", numOfPages);




	if(numOfPages==0){
		cprintf("number of pages is 0, original break is returned. \n");
		cprintf("2.1 sbrk return at 99 \n \n");
		return (void*)Break;
	}
	else if(numOfPages>0 ){
		if( sizeNeeded<sizeAvailable){
		//if there is space for the new allocations
		uint32 prevBreak=Break;
		for(int i=0; i<numOfPages;i++){

			struct FrameInfo *ptr=NULL;

			int x=allocate_frame(&ptr); //allocation of the new frame
			if (x == E_NO_MEM){
				cprintf("2.2 sbrk return at 113 \n \n");
				panic("NO MEMORY ....");
				return (void*)-1;
			}

			int y = map_frame(ptr_page_directory,ptr,i,PERM_AVAILABLE|PERM_WRITEABLE); //mapping the allocated frame
			if (y == E_NO_MEM){
				cprintf("2.3 sbrk return at 120 \n \n");
				panic("NO MEMORY ....");
				return (void*)-1;
			}
		}
		Break+=sizeNeeded;
		//cprintf("this is where the break stands now: %d \n", Break);
		return (void*)prevBreak;
		}
		else{ //if the number of pages is less than 0
			cprintf("2.4 sbrk return at 129 \n \n");
			return (void*)-1 ;
		}
	}
	cprintf("2.5 returned with null at line 132\n \n");
	return (void*)-1;

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	 //TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	  // Write your code here, remove the panic and write your code
	  //kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	  // use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	  //page size = 4KB
	if(isKHeapPlacementStrategyFIRSTFIT()==0)
		return NULL;
//	  uint32 hard_limit=0;
	  uint32 first_va_found=hard_limit+4;
	  int pagesCounter=0;
	  if(size < DYN_ALLOC_MAX_BLOCK_SIZE){
	    return alloc_block_FF(size);
	  }
	  else{
	    int no_Of_required_pages = ROUNDUP(size/4,1);
	    //found a free frame to allocate
	      while(first_va_found<KERNEL_HEAP_MAX){
	      uint32 *page_table;
	      struct FrameInfo *frame = get_frame_info(ptr_page_directory,first_va_found,&page_table);
	      if(frame==NULL){
	    	  pagesCounter++;
	       if(pagesCounter==no_Of_required_pages){
	    	   first_va_found-=no_Of_required_pages*PAGE_SIZE;
	    	   break;
	       }
	      }
	      else{
	    	  pagesCounter=0;
	      }
	      first_va_found+=PAGE_SIZE;
	    }//mapping to frames
	      for(int i=0;i<no_Of_required_pages;i++){
	         	  struct FrameInfo *frame_info =NULL;
	         	  int ret = allocate_frame(&frame_info);
	         	  if(ret==E_NO_MEM){//no free frame
	         	       cprintf("no memory");
	         	      return NULL;
	         	     }
	         	  else{
	     		 int ret1 = map_frame(ptr_page_directory,frame_info,first_va_found,PERM_AVAILABLE | PERM_WRITEABLE);//hanshofha tany
	     		 if(ret1==E_NO_MEM){
	     			 return NULL;
	     		 }
	     		 first_va_found+= PAGE_SIZE;
	     		   }
	            }

	      return (uint32 *)(first_va_found-no_Of_required_pages*PAGE_SIZE);
	  }


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
//    panic("kheap_virtual_address() is not implemented yet...!!");

    //return the virtual address corresponding to given physical_address
    //refer to the project presentation and documentation for details

    //EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

    uint32 offset = physical_address & 0xFFF;
    struct FrameInfo* ptr_frame_info = to_frame_info(physical_address);

    uint32 dirIndex = ptr_frame_info->DirIndex;
    uint32 pageTableIndex = ptr_frame_info->PageIndex;

    return dirIndex + pageTableIndex + offset;

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
