// Test the creation of shared variables and using them
// Master program: create the shared variables, initialize them and run slaves
#include <inc/lib.h>

void
_main(void)
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE THAT SOME
	 * PAGES ARE ALLOCATED IN KERNEL DYNAMIC ALLOCATOR DUE
	 * TO sbrk()
	 *********************************************************/

	/*=================================================*/
	//Initial test to ensure it works on "PLACEMENT" not "REPLACEMENT"
#if USE_KHEAP
	{
		if (LIST_SIZE(&(myEnv->page_WS_list)) >= myEnv->page_WS_max_size)
			panic("Please increase the WS size");
	}
#else
	panic("make sure to enable the kernel heap: USE_KHEAP=1");
#endif
	/*=================================================*/
	//cprintf("entered the test! \n");
	int eval = 0;
	bool is_correct = 1;
	uint32 pagealloc_start = USER_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE; //UHS + 32MB + 4KB
	uint32 *x, *y, *z ;
	int diff, expected;

	//x: Readonly
	//cprintf("before allocating x! \n");

	int freeFrames = sys_calculate_free_frames() ;
	x = smalloc("x", 4, 0);
	if (x != (uint32*)pagealloc_start) {is_correct = 0; cprintf("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");}
	expected = 1+1 ; /*1page +1table*/
	diff = (freeFrames - sys_calculate_free_frames());
	if (diff < expected || diff > expected +1+1 /*extra 1 page & 1 table for sbrk (at max)*/) {is_correct = 0; cprintf("Wrong allocation (current=%d, expected=%d): make sure that you allocate the required space in the user environment and add its frames to frames_storage", freeFrames - sys_calculate_free_frames(), expected);}
	//cprintf("after allocating x! \n");

	//y: Readonly
	freeFrames = sys_calculate_free_frames() ;
	y = smalloc("y", 4, 0);
	if (y != (uint32*)(pagealloc_start + 1 * PAGE_SIZE)) {is_correct = 0; cprintf("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");}
	expected = 1 ; /*1page*/
	diff = (freeFrames - sys_calculate_free_frames());
	if (diff < expected || diff > expected +1+1 /*extra 1 page & 1 table for sbrk (at max)*/) {is_correct = 0; cprintf("Wrong allocation (current=%d, expected=%d): make sure that you allocate the required space in the user environment and add its frames to frames_storage", freeFrames - sys_calculate_free_frames(), expected);}
	//cprintf("after allocating y! \n");

	//z: Writable
	freeFrames = sys_calculate_free_frames() ;
	z = smalloc("z", 4, 1);
	if (z != (uint32*)(pagealloc_start + 2 * PAGE_SIZE)) {is_correct = 0; cprintf("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");}
	expected = 1 ; /*1page*/
	diff = (freeFrames - sys_calculate_free_frames());
	if (diff < expected || diff > expected +1+1 /*extra 1 page & 1 table for sbrk (at max)*/) {is_correct = 0; cprintf("Wrong allocation (current=%d, expected=%d): make sure that you allocate the required space in the user environment and add its frames to frames_storage", freeFrames - sys_calculate_free_frames(), expected);}
	//cprintf("after allocating z! \n");

	if (is_correct)	eval+=25;
	is_correct = 1;
	cprintf("EVAL: %d \n",eval);


	*x = 10 ;
	*y = 20 ;

	//cprintf("after manipulating y and z! \n");

	int id1, id2, id3;
	id1 = sys_create_env("shr2Slave1", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
	id2 = sys_create_env("shr2Slave1", (myEnv->page_WS_max_size), (myEnv->SecondListSize),(myEnv->percentage_of_WS_pages_to_be_removed));
	id3 = sys_create_env("shr2Slave1", (myEnv->page_WS_max_size), (myEnv->SecondListSize),(myEnv->percentage_of_WS_pages_to_be_removed));
	//cprintf("after creating enviroment 1 2 and 3! \n");

	//to check that the slave environments completed successfully
	rsttst();

	sys_run_env(id1);
	sys_run_env(id2);
	sys_run_env(id3);
	//cprintf("after running enviroment 1 2 and 3! \n");

	cprintf("before entering the while loop! \n");

	//to ensure that the slave environments completed successfully
	while (gettst()!=3) ;// panic("test failed");

	cprintf("after finishing the while loop! \n");

	if (*z != 30)
	{is_correct = 0; cprintf("Error!! Please check the creation (or the getting) of shared variables!!\n\n\n");}

	if (is_correct)	eval+=25;
	is_correct = 1;
	cprintf("EVAL: %d \n",eval);

	atomic_cprintf("%@Now, attempting to write a ReadOnly variable\n\n\n");

	id1 = sys_create_env("shr2Slave2", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));

	sys_run_env(id1);

	//to ensure that the slave environment edits the z variable
	while (gettst() != 4) ;

	if (*z != 50)
	{is_correct = 0; cprintf("Error!! Please check the creation (or the getting) of shared variables!!\n\n\n");}

	if (is_correct)	eval+=25;
	is_correct = 1;
	cprintf("EVAL: %d \n",eval);


	//Signal slave2
	inctst();

	//to ensure that the slave environment attempt to edit the x variable
	while (gettst()!=6) ;// panic("test failed");

	if (*x != 10)
	{is_correct = 0; cprintf("Error!! Please check the creation (or the getting) of shared variables!!\n\n\n");}

	if (is_correct)	eval+=25;
	is_correct = 1;
	cprintf("EVAL: %d \n",eval);


	cprintf("\n%~Test of Shared Variables [Create & Get] completed. Eval = %d%%\n\n", eval);
	return;
}
