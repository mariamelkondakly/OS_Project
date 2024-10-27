/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
#include <inc/queue.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;//goes back to header pointer
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...

	uint32* BEG = (uint32*)daStart;
	*BEG = 1;
	uint32* END = (uint32*)(daStart + initSizeOfAllocatedSpace - 4);
	*END = 1;

	struct BlockElement* block = (struct BlockElement*)(daStart + 8);//points to va
	uint32* header = (uint32*)(daStart + 4);
	*header = initSizeOfAllocatedSpace - 8;
	uint32* footer = (uint32*)(daStart + initSizeOfAllocatedSpace - 8);
	*footer = initSizeOfAllocatedSpace - 8;

	LIST_INIT(&freeBlocksList);

	LIST_INSERT_HEAD(&freeBlocksList, block);


//	    uint32* va = header;
//
//		set_block_data( va, 30, 0);
//		int sizee = get_block_size(va);
//		int8 check = is_free_block(va);
//		cprintf("va = %x \n",va);
//		cprintf("check if free: %d \n", check);
//		cprintf("size: %d\n", sizee);

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("set_block_data is not implemented yet");
	//Your Code is Here...
	if(totalSize%2!=0){
		totalSize++;
	}
	uint32* header = (uint32*)((uint32) va - 4);//edited
			//cprintf("header: %p \n", header);
			uint32* footer = (uint32*)( (uint32)va + totalSize - 8);
			//cprintf("footer: %p \n", footer);
			if(isAllocated == 0) {
				*header = totalSize;
				*footer = totalSize;
			}
			else{
				//cprintf("isAllocated == 1\n");
				*header = totalSize + 1;
				*footer = totalSize + 1;
				//cprintf("header value: %d \n", *header);
			}

}

//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
	uint32 total_size = size + 8; // Adjust for metadata

	    if (total_size < 16) {
	        cprintf("Size must be >= 16");
	        return NULL;
	    }
	    struct BlockElement* current;//va
	    LIST_FOREACH(current, &freeBlocksList) {
	        //hwa hyfdal ylf l7ad ma next yb2a b null
	    	uint32 current_size=get_block_size(current);
	        if (current_size >= total_size) {
	        	uint32 size_diff = (current_size - total_size);
	            if (size_diff >= 16) {
	                // Split the block
	                struct BlockElement* free_block = (struct BlockElement*)((uint32)current + total_size); // edited is this va
	                set_block_data(current,total_size , 1);
	                set_block_data(free_block, (current_size- total_size), 0); // New block size , splitted correct
	                LIST_INSERT_AFTER(&freeBlocksList, current, free_block);
	                LIST_REMOVE(&freeBlocksList, current); // Remove from free list
                    return current;
	            } else {
	                // Internal fragmentation
	                set_block_data(current, current_size, 1); // Just mark current block as used
	                //cprintf("current size when size difference <16 : %d\n",current_size);
	                LIST_REMOVE(&freeBlocksList, current);
	                return current;
	            }
	        }
	    }
	    sbrk(total_size);
	    return NULL; // No suitable block found

}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
    //Your Code is Here...
