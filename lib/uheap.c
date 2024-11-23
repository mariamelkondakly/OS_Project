#include <inc/lib.h>



//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define USER_HEAP_MAX_PAGES ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE)
static uint8 userHeapBitmap[USER_HEAP_MAX_PAGES] = {0}; // Bitmap to track allocations

int isAddressAllocated(uint32 address) {
    uint32 pageIndex = ROUNDUP((address - USER_HEAP_START),PAGE_SIZE) / PAGE_SIZE;
    return userHeapBitmap[pageIndex];
}

void markAddressRangeAsAllocated(uint32 startAddress, int numOfPages) {
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = ROUNDUP((startAddress - USER_HEAP_START),PAGE_SIZE) / PAGE_SIZE + i;
        userHeapBitmap[pageIndex] = 1;
    }
}

void markAddressRangeAsFree(uint32 startAddress, int numOfPages) {
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = (startAddress - USER_HEAP_START) / PAGE_SIZE + i;
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
//void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
//{
//	//==============================================================
//	//DON'T CHANGE THIS CODE========================================
//	if (size == 0) return NULL ;
//	//==============================================================
//	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
//	// Write your code here, remove the panic and write your code
//	//panic("smalloc() is not implemented yet...!!");
//	 if (size > (USER_HEAP_MAX - USER_HEAP_START)) {
//		return NULL;
//	}
//	uint32 first_va_found = myEnv->hard_limit+PAGE_SIZE;
//	int numOfPagesNeeded= ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
//    int pagesCounter = 0;
//
//	while (first_va_found < USER_HEAP_MAX) {
////		uint32 *page_table=NULL;
////		struct FrameInfo *frame = get_frame_info(myEnv->env_page_directory, first_va_found, &page_table);
//		uint32 permissions= pt_get_page_permissions((uint32*)myEnv,  first_va_found );
//		if (!(permissions&&PERM_PRESENT)) {
//			pagesCounter++;
//			if (pagesCounter == numOfPagesNeeded) {
//				first_va_found -= (numOfPagesNeeded - 1) * PAGE_SIZE;
//				break;
//			}
//		}else {
//            pagesCounter = 0;
//        }
//
//        first_va_found += PAGE_SIZE;
//    }
//
//    if (pagesCounter < numOfPagesNeeded) {
//        cprintf("Not enough contiguous space in kernel heap\n");
//        return NULL;
//    }
//
//    int x=sys_createSharedObject(sharedVarName,size,isWritable,(void*)first_va_found);
//    if(x==E_NO_SHARE||x==E_SHARED_MEM_EXISTS||x==E_NO_MEM){
//    	return NULL;
//    }
//	return (void*)first_va_found;
//}
void* smalloc(char* sharedVarName, uint32 size, uint8 isWritable)
{
    //==============================================================
    // DON'T CHANGE THIS CODE
    if (size == 0) return NULL;
    //==============================================================

    // Check if the size exceeds the available heap space
    if (size > (USER_HEAP_MAX - USER_HEAP_START)) {
        return NULL;
    }

    uint32 first_va_found = myEnv->hard_limit + PAGE_SIZE; // Start searching after the hard limit
    int numOfPagesNeeded = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
    int pagesCounter = 0;

    // Iterate through user heap to find contiguous free space
    while (first_va_found < USER_HEAP_MAX) {
        // Check if the current address is free using a custom tracker (e.g., a bitmap or your own metadata)
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

    // If no suitable space is found
    if (pagesCounter < numOfPagesNeeded) {
        cprintf("Not enough contiguous space in user heap\n");
        return NULL;
    }

    // Use sys_createSharedObject to create the shared memory object
    int status = sys_createSharedObject(sharedVarName, size, isWritable, (void*)first_va_found);
    if (status == E_NO_SHARE || status == E_SHARED_MEM_EXISTS || status == E_NO_MEM) {
        return NULL; // Handle shared memory creation errors
    }

    // Mark the allocated memory as used in your tracker
    markAddressRangeAsAllocated(first_va_found, numOfPagesNeeded);

    return (void*)first_va_found;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
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
