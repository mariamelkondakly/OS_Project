#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"




#include<kern/conc/sleeplock.h>
// edited lala
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
// Made lock
	struct sleeplock Myshareslock;
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");

	init_sleeplock(&Myshareslock, "My shares lock");

#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

	struct FrameInfo** frames=(struct FrameInfo**)kmalloc(numOfFrames*sizeof(struct FrameInfo*));
	for(int i=0; i<numOfFrames;i++){
		frames[i]=NULL;
	}

	if(frames==NULL){
		panic("Failed to allocate memory for frames storage");
		return NULL;
	}
	return (struct FrameInfo**)frames;

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...


	struct Share* shareObject=(struct Share*) kmalloc(sizeof(struct Share));
	if (!shareObject) {
    //cprintf("exited create_share at shareObject =NULL\n");
	        return NULL;  // Memory allocation failed
	    }
	uint32 mask = 0x7FFFFFFF;
	uint32 maskedVA=((uint32)shareObject&mask);

	shareObject->framesStorage=create_frames_storage(ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE);
	if (!shareObject->framesStorage) {
       //cprintf("exited create_share at framesStorage =NULL\n");
		kfree(shareObject);
		return NULL;
	}

    strcpy(shareObject->name, shareName);
	shareObject->ownerID=ownerID;
	shareObject->references=1;
	shareObject->isWritable=isWritable;
	shareObject->ID=maskedVA;
	shareObject->size=size;

    //cprintf("exited create_share normally");

	return shareObject;

}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...
	acquire_spinlock(&AllShares.shareslock);
	struct Share* current;
	int i=1;

	LIST_FOREACH(current,&(AllShares.shares_list)){

    //cprintf("%d share's name: %s, share's ownerId: %d \n", i,current->name,current->ownerID);
		i++;
		bool isFound=!(strncmp(name, current->name, strlen(name)))&&current->ownerID==ownerID;
    //cprintf("is found = %d \n",isFound);
	//cprintf("current: %d \n", (uint32)current);

		if(isFound){
     //cprintf("current: %d \n", (uint32)current);
			release_spinlock(&AllShares.shareslock);
			return current;
		}
	}
	release_spinlock(&AllShares.shareslock);
	return NULL;

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...

   //cprintf("\n entered createSharedObject\n \n");


	struct Env* myenv = get_cpu_proc(); //The calling environment
	acquire_sleeplock(&Myshareslock);

	struct Share* found=get_share(ownerID,shareName);


	if(found!=NULL){
    //cprintf("exited createSharedObject with the shared memory already exists \n");
		release_sleeplock(&Myshareslock);
		return E_SHARED_MEM_EXISTS;
	}
	struct Share* sharedObject= create_share(ownerID, shareName,size,isWritable);

	if(!sharedObject){
    //cprintf("exited createSharedObject at no memory to create shared object\n");
		release_sleeplock(&Myshareslock);
		return E_NO_SHARE;
	}

	int noOfPagesNeeded=ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;

	int frameStorageIndex=0;
	for(int i=(uint32)virtual_address; i<((uint32)virtual_address+ROUNDUP(size, PAGE_SIZE));i+=PAGE_SIZE){
		struct FrameInfo* ptr;
		int x=allocate_frame(&ptr);

		if(x==E_NO_MEM){
			kfree(sharedObject);

			for (uint32 k = (uint32)virtual_address; k < i; k += PAGE_SIZE) {
				unmap_frame(myenv->env_page_directory, k);
				free_frame(get_frame_info(myenv->env_page_directory, k, NULL));
			}
			release_sleeplock(&Myshareslock);

		return E_NO_SHARE;
		}
//		cprintf("before mapping frame! %x \n",i);

		x=map_frame(myenv->env_page_directory, ptr, (uint32)i, PERM_WRITEABLE);
		pt_set_page_permissions(myenv->env_page_directory, (uint32)i,PERM_USER,0);
//		cprintf("after mapping frame! %x \n",i);


		if(x==E_NO_MEM){
			kfree(sharedObject);

			for (uint32 k = (uint32)virtual_address; k < ((uint32)virtual_address+ROUNDUP(size, PAGE_SIZE)); k += PAGE_SIZE) {
				unmap_frame(myenv->env_page_directory, k);
				free_frame(get_frame_info(myenv->env_page_directory, k, NULL));
			}
			release_sleeplock(&Myshareslock);
			return E_NO_SHARE;
		}

//		uint32 pd_used;
//		pd_used =pd_is_table_used(myenv->env_page_directory,(uint32)i);
//		cprintf("is page table used after allocation? %d \n", pd_used);

		sharedObject->framesStorage[frameStorageIndex]=ptr;
		frameStorageIndex++;

	}
    //cprintf("frame storage hold %d in creation \n", frameStorageIndex);
	acquire_spinlock(&AllShares.shareslock);
	LIST_INSERT_TAIL(&AllShares.shares_list, sharedObject);
	release_spinlock(&AllShares.shareslock);
    //cprintf("is the sharedObject writeable? %d \n",sharedObject->isWritable);
    //cprintf("exited createSharedObject normally\n");
    //((uint32*)virtual_address)[0]=-1;
    //cprintf("trial 1: %d \n", ((uint32*)virtual_address)[0]);
	int32 id =sharedObject->ID;
	release_sleeplock(&Myshareslock);
	return id;

}



