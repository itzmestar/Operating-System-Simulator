#include <sys/types.h>   /* various type definitions.            */
#include <sys/ipc.h> 	 /* general SysV IPC structures          */
#include <getopt.h>		 /* CMD line argument handling			 */
#include <sys/shm.h>	 /* shared mem functions and structs.     */
#include <stdlib.h>	 	 /* rand(), etc.                         */
#include <stdlib.h>		 /* rand(), etc.                         */
#include <stdio.h>		 /* printf() etc.						 */
#include <sys/sem.h>	 /* semaphore functions and structs.     */
#include <time.h>	 	 /* nanosleep(), etc.                    */
#include<signal.h>		 /* handle signal						 */

#define CLOCKID 'C'		 /* to be used in generation of key for clock shared mem*/
#define CLOCKSIZE 2*sizeof(int)	/* size of clock */

#define MAINPRGM "oss.c"
#define SHMMSGID 'M'		 /* to be used in generation of key for shm_msg */

//Read the 2 int Shared M/M clock
//ptr -> pointer to shared memory
//sec -> address of variable to store the second from shared memory after read
//nsec -> address of variable to store the nanosecond from shared memory after read
void read_clock(int *ptr, int *sec, int *nsec);

//Update the Shared M/M clock
//ptr -> pointer to shared memory
//sec -> value of sec to store in shared memory after write
//nsec -> value of nanosec to store in shared memory after write
void update_clock(int *ptr, int sec, int nsec);

//Read the 2 int shm_msg
//ptr -> pointer to shared memory
//sec -> address of variable to store the second from shared memory after read
//nsec -> address of variable to store the nanosecond from shared memory after read
void read_shm_msg(int *ptr, int *sec, int *nsec);

//update the shm_msg
//ptr -> pointer to shared memory
//sec -> value of sec to store in shared memory after write
//nsec -> value of nanosec to store in shared memory after write
void update_shm_msg(int *ptr, int sec, int nsec);

//find if m/m is clear
// return-> 0 if cleared
// return -> 1 if not cleared
int is_clear_shm_msg(int *ptr);

//Same as ftok
// returns key or exits program if unable to do so.
key_t generate_key(const char *pathname, int proj_id);

//Same as shmget
// returns shared memory id or exits program if unable to do so.
int get_shared_memory(key_t key, size_t size, int shmflg);

//Same as shmat
//returns shared memory address or exits program if unable to do so.
int * get_shared_memory_addr(int shmid);

//Same as shmdt
//print error if unable to do so.
void detach_shared_memory(int *ptr);

//deallocate shared memory
//print error if unable to do so.
void deallocate_shared_memory(int shmid);

//Same as semget
// returns semaphore set id or exits program if unable to do so.
int get_semaphore(key_t key, int nsem, int semflg);

/*
 * function: sem_lock. locks the semaphore, for exclusive access to a resource.
 * input:    semaphore set ID.
 * output:   none.
 */
void sem_lock(int sem_set_id);

/*
 * function: sem_unlock. un-locks the semaphore.
 * input:    semaphore set ID.
 * output:   none.
 */
void sem_unlock(int sem_set_id);

//delete the 
void delete_semaphore(int sem_id);

/*
 * function: random_num. 
 * input:    min & max range (including)
 * output:   random number between range.
 */
int random_num(int min, int max);

/*
 * function: random_delay. delay the executing process for a random number
 *           of nano-seconds.
 * input:    none.
 * output:   none.
 */
void random_delay();

