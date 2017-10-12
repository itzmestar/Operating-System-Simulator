#include <string.h>
#include "global.h"
#include <unistd.h>

#define CHILD "./child"

#define INCR 5000

//global variable section
	int *ShmCLOCKPTR=NULL;
	int ShmCLOCKID;
	static int clock_cleared=0;
	
	int *shm_msgPTR=NULL;                        
    int shm_msgID;
	static int msg_cleared=0;
	
	int clock_sem_set_id;            /* ID of the semaphore set */
	static int sema_cleared=0;
	
	pid_t parent_pid;
	pid_t *cpid=NULL;		//pointer to an array of childs pid
	int childs_created=0;
	
	int p_seconds=0;
	int p_nano_seconds=0;
	
	FILE *fp;
	
void log_child_exit(int sec, int nsec){
	fprintf(fp, "\nMaster: Child pid is terminating at time %d.%d because it reached %d.%d in slave",p_seconds,p_nano_seconds,sec,nsec);
}

void increment_clock(){
		p_nano_seconds = p_nano_seconds + INCR;
		if(p_nano_seconds > 1000000000){
			p_seconds ++;
			p_nano_seconds = p_nano_seconds % 1000000000;
		}
}

int is_time_up(int term){
	if(p_seconds >= term){
		return 0;
	}
	return 1;
}

//Print program usage
void print_usage(){
	printf("Usage: ./oss [-s x] [-l filename] [-t z]\n");
	printf("Options:\n\t-h\t\tPrints usage on screen with options and exits.");
	printf("\n\t-s x\t\tNumber of child processes oss should create at startup. Default is 5.");
	printf("\n\t-l filename\tLogfile name for oss. Default is ./oss.log file.");
	printf("\n\t-t z\t\tNumber of seconds oss should run. Default is 20.\n");
}

//kill all the childs processes
void kill_childs(){
	int i;
	for( i = 0; i < childs_created; i++){
		kill(*(cpid+i), SIGQUIT);
		//printf("\n%d",*(cpid+i));
	}
}

//clean up
void cleanup(){
	//Detach & Free shared memory
	if(ShmCLOCKPTR!=NULL && clock_cleared==0){
		clock_cleared++;
		detach_shared_memory(ShmCLOCKPTR);
		deallocate_shared_memory(ShmCLOCKID);
	}
	
	//Detach & Free shared memory
	if(shm_msgPTR!=NULL && msg_cleared==0){
		msg_cleared++;
		detach_shared_memory(shm_msgPTR);
		deallocate_shared_memory(shm_msgID);
	}		
	
	//delete semaphore
	if(sema_cleared==0){
		sema_cleared++;
		delete_semaphore(clock_sem_set_id);
	}
	
	if(fp != NULL)
		close(fp);
}

//handle signal
void sig_handler(int signo)
{
  if (signo == SIGINT){
    printf("received SIGINT\n");
	kill_childs();
	cleanup();
	exit (1);
  }
}

//forks off child process
pid_t new_child(){
	//Child process name & args
	char *args[] = {CHILD, NULL };
	pid_t child_pid;
	
	child_pid = fork();
		
	//child process
	if( child_pid == 0 ) {
		execvp(args[0], args);
	}
	return child_pid;
}

void clear_shm_msg(int *ptr){
	update_shm_msg(ptr, 0, 0);
}

int critical_section(){
	int sec=0, nsec=0;
	update_clock(ShmCLOCKPTR, p_seconds, p_nano_seconds);
	
	//read shm_msg
	read_shm_msg(shm_msgPTR, &sec, &nsec);
	
	//if shm_msg not empty then return
	if ( sec !=0 || nsec != 0 ){
		log_child_exit(sec, nsec);
		
		//clear shm_msg
		clear_shm_msg(shm_msgPTR);
		return 1;
	}
	return 0;
}

int main (int argc, char **argv)
{
	int slave_process=5; //default value
	int terminate_time=20; //seconds, default value
	char *logfile="oss.log"; //default value
	
	//Shared Memory Clock related
	key_t clock_key;
	
	//Shared Memory shm_msg related            
	key_t shm_msg_key;                         
	
	int c_seconds=0;
	int c_nano_seconds=0;
	
	//Semaphore related

    union semun {              /* semaphore value, for semctl().     */
                int val;
                struct semid_ds *buf;
                ushort * array;
    } clock_sem_val;
		
	pid_t wpid;
	
	int c;
    int i = 0;
    long sum;
    //int pid;
    int status, ret;

	//handle signal
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");
	
	parent_pid = getpid();
	
	//handle arguments
	while ((c = getopt (argc, argv, "hs:l:t:")) != -1)
    switch (c)
    {
      case 'h':
        print_usage();
        return 0;
		
      case 's':
        slave_process = atoi(optarg);
        break;
      case 'l':
        logfile = optarg;
        break;
	  case 't':
        terminate_time = atoi(optarg);
        break;
      case '?':
        if (optopt == 's' || optopt == 'l' || optopt == 't')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		  
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
		print_usage();
        return 1;
    }
	  
	  //open log file to write
	  
	fp = fopen ( logfile, "w" ) ;
	if ( fp == NULL )
	{
		puts ( "Cannot open log file" ) ;
		exit(1) ;
	}
	//generate key to be used in semaphore & shared mem for clock
	clock_key = generate_key(MAINPRGM, CLOCKID); //printf("%d",clock_key);
	  
	/* create a semaphore set with key clock_key, with one semaphore   */
	clock_sem_set_id = get_semaphore(clock_key, 1, 0644 | IPC_CREAT | IPC_EXCL);
	  
	/* intialize the first (and single) semaphore in our set to '1'. */
	clock_sem_val.val = 1;
    if (semctl(clock_sem_set_id, 0, SETVAL, clock_sem_val) == -1) {
		perror("main: semctl");
		exit(1);
    }
	  
	/* allocate a shared memory segment */
	ShmCLOCKID = get_shared_memory(clock_key, CLOCKSIZE, 0644 | IPC_CREAT );
	  
	/* attach the shared memory segment to process's address space. */
	ShmCLOCKPTR = get_shared_memory_addr(ShmCLOCKID);

	//generate key to be used in Shared Memory shm_msg
	shm_msg_key = generate_key(MAINPRGM, SHMMSGID);
	
	/* allocate a shared memory segment */
	shm_msgID = get_shared_memory(shm_msg_key, CLOCKSIZE, 0664 | IPC_CREAT );
	
	/* attach the shared memory segment to process's address space. */
	shm_msgPTR = get_shared_memory_addr(shm_msgID);
	
	//clear shm_msg
	clear_shm_msg(shm_msgPTR);
	
	cpid = (int *)malloc(slave_process*sizeof(pid_t));
	if(cpid == NULL){
		cleanup();
		exit(1);
	}
	
	/* fork-off a child process as clock is already set to 0, 0 */
	for(i=0;i<slave_process;i++)
	{
		*(cpid+i) = new_child();
		childs_created++;
	}
	
	//loop forever
	while (1)
    {
		increment_clock();
		
		sem_lock(clock_sem_set_id);

		if(critical_section()==1){
			wpid = wait(&status);
			for( i = 0; i < childs_created; i++){
				if(*(cpid+i) == wpid){
					*(cpid+i) = new_child();
					break;
				}
			}
		}
		if(is_time_up(terminate_time)==0){
			kill_childs();
			sem_unlock(clock_sem_set_id);
			break;
		}
		sem_unlock(clock_sem_set_id);
    }

	while ((wpid = wait(&status)) > 0){
		
	}
	cleanup();
	return 0;
}
