#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define USER_HEAP_MAX_PAGES ((USER_HEAP_MAX - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)) / PAGE_SIZE)
static uint8 userHeapPages[USER_HEAP_MAX_PAGES] = {0}; // to track allocations


int isAddressAllocated(uint32 address) { //checks if the page is allocated or not (used to allocate by first fit)
    uint32 pageIndex = ROUNDUP((address - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)),PAGE_SIZE) / PAGE_SIZE;
    return userHeapPages[pageIndex];
}

void markAddressAsAllocated(uint32 startAddress, int numOfPages) { //marks the pages as allocated
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = ROUNDUP((startAddress - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE)),PAGE_SIZE) / PAGE_SIZE + i;
        userHeapPages[pageIndex] = 1;
    }
}

void markAddressAsFree(uint32 startAddress, int numOfPages) { //marks the pages as free
    for (int i = 0; i < numOfPages; i++) {
        uint32 pageIndex = (startAddress - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE))/ PAGE_SIZE + i;
        userHeapPages[pageIndex] = 0;
    }
}

#define U_ARR_SIZE ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE)
struct allocatedtogether{
		uint32 size;
		void* VA;
};
struct allocatedtogether Allpages[U_ARR_SIZE];
struct sharedBundle{
	int32 ID ;
	void* VA;
	uint32 size;
};
struct sharedBundle sharedBundles[USER_HEAP_MAX_PAGES];


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
    //panic("malloc() is not implemented yet...!!");

	//cprintf("entered malloc with size %d \n ",size);

	//Block Allocator:
	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		//cprintf("ms1 alloc \n");
		void * ptr = alloc_block_FF(size);
		//cprintf("5alasha ? \n");

		if(ptr==NULL)
			return NULL;

		return ptr;
	}
	//Page Allocator:
	uint32 start_page_alloc = myEnv->hard_limit+PAGE_SIZE;
	uint32 first_va_found = myEnv->hard_limit+PAGE_SIZE;
	int no_Of_required_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	int pagesCounter = 0;
	bool found = 0;

	// Find contiguous free pages
	int alloc = 0;
	while(first_va_found<USER_HEAP_MAX){
		//checking for free pages first fit
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
    //putting the allocated page in the array to have its size with its VA
	for(int i=0;i<U_ARR_SIZE;i++){
		if(Allpages[i].VA==NULL){
			Allpages[i].VA = (void*)first_va_found;
			Allpages[i].size = size;
			break;
		}
	}
    //mark them
	markAddressAsAllocated(first_va_found, no_Of_required_pages);
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

   //Block Allocator:
	if((uint32)virtual_address>=USER_HEAP_START && (uint32)virtual_address<=(myEnv->Break)){
		free_block(virtual_address);
	}

	//Page Allocator:
	else if((uint32)virtual_address>=(myEnv->hard_limit)+PAGE_SIZE && (uint32)virtual_address<USER_HEAP_MAX){

		uint32 VA;
		int size;

        //searching for the size of this VA in the array
		for(int i=0;i<U_ARR_SIZE;i++){

        //cprintf("VA: %d\n", Allpages[i].VA);
        //cprintf("size: %d\n\n\n", Allpages[i].size);

			if(Allpages[i].VA==(uint32*)virtual_address){

				size = Allpages[i].size;

				Allpages[i].VA = NULL;
				Allpages[i].size = 0;
				break;
			}

		}

		int no_Of_required_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		VA = (uint32) virtual_address;
        //mark it as free
		markAddressAsFree(VA, no_Of_required_pages);
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
void* smalloc(char* sharedVarName, uint32 size, uint8 isWritable)
{
    //==============================================================
    // DON'T CHANGE THIS CODE
    if (size == 0) return NULL;
    //==============================================================
    //TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
    // Write your code here, remove the panic and write your code
    //panic("smalloc() is not implemented yet...!!");

    // Check if the size exceeds the available heap space
    if (size > (USER_HEAP_MAX - (USER_HEAP_START+ DYN_ALLOC_MAX_SIZE+PAGE_SIZE))) {
    	cprintf("exceeds ava space \n");
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

    if (pagesCounter < numOfPagesNeeded) {
           cprintf("Not enough contiguous space in kernel heap\n");
           return NULL;
       }
       int x=sys_createSharedObject(sharedVarName,size,isWritable,(void*)first_va_found);
       if(x==E_NO_SHARE||x==E_SHARED_MEM_EXISTS){//||x==E_NO_MEM
    	   cprintf("no mem \n");
       	return NULL;
       }
       markAddressAsAllocated(first_va_found, numOfPagesNeeded);
       for(int i=0; i<USER_HEAP_MAX_PAGES;i++){
			   if(sharedBundles[i].VA==NULL){
			   sharedBundles[i].VA=(void*)first_va_found;
			   sharedBundles[i].ID=x;
			   sharedBundles[i].size=size;
			   break;
		   }
		 }
//       ((uint32*)first_va_found)[0]=-1;
//       cprintf("what is in that location: %d \n",((uint32*)first_va_found)[0]);
       //cprintf("expected va: %d \n",first_va_found);
//       ((uint32*)first_va_found)[0]=-1;
//       cprintf("trial 1: %d \n", ((uint32*)first_va_found)[0]);
   	return (void*)first_va_found;

}


//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName){
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

//	cprintf("entered sget! \n");
//	cprintf("ownerEnvId: %d, sharedVarName: %s \n", ownerEnvID, sharedVarName);
	int size= sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(size==E_SHARED_MEM_NOT_EXISTS){
		cprintf("sget exited at E_SHARED_MEM_NOT_EXISTS \n");
		return NULL;
	}

	uint32 first_va_found = USER_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE; //UHS + 32MB + 4KB
	//cprintf("set the first_va_found first place %d \n", first_va_found);
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
		//cprintf("the first_va_found found %d \n", first_va_found);

	    if (pagesCounter < numOfPagesNeeded) {
	               cprintf("Not enough contiguous space in kernel heap\n");
	               return NULL;
	           }
        //cprintf("before calling getSharedObject\n");

	    int x=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)first_va_found);
        //cprintf("after calling getSharedObject\n");

	    if(x==E_NO_SHARE||x==E_NO_MEM){
            cprintf("get shared object being mean\n");

	           	return NULL;
	     }

	    markAddressAsAllocated(first_va_found, numOfPagesNeeded);
	    for(int i=0; i<USER_HEAP_MAX_PAGES;i++){
		   if(sharedBundles[i].VA==NULL){
		   sharedBundles[i].VA=(void*)first_va_found;
		   sharedBundles[i].ID=x;
		   sharedBundles[i].size=size;
		   break;
	   }
	  }


	    //cprintf("here is the virtual address returned: %d \n", (uint32)first_va_found);
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
	//panic("sfree() is not implemented yet...!!");

	int32 ID;
	uint32 size =0;
	bool found=0;
	for(int i=0;i<USER_HEAP_MAX_PAGES;i++){
//			cprintf("this is the current va in the array at %d : %x \n", i, sharedBundles[i].VA);
			found=(uint32)sharedBundles[i].VA==(uint32)virtual_address;
			if(found){
				ID=sharedBundles[i].ID;
				size = sharedBundles[i].size;
				break;
			}
		}
		if(found){
			markAddressAsFree((uint32)virtual_address,ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE);
//			cprintf("found the shared object, va: %x id: %x \n", virtual_address, ID);
			sys_freeSharedObject(ID,virtual_address);
//			cprintf("returned from freeSharedObject \n");
			for(int i=0;i<=USER_HEAP_MAX_PAGES;i++){
//				cprintf("looking for the deleted object \n");
				if((uint32)sharedBundles[i].VA==(uint32)virtual_address){
//					cprintf("successfully found the object \n");

					sharedBundles[i].ID=0;
					sharedBundles[i].VA=NULL;
					sharedBundles[i].size=0;
					break;
				}
			}

			}

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
