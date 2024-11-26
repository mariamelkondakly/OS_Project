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


#define U_ARR_SIZE ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE)
struct allocatedtogether{
		uint32 size;
		void* VA;
};
struct allocatedtogether Allpages[U_ARR_SIZE];


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
//	panic("malloc() is not implemented yet...!!");
	//cprintf("entered malloc with size %d \n ",size);

	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		//cprintf("ms1 alloc \n");
		void * ptr = alloc_block_FF(size);
		//cprintf("5alasha ? \n");

		if(ptr==NULL)
			return NULL;

		return ptr;
	}

	uint32 start_page_alloc = myEnv->hard_limit+PAGE_SIZE;
	uint32 first_va_found = myEnv->hard_limit+PAGE_SIZE;
	int no_Of_required_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	int pagesCounter = 0;
	bool found = 0;

	// Find contiguous free pages
	int alloc = 0;
	while(first_va_found<USER_HEAP_MAX){
		alloc = isAddressAllocated(first_va_found);
		if(alloc == 1){
			pagesCounter = 0;
			first_va_found+=PAGE_SIZE;
			continue;
		}

		pagesCounter++;

		if(pagesCounter==no_Of_required_pages){
			first_va_found -= (no_Of_required_pages - 1) * PAGE_SIZE;
			break;
		}

		first_va_found +=PAGE_SIZE;
	}

	if (pagesCounter < no_Of_required_pages) {
		cprintf("Not enough contiguous space in User heap\n");
		return NULL;
	}

	for(int i=0;i<U_ARR_SIZE;i++){
		if(Allpages[i].VA==NULL){
			Allpages[i].VA = (void*)first_va_found;
			Allpages[i].size = size;
			break;
		}
	}

	markAddressRangeAsAllocated(first_va_found, no_Of_required_pages);
	sys_allocate_user_mem(first_va_found,size);
	return (void*)first_va_found;

	//return NULL;
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
	//panic("free() is not implemented yet...!!");

	if((uint32)virtual_address>=USER_HEAP_START && (uint32)virtual_address<=(myEnv->Break)){
		free_block(virtual_address);
	}

	else if((uint32)virtual_address>=(myEnv->hard_limit)+PAGE_SIZE && (uint32)virtual_address<USER_HEAP_MAX){

//		struct allocatedtogether* my_pages = NULL;
//		for(int i=0;i<U_ARR_SIZE;i++){
//			if(Allpages[i].VA!=NULL && Allpages[i].VA==virtual_address)
//			{
//				my_pages = (struct allocatedtogether*)&(Allpages[i]);
//				Allpages[i].VA = NULL;
//				my_pages->size=0;
//				break;
//			}
//		}
//		if(my_pages!= NULL) {
//			sys_free_user_mem((uint32)virtual_address,my_pages->size);
//		}

		uint32 VA;
		int size;

		for(int i=0;i<U_ARR_SIZE;i++){

//			cprintf("VA: %d\n", Allpages[i].VA);
//			cprintf("size: %d\n\n\n", Allpages[i].size);

			if(Allpages[i].VA==(uint32*)virtual_address){

				size = Allpages[i].size;

				Allpages[i].VA = NULL;
				Allpages[i].size = 0;
				break;
			}

		}

		int no_Of_required_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		VA = (uint32) virtual_address;

		markAddressRangeAsFree(VA, no_Of_required_pages);
		sys_free_user_mem(VA,size);
	}

	else{
		panic("Invalid Address \n");
		return;
	}
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");
	return NULL;
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
