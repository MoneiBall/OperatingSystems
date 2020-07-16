#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


void handler(int signumber,siginfo_t* info,void* nonused){
  switch (info->si_code){ 
    case SI_USER:  /*printf("Process (PID number %i) sent the signal (calling kill)\n",info->si_pid)*/; break;
    case SI_QUEUE: /*printf("It was sent by a sigqueue, sending process (PID %i)\n",info->si_pid)*/;
                   printf("Additional value: %i\n",info->si_value.sival_int);
                   break;
    default: printf("It was sent by something else \n");
  } 
}


struct messg { 
     long mtype;
     char mtext [ 1024 ];
}; 

// sendig a message
int send( int mqueue ) 
{ 
     const struct messg m = { 5, "To wear a mask is really very important to save other people and yourself against the virus in the shops and on the roads"}; 
     int status; 
     status = msgsnd( mqueue, &m, sizeof ( m.mtext ) , 0 ); 
     if ( status < 0 ) 
          perror("msgsnd error"); 
     return 0; 
} 
     
// receiving a message. 
int receive( int mqueue ) 
{ 
     struct messg m; 
     int status; 	 
     status = msgrcv(mqueue, &m, 1024, 5, 0 ); 
     
     if ( status < 0 ) 
          perror("msgsnd error"); 
     else
          printf( "Message :  %s\n", m.mtext ); 
     return 0; 
} 

sem_t* semaphore_create(char*name,int semaphore_value)
{
    sem_t *semid=sem_open(name,O_CREAT,S_IRUSR|S_IWUSR,semaphore_value );
	if (semid==SEM_FAILED)
	perror("sem_open");
       
    return semid;
}

void semaphore_delete(char* name)
{
      sem_unlink(name);
}


int main(int argc, char* argv[]) {


 pid_t pid, child_a, child_b;
 int status;

  struct sigaction sigact;
  sigact.sa_sigaction= handler; 
  sigemptyset(&sigact.sa_mask); 
  sigact.sa_flags=SA_SIGINFO; 
  sigaction(SIGTERM,&sigact,NULL); 

  int pipefd[2];
  int pipefd2[2];
  char c[100];

   

  //start pipe before forking
  if(pipe(pipefd) == -1 || pipe(pipefd2) == -1){ 
        perror("Opening pipe failed!");
        exit(EXIT_FAILURE);
  }

     int messg;
     key_t key; 
     key = ftok(argv[0],1); 
     messg = msgget( key, 0600 | IPC_CREAT ); 
     if ( messg < 0 ) { 
          perror("msgget error"); 
          return 1; 
     } 

     int sh_mem_id;
     int *number;

    key_t keym; 
    keym = ftok(argv[0],1); 
    sh_mem_id=shmget(keym,513,IPC_CREAT|S_IRUSR|S_IWUSR);
    number = shmat(sh_mem_id,NULL,0);

   

    char* sem_name="/blbla";
    sem_t* semid;
    semid = semaphore_create(sem_name,0);



  child_a = fork();


   if(child_a == -1){ //failed
        perror("Fork failed!");
        exit(EXIT_FAILURE);
   }

   if(child_a == 0){ //child a /// police officer (child)
       printf("Police Lieutenant-colon (child) settle down and READY | pid: %d\n",getpid());
       kill(getppid(),SIGTERM);

        close(pipefd[1]);
        read(pipefd[0],c,sizeof(c)); // reading max 100 chars
        printf("Police Lieutenant-colon (child) read the message: %s\n",c);
        close(pipefd[0]); // Closing write descriptor 

       
        close(pipefd2[0]);
        write(pipefd2[1], "Yes, it is compulsory to wear the mask when you leave your flat!",80);
        close(pipefd2[1]); // Closing write descriptor 
        printf("Police Lieutenant-colon (child) answered to the message and sent back to  Communication Officer (parent process)!\n");

        kill(getppid(),SIGTERM);
   }

//*************************************************************************************
   else{ 

      child_b = fork();

      if(child_b == -1){ 
        perror("Fork b failed!");
        exit(EXIT_FAILURE);
      }

      if(child_b == 0){ 
       printf("National Head Physician  (child) Settle Down, and READY | pid: %d\n",getpid());
       kill(getppid(),SIGTERM);
       sleep(2);


       send(messg);
        status = msgctl(messg, IPC_RMID, NULL ); 
          if ( status < 0 ) 
               perror("msgctl error"); 
       printf("National Head Physician  (child) sent message to the  Communication Officer (parent process) \n%s");


       srand(time(0));
       int n = 5 + rand() % 100;
       number[0] = n;
       
       printf("National Head Physician  (child)  semaphore up!\n");
       	sem_post(semid);	
           
       shmdt(number);
       shmctl(sh_mem_id,IPC_RMID,NULL);
       sleep(3);

      }
      

      else { 

      printf("I am  Communication Officer (parent process) | pid: %d\n",getpid());
      wait(&status);

      close(pipefd[0]);
      write(pipefd[1], "Is it compulsory to wear a mask in the shops?",50);
      close(pipefd[1]); 
      printf("Communication Officer (parent process) wrote the message to the pipe!\n");

      
      wait(&status);

      close(pipefd2[1]);
      read(pipefd2[0],c,sizeof(c)); 
      printf("Communication Officer (parent process) read the answers: %s\n",c);
      close(pipefd2[0]); 
      
      
      wait(&status);
      receive(messg);

       		sem_wait(semid);	
       		printf("Infected people: %d\n",number[0]); 
       		sem_post(semid); 
            printf("Communication Officer (parent process) closed the semaphore!\n");  


      semaphore_delete(sem_name);
      shmdt(number);

       wait(NULL);
       fflush(NULL);     
      }
 }

  
    return 0;
}