//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//cprintf("entered getSharedObject! \n");
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...


	    struct Env* myenv = get_cpu_proc(); //The calling environment
	    acquire_sleeplock(&Myshareslock);
	    //cprintf("before the lock! \n");


	    struct Share* x= get_share(ownerID,shareName);
		//cprintf("after the lock! share found at: %d \n",(uint32) x);



			if(x==NULL){
				//cprintf("exited with the share not found \n");
				release_sleeplock(&Myshareslock);
					return E_NO_SHARE ;
				}
			struct FrameInfo** frames =x->framesStorage;
			int index=0;
	    	 //cprintf("starting to map! \n");
			int ret;
	     while(index<(ROUNDUP(x->size,PAGE_SIZE)/PAGE_SIZE)){
	    	 //cprintf("we're mapping! \n");
	    	 if(x->isWritable){
	    	 ret=map_frame(myenv->env_page_directory, frames[index], (uint32)virtual_address+(index*PAGE_SIZE),PERM_WRITEABLE );
	    	 }else{
		     ret=map_frame(myenv->env_page_directory, frames[index], (uint32)virtual_address+(index*PAGE_SIZE),0 );
	    	 }

	 		pt_set_page_permissions(myenv->env_page_directory, (uint32)virtual_address+(index*PAGE_SIZE),PERM_USER,0);
	 		//cprintf("just mapped with result %d \n", ret);

	    			if(ret==E_NO_MEM){
	    				 int index2=0;
	    				 while(index2!=index){
	    				 unmap_frame(myenv->env_page_directory,(uint32)virtual_address+(index2*PAGE_SIZE) );
	    				 index2++;

	    				 }
	    				//cprintf("exited with no memory found for mapping\n");
	    				 release_sleeplock(&Myshareslock);
	    				return E_NO_MEM;
	    					}




	    index++ ;
	     }

	     x->references++;
	     //cprintf("EXITING GETSHAREDOBJECT NORMALLY!\n");
	     int32 id = x->ID;
	     release_sleeplock(&Myshareslock);
		 return id;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	LIST_REMOVE(&AllShares.shares_list, ptrShare);
	int index=0;
//	while(index<(ROUNDUP(ptrShare->size,PAGE_SIZE)/PAGE_SIZE)){
//			free_frame(ptrShare->framesStorage[index]);
//			index++;
//	}
	index=0;
	while(index<(ROUNDUP(ptrShare->size,PAGE_SIZE)/PAGE_SIZE)){
		ptrShare->framesStorage[index]=NULL;
		index++;

	}

	kfree(ptrShare->framesStorage);
	kfree(ptrShare);


}
//========================
// [B2] Free Share Object:
//========================

int isTableUsed(uint32* ptr_page_table){
	bool pd_used=1;
	for(int j=(uint32)ptr_page_table; j<(PAGE_SIZE+(uint32)ptr_page_table);j+=sizeof(ptr_page_table)){
//			cprintf("the page table pointer points at: %x \n",i);
		if((uint32*)*((uint32*)j)!=NULL){
			pd_used=1;
			break;
		}
		else{
			pd_used=0;
		}
	}
	return pd_used;
}

int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment
//	cprintf("the startVA: %x \n", startVA);
    acquire_spinlock(&AllShares.shareslock);
	struct Share* current;
	bool found=0;
	LIST_FOREACH(current,&(AllShares.shares_list)){
		if(current->ID==sharedObjectID){
			found=1;
			break;
		}

	}

	if(!found){
		return E_NO_SHARE;
	}
	current->references--;

	if(current->references==0){
		uint32 *ptr_page_table =NULL;
		struct FrameInfo* ptr =NULL;
		bool pd_used=1;

		//unmapping the shared object's frames 1st
//		for (uint32 i = (uint32)startVA; i < ((uint32)startVA+ROUNDUP(current->size, PAGE_SIZE)); i += PAGE_SIZE) {
//			ptr =get_frame_info(myenv->env_page_directory, (uint32)i, &ptr_page_table);
//			if(ptr!= NULL){
////							free_frame(ptr);
//				cprintf("while unmapping the VA itself, startVA: %x , page table pointer: %x \n",i, ptr_page_table);
//			}
//		}

		for (uint32 i = (uint32)startVA; i < ((uint32)startVA+ROUNDUP(current->size, PAGE_SIZE)); i += PAGE_SIZE) {
			ptr =get_frame_info(myenv->env_page_directory, (uint32)i, &ptr_page_table);

//			cprintf("startVA: %x , page table pointer: %x \n",i, ptr_page_table);
			if(ptr!= NULL){
				unmap_frame(myenv->env_page_directory, i);
			}

//			cprintf("is pd_used? %d \n", pd_used);
	//		pd_used =pd_is_table_used(myenv->env_page_directory,i);
			if(!isTableUsed(ptr_page_table)){
				if(ptr_page_table!=NULL&&ptr_page_table!=0){
//				    cprintf("PAGE TABLE DELETED HERE! \n");
					pd_clear_page_dir_entry(myenv->env_page_directory,(uint32)i);

					unmap_frame(myenv->env_page_directory,(uint32) ptr_page_table);
				}
			}
		}
		free_share(current);

	}
	else{
//		cprintf("no of refs to shared object: %d \n", current->references);
		for (uint32 i = (uint32)startVA; i < ((uint32)startVA+ROUNDUP(current->size, PAGE_SIZE)); i += PAGE_SIZE) {
//			cprintf("unmapped here!\n");
			unmap_frame(myenv->env_page_directory, i);
		}

	}
	//tlb_invalidate(myenv->env_page_directory,(uint32)startVA);

	tlbflush();
    release_spinlock(&AllShares.shareslock);


	return 0;

}