if (size == 0) {
			cprintf("Size cant be 0");
			return NULL;
			}
	uint32 total_size = size + 8; // Adjust for metadata
	if(total_size<16)
		total_size=16;

	uint32 min_size_diff = 4294967295;
	struct BlockElement* current;//va
	struct BlockElement* best_block = NULL;//va
	uint32 best_size =0;
	LIST_FOREACH(current, &freeBlocksList) {
		uint32 current_size=get_block_size(current);
		if (current_size >= total_size) {
			uint32 size_diff = (current_size - total_size);
			if(size_diff<min_size_diff){
			min_size_diff=size_diff;
			best_block=current;
			best_size=current_size;
			}
		}
	}
	if(best_block!=NULL){
			if (min_size_diff >= 16) {
			// Split the block

			struct BlockElement* free_block = (struct BlockElement*)((uint32)best_block + total_size); // edited is this va
			set_block_data(free_block, best_size- total_size, 0); // New block size , splitted correct

			LIST_INSERT_AFTER(&freeBlocksList, best_block, free_block);
			set_block_data(best_block,total_size , 1);
			LIST_REMOVE(&freeBlocksList, best_block); // Remove from free list
			return best_block;
		}
		else {
			// Internal fragmentation
			set_block_data(best_block, best_size, 1); // Just mark current block as used
			cprintf("current size when size difference <16 : %d\n",best_size);
			LIST_REMOVE(&freeBlocksList, best_block);
			return best_block;
			}
	}
	sbrk(total_size);
	return NULL; // No suitable block found



}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...
	uint32 blockSize = get_block_size(va);

	if(va==NULL){
		return;
	}
	else if(is_free_block(va)==1){
		return;
    }
	else{

//		test_free_block_FF

		uint32* prevFooter = (uint32*)((uint32)(va-8));
		uint32 prevSize = *prevFooter & ~1 ;  //there's an extra bit we need to minus here!!!!!!!!!!
		uint32* prevVa = (uint32*)((uint32)prevFooter-prevSize+8);

		uint32* nextHeader = (uint32*)((uint32)(va+blockSize-4));
		uint32* nextVa = (uint32*)((uint32)nextHeader+4);
		uint32 nextSize = get_block_size(nextVa);

		struct BlockElement* newBlock =(struct BlockElement*)va;

		if((is_free_block(prevVa)==0)&&(is_free_block(nextVa)==0)){
			 newBlock =(struct BlockElement*)va; //to make the new block points to the prevVa
			 set_block_data(va,blockSize,0);
		}
		else if((is_free_block(prevVa)==1)&&(is_free_block(nextVa)==1)){//merge
			cprintf("when both of the prev & next is free");

			uint32 totalNewBlockSize = prevSize + nextSize + blockSize;

			newBlock =(struct BlockElement*)prevVa; //to make the new block points to the prevVa

			struct BlockElement* prevBlock =(struct BlockElement*)prevVa ;
			LIST_REMOVE(&freeBlocksList,prevBlock);
			struct BlockElement* nextBlock =(struct BlockElement*)nextVa ;//prev block of the newBlock to insert after
			LIST_REMOVE(&freeBlocksList,nextBlock);

			set_block_data(prevVa,totalNewBlockSize,0);

		}
		else if((is_free_block(prevVa)==1)){
			cprintf("when the prev is free");
			uint32 totalNewBlockSize =  prevSize + blockSize;

		    newBlock =(struct BlockElement*)prevVa;

			struct BlockElement* prevBlock =(struct BlockElement*)prevVa ;
			LIST_REMOVE(&freeBlocksList,prevBlock);

			set_block_data(prevVa,totalNewBlockSize,0);

		}
		else{
			cprintf("when the next is free");
			uint32 totalNewBlockSize =  nextSize + blockSize;

			newBlock =(struct BlockElement*)va;

         	struct BlockElement* nextBlock =(struct BlockElement*)nextVa ;
			LIST_REMOVE(&freeBlocksList,nextBlock);

			set_block_data(va,totalNewBlockSize,0);

		}
		struct BlockElement* current;
		uint32 inserted = 0;

		LIST_FOREACH( current , &freeBlocksList ){
			if ( current > newBlock  ){
				LIST_INSERT_BEFORE(&freeBlocksList, current, newBlock );
				inserted = 1;
				break;
			}
		}
		if( inserted == 0 ){
			LIST_INSERT_TAIL(&freeBlocksList,newBlock);
		}
	}

 }

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
	//comment here
