#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define USER_HEAP_MAX_PAGES ((USER_HEAP_MAX - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)) / PAGE_SIZE)
static uint8 userHeapBitmap[USER_HEAP_MAX_PAGES] = {0}; // Bitmap to track allocations

int isAddressAllocated(uint32 address) {
    uint32 pageIndex = ROUNDUP((address - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)),PAGE_SIZE) / PAGE_SIZE;
    return userHeapBitmap[pageIndex];
}

void markAddressRangeAsAllocated(uint32 startAddress, int numOfPages) {
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = ROUNDUP((startAddress - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)),PAGE_SIZE) / PAGE_SIZE + i;
        userHeapBitmap[pageIndex] = 1;
    }
}

void markAddressRangeAsFree(uint32 startAddress, int numOfPages) {
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = (startAddress - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE))/ PAGE_SIZE + i;
        userHeapBitmap[pageIndex] = 0;
    }
}
//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	panic("malloc() is not implemented yet...!!");
	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	panic("free() is not implemented yet...!!");
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char* sharedVarName, uint32 size, uint8 isWritable)
{
    //==============================================================
    // DON'T CHANGE THIS CODE
    if (size == 0) return NULL;
    //==============================================================

    // Check if the size exceeds the available heap space
    if (size > (USER_HEAP_MAX - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE))) {
        return NULL;
    }

    //uint32 first_va_found = myEnv->hard_limit + PAGE_SIZE; // Start searching after the hard limit
	uint32 first_va_found = USER_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE; //UHS + 32MB + 4KB

    int numOfPagesNeeded = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
    int pagesCounter = 0;
    bool found = 0;


    while (first_va_found < USER_HEAP_MAX) {
        if (!isAddressAllocated(first_va_found)) {
            pagesCounter++;
            if (pagesCounter == numOfPagesNeeded) {
                // Calculate the start of the contiguous block
                first_va_found -= (numOfPagesNeeded - 1) * PAGE_SIZE;
                break;
            }
        } else {
            pagesCounter = 0; // Reset if the block is not contiguous
        }

        first_va_found += PAGE_SIZE;
    }
//	 while(first_va_found<USER_HEAP_MAX){
//		 for(int i=0;i<U_ARR_SIZE;i++){
//			 if(Allpages[i].VA!=NULL){
//				 if(Allpages[i].VA==(void*)first_va_found){
//					 pagesCounter=0;
//					 first_va_found += ROUNDUP(Allpages[i].size,PAGE_SIZE);
//					 found=1;
//					 break;
//				 }
//			 }
//		 }
//		 if (found) {
//		   continue;
//		 }
//
//		 pagesCounter++;
//		 if(pagesCounter==numOfPagesNeeded){
//			first_va_found -= (numOfPagesNeeded - 1) * PAGE_SIZE;
//			break;
//		 }
//		 first_va_found +=PAGE_SIZE;
//	 }


    if (pagesCounter < numOfPagesNeeded) {
           cprintf("Not enough contiguous space in kernel heap\n");
           return NULL;
       }

       int x=sys_createSharedObject(sharedVarName,size,isWritable,(void*)first_va_found);
       if(x==E_NO_SHARE||x==E_SHARED_MEM_EXISTS||x==E_NO_MEM){
       	return NULL;
       }
       markAddressRangeAsAllocated(first_va_found, numOfPagesNeeded);
//       ((uint32*)first_va_found)[0]=-1;
//       cprintf("what is in that location: %d \n",((uint32*)first_va_found)[0]);
       //cprintf("expected va: %d \n",first_va_found);

   	return (void*)first_va_found;
}


//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	int size= sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(size==E_SHARED_MEM_NOT_EXISTS){
		return NULL;
	}
	uint32 first_va_found = USER_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE; //UHS + 32MB + 4KB

	    int numOfPagesNeeded = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	    int pagesCounter = 0;
	    bool found = 0;


	    while (first_va_found < USER_HEAP_MAX) {
	        if (!isAddressAllocated(first_va_found)) {
	            pagesCounter++;
	            if (pagesCounter == numOfPagesNeeded) {
	                // Calculate the start of the contiguous block
	                first_va_found -= (numOfPagesNeeded - 1) * PAGE_SIZE;
	                break;
	            }
	        } else {
	            pagesCounter = 0; // Reset if the block is not contiguous
	        }

	        first_va_found += PAGE_SIZE;
	    }
	    if (pagesCounter < numOfPagesNeeded) {
	               cprintf("Not enough contiguous space in kernel heap\n");
	               return NULL;
	           }
	    int x=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)first_va_found);
	    if(x==E_NO_SHARE||x==E_NO_MEM){
	           	return NULL;
	     }
	    markAddressRangeAsAllocated(first_va_found, numOfPagesNeeded);


	return (void*)first_va_found;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
