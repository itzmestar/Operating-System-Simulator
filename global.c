#include "global.h"


void read_clock(int *ptr, int *sec, int *nsec){
	*sec = *ptr;
	ptr++;
	*nsec = *ptr;
}

void update_clock(int *ptr, int sec, int nsec){
	*ptr = sec;
	ptr++;
	*ptr = nsec;
}

void read_shm_msg(int *ptr, int *sec, int *nsec){
	*sec = *ptr;
	ptr++;
	*nsec = *ptr;
}

void update_shm_msg(int *ptr, int sec, int nsec){
	*ptr = sec;
	ptr++;
	*ptr = nsec;
}

int is_clear_shm_msg(int *ptr){
	if (*ptr != 0){
		return 1;
	}
	ptr++;
	if (*ptr != 0){
		return 1;
	}
	return 0;
}

key_t generate_key(const char *pathname, int proj_id){
	key_t key;
	/* make the key: */
    if ((key = ftok(pathname, proj_id)) == -1) {
        perror("ftok");
        exit(1);
    }
	return key;
}

int get_shared_memory(key_t key, size_t size, int shmflg){
	int shmid;
	if ((shmid = shmget(key, size, shmflg)) == -1) {
        perror("shmget");
        exit(1);
    }
	return shmid;
}

int * get_shared_memory_addr(int shm_id){
	int *ptr = shmat(shm_id, NULL, 0);
    if (!ptr) {
        perror("shmat");
        exit(1);
    }
	return ptr;
}

void detach_shared_memory(int *ptr){
	if (shmdt(ptr) == -1) {
        perror("shmdt: ");
    }
}

void deallocate_shared_memory(int shm_id){
	struct shmid_ds shm_desc;
	if (shmctl(shm_id, IPC_RMID, &shm_desc) == -1) {
        perror("shmctl: ");
		//puts(getpid());
    }
}

int get_semaphore(key_t key, int nsem, int semflg){
	int sem_set_id;
    sem_set_id = semget(key, nsem, semflg);
    if (sem_set_id == -1) {
	perror("semget");
	exit(1);
    }
	return sem_set_id;
}

void delete_semaphore(int sem_id){
    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
		//puts(getpid());
    }
}

void sem_lock(int sem_set_id)
{
    /* structure for semaphore operations.   */
    struct sembuf sem_op;

    /* wait on the semaphore, unless it's value is non-negative. */
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}

void sem_unlock(int sem_set_id)
{
    /* structure for semaphore operations.   */
    struct sembuf sem_op;

    /* signal the semaphore - increase its value by one. */
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;   
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}

int random_num(int min, int max)
{
    static int init = 0;
    int random;

    if (!init) {
		srand(time(NULL)+ getpid());
		init = 1;
    }

    random = rand() % max;
	return (random + min);
}

void random_delay()
{
    static int initialized = 0;
    int random;
    struct timespec delay;            /* used for wasting time. */

    delay.tv_sec = 0;
    delay.tv_nsec = random_num(1,100000) * 10;
    nanosleep(&delay, NULL);
}
