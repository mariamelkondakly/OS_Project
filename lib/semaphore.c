// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore *Semaphore_ptr = NULL;
	struct __semdata *Semdata_ptr = NULL;
	struct semaphore Semaphore;
//	struct __semdata Semdata;
//	struct Env_Queue blocked_envs_queue;
//	//cprintf("add of sem ptr %d \n",&blocked_envs_queue);
//	sys_init_queue(&blocked_envs_queue);


	Semaphore_ptr=(struct semaphore*)smalloc(semaphoreName,(uint32)sizeof(struct semaphore),1);//writable be 1??
	//cprintf("add retured from first smalloc %d \n",Semaphore_ptr);
	Semdata_ptr =(struct __semdata *)smalloc("semaphore_shared_semdata",sizeof(struct __semdata),1);
	//cprintf("add retured from first smalloc %d \n",Semdata_ptr);
	if(Semaphore_ptr==NULL ||Semdata_ptr==NULL)
		return Semaphore;

	Semdata_ptr->count = value;
	Semdata_ptr->lock = 0;
	strcpy(Semdata_ptr->name, semaphoreName);
	sys_init_queue(&Semdata_ptr->queue);

	Semaphore_ptr->semdata = Semdata_ptr;

    //cprintf("5alast create \n");
	return *Semaphore_ptr;

}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("entered get sem \n");
	struct semaphore *semptr = NULL;
	semptr=(struct semaphore*) sget(ownerEnvID,semaphoreName);
	//cprintf("exit get sem \n");
	return *semptr;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	cprintf("entered wait \n");
	cprintf("lock value is %d \n ",sem.semdata->lock);
	cprintf("name value is %d \n ",sem.semdata->name);
//	uint32 key =1;
	while(xchg(&(sem.semdata->lock), 1) != 0);
	cprintf("took lock lock value insise while is %d \n",sem.semdata->lock);
	sem.semdata->count--;
	if(sem.semdata->count<0){
		cprintf("enqueue \n");
		sys_enqueue(&sem.semdata->queue);
		cprintf("finished enqeue");
		sem.semdata->lock=0;
	}
	else{
		cprintf("passed into cs \n");
		sem.semdata->lock=0;
	}
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("entered signal \n");
	while(xchg(&sem.semdata->lock, 1) != 0);
	sem.semdata->count++;
	if(sem.semdata->count<=0){
		if(sys_queue_size(&sem.semdata->queue)==0)
		{
		struct Env* process= sys_dequeue(&sem.semdata->queue);
//		process->env_status=1;
		sys_sched_insert_ready(process);
		}
	}
	sem.semdata->lock=0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