if(new_size == 0){
	free_block(va);
    return NULL;
	}
  else if(va == NULL){
	   return alloc_block_FF(new_size);
		   }
  else if(va == NULL && new_size == 0){
	    return NULL;
		   }
  else if( new_size < 8){
  	    return NULL;
  		   }

  else{
	new_size+=8;//for meta data H+F
	cprintf("block size i want to get %d \n" , new_size) ;
	cprintf("block add i got %p \n" , va) ;

	uint32 blockOriginalSize = get_block_size(va);
	int diffMin = blockOriginalSize - new_size; // new_size smaller than original
	int diffMax = new_size - blockOriginalSize; // Original smaller than new_size

	cprintf("block size before (original block size) %d \n" , get_block_size(va) );
	cprintf("expected block size  %d \n" , new_size );
	cprintf("diffMin size  %d \n" , diffMin );
	cprintf("diffMax size  %d \n" , diffMax );


	uint32* nextHeader = (uint32*)((uint32)(va+blockOriginalSize-4));
	uint32* nextVa = (uint32*)((uint32)nextHeader+4);
	uint32 nextSize = get_block_size(nextVa);

if(blockOriginalSize > new_size ){  // minimize block

		if( is_free_block(nextVa) == 1 ){

//						 ######  NOT TESTED  #####
//     DONE TESTEDDDDDD:)
		struct BlockElement* nextBlock=(struct BlockElement*) nextVa;
		struct BlockElement* splittedVa = (struct BlockElement*)((uint32)va + new_size) ;
		LIST_INSERT_BEFORE(&freeBlocksList,nextBlock,splittedVa);
		LIST_REMOVE(&freeBlocksList,nextBlock);
		set_block_data(splittedVa,diffMin+nextSize,0);
		set_block_data(va,new_size,1);
		cprintf("the  blockOriginal size : %d \n \n", blockOriginalSize);
		cprintf("the  new size : %d \n \n", new_size);
		cprintf("the size of the min difference: %d \n \n",diffMin);
		cprintf("the  nextblock size : %d \n \n", nextSize);
		cprintf("the  splitedblock  size : %d \n \n", get_block_size(splittedVa));
		return (struct BlockElement*) va;


	}
else if(is_free_block(nextVa) == 0){    // is_free_block(nextVa) == 0  && no space around it is free ( check size for compaction )
	if(diffMin<16){//internal fragmentation
	return va;
	}
	struct BlockElement* splittedVa = (struct BlockElement*)((uint32)va + new_size) ;

	struct BlockElement* current;
	uint32 inserted = 0;

	LIST_FOREACH( current , &freeBlocksList ){
		if ( current > splittedVa  ){
			LIST_INSERT_BEFORE(&freeBlocksList, current, splittedVa );
			inserted = 1;
			break;
		}
	}
	if( inserted == 0 ){
		LIST_INSERT_TAIL(&freeBlocksList,splittedVa);
	}
	//free_block(splittedVa);
	set_block_data(splittedVa,diffMin,0);
	set_block_data(va,new_size,1);
	return (struct BlockElement*) va;

}
}
else{  //  maximize  blockOriginalSize < new_size
	cprintf("the size of the next block: %d \n \n",nextSize);
	cprintf("the size of the max difference: %d \n \n",diffMax);
	if( is_free_block(nextVa) == 1 && nextSize >= diffMax){
	//no relocate no split TESTED
	if( nextSize < diffMax+16){
// make it accepted when nextSize = diffMax & when it's splited & next_after_split < 16 ( internal fragmentation)
	   struct BlockElement* nextBlock = (struct BlockElement*) nextVa;
	   LIST_REMOVE(&freeBlocksList, nextBlock);
		set_block_data(va, blockOriginalSize+nextSize, 1);
		return (struct BlockElement*) va;

	}
	//no relocate split TESTED
	else{//nextSize > diffMax

		//set_block_data(va, new_size , 1);not needed
		//treated the free blocks as blocks not pointers and didn't use fee_block
		struct BlockElement* nextBlock=(struct BlockElement*) nextVa;
		struct BlockElement* splittedVa = (struct BlockElement*)((uint32)va + new_size) ;
		LIST_INSERT_BEFORE(&freeBlocksList,nextBlock,splittedVa);
		LIST_REMOVE(&freeBlocksList,nextBlock);
		set_block_data(splittedVa,nextSize-diffMax,0); //edited (-) instead of (+)
		set_block_data(va,new_size,1);
		return(struct BlockElement*) va; //edited return immediately

		}
        }
	else { // no space around
		//######NOT TESTED #######
		free_block(va);
		return alloc_block_FF( new_size-8 );}
      }
  }
	//  ####### NOT TESTED ######
	sbrk(new_size);
	return NULL; // No suitable block found


}


/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
