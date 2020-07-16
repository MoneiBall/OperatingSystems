//Author: Sadi Mamedov
//ver: 1.0.0
//rel: 02/05/2020
//hw2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

int MONDAY_LIMIT = 4;
int TUESDAY_LIMIT = 4;
int WEDNESDAY_LIMIT = 4;
int THURSDAY_LIMIT = 4;
int FRIDAY_LIMIT = 3;
int SATURDAY_LIMIT = 2;
int LIMIT;

struct {
  char name[30];
  char address[50];
  char weekday[50]; 
} employee;


void handler(int signumber,siginfo_t* info,void* nonused){
  switch (info->si_code){ 
    case SI_USER: printf("Process (PID number %i) sent the signal\n",info->si_pid); break;
    case SI_QUEUE: printf("Process (PID number %i) sent the signal by sigqueue\n",info->si_pid);
                   printf("Number of delivered workers: %i\n",info->si_value.sival_int);
                   break;
    default: printf("It was sent by something else \n");
  } 
}


void pipeWork(){
    char* parts[] = {"Brilliant Land","Sparkling Ground","Long Plot","Silky Land","Miller Ground","Red Plot"};
    char* works[] = {"incision", "spraying", "spring opening","hoe work"};


  struct sigaction sigact;
  sigact.sa_sigaction=handler; //instead of sa_handler, we use the 3 parameter version
  sigemptyset(&sigact.sa_mask); 
  sigact.sa_flags=SA_SIGINFO; //we need to set the siginfo flag 
  sigaction(SIGTERM,&sigact,NULL); 


  
    
    int pipefd[2]; // unnamed pipe file descriptor array
    int pipefmessage[2];
    //access pipefd[0] for reading data //access poss (Parent) finished!
    //pipefd[1] for writing data //
    pid_t parent = getpid();
    pid_t child_a;
    pid_t child_b;
    char c[100];
    char c_next[100];  
    char current_day[50];
    printf("Enter Current Workday: \n");
    scanf(" %50[^\n]",&current_day);



     



   

    if(pipe(pipefd) == -1 || pipe(pipefmessage) == -1){ //start pipes()
        perror("Opening pipe failed!");
        exit(EXIT_FAILURE);
    }

    child_a = fork(); //forking creating parent/child process

    if(child_a == -1) {//fork not succesfull!
        perror("Fork failed!");
        exit(EXIT_FAILURE);
    }
//=========================================================================== FIRST CHILD
    if(child_a == 0) {
        //child_a
        //first child process -- Steward
       printf("Under Steward (first Child) starts!\n");
       sleep(2);
       close(pipefd[0]); //close unused read

       
       //sending proper data later on
           


    
      srand(time(0));
      int n = rand() % sizeof(parts)/sizeof(parts[0]);
      int m = rand() % sizeof(works)/sizeof(works[0]);

       char str[100];
       strcpy(str,"");
       strcat(str,parts[n]);
       strcat(str,", ");
       strcat(str,works[m]);

       write(pipefd[1], str,50); //write name of Vineyard Part
       
       sleep(2);
       printf("****************************************\n");
       printf("Under Steward wrote the urgent task to the pipe!\n");
       
       close(pipefd[1]); ///closing write desc after we done
    

       kill(getppid(),SIGTERM); //call the parent process
       printf("Under Steward (first Child) Finished! \n");
       exit(0);
       
    }
    
    else {   
        //sleep(3);
       
       child_b = fork();

       if(child_b == -1) {//second fork not succesfull!
        perror("Second Fork failed!");
        exit(EXIT_FAILURE);
       }

//=========================================================================++Second CHILD
        if(child_b == 0){
        //child_b  //workers flight
        sleep(2);
        printf("Workers Leader (Second Child) Starts!\n");
        pause(); //wait till siganl arrive
        printf("Signal %d arrived to Workers Leader\n",SIGTERM);
        
           close(pipefmessage[1]);
           read(pipefmessage[0], c_next, sizeof(c_next)); //read message of workers and the task

           sleep(3);
           printf("****************************************\n");
           printf("Workers Leader read Task and Available Workers list: %s\n",c_next);
           close(pipefmessage[0]);

          int cnt = -1;
           for(int i=0;i<strlen(c_next);i++){
             if(c_next[i] == ',')
                cnt++;
           }

          srand(time(0));
          LIMIT = 3 + rand() % 4;
           
           int k = 0;
           char* workerQueue[] = {"John Carter","Hilary Clinton","Eva Adams","Ruy Lopez","Caruso Lopez","Anthony Hawakins","Steve Jobs","Josh Brolin"};
           while(cnt < LIMIT){
             strcat(c_next,workerQueue[k]);
             strcat(c_next,", ");
             k++;
             cnt++;
           }
           
           if(k != 0){
             sleep(3);
             printf("****************************************\n");
             printf("Workers on queue added to flight!!\n%s\n",c_next);
           }


        //cnt is the number of workers on field
        union sigval s_value_int = {cnt};
        sigqueue(getppid(),SIGTERM,s_value_int);

         //fflush(NULL); 	// flushes all write buffers (not necessary)
         //wait(NULL);		// waiting for parent process (not necessary) 

         printf("Workers Leader (Second Child) finished!\n");
         exit(0);     
       }
       
//===============================================================================Parent
       else{
           // parent process --  Central Vineyard Office
           //parent process
            sleep(2);
            printf("Boss (Parent process) Starts!\n");
            

            //-------------------waits for the signal from first child
            fflush(NULL); 	// flushes all write buffers (not necessary)
            wait(NULL);		// waiting for parent process (not necessary) 
            printf("Signal %d arrived to BOSS \n",SIGTERM);
            close(pipefd[1]); //Usually we close unused read end
            read(pipefd[0],c,sizeof(c)); //read the name of Vineyard child sent

            sleep(2);
            printf("****************************************\n");
            printf("Boss received the message from Steward: %s\n",c);
            close(pipefd[0]); // Closing read descriptor after we done
            
//--------------------------------------------------------------------- read from file
            FILE *file = fopen("./register_list.txt","a+"); 
             if(file == NULL){
                 printf("%s","Error! File could not be opened!");
                 exit(-1);
              }

              char message[255];
              strcpy(message,c);
              strcat(message," --> ");


              char line[255];
              int counter = 0;
        while (fgets(line, sizeof(line), file)){
                    char *token;
                    token = strtok (line,",");
                    char* firstToken = token;
                    char* lastToken;
              while (token != NULL)
                {
                 lastToken = token ;
                 token = strtok (NULL, ",");
             }
     

            if(strstr(lastToken,current_day)){
                strcat(message,firstToken);
                strcat(message,", ");
                counter++;
             }     
         } 

           if(counter == 0){
                printf("%s","No employee on duty in this day!\n");
            }
            fclose(file);

            close(pipefmessage[0]);
            write(pipefmessage[1], message, 255); //write name of workers

            sleep(2);
            printf("Boss (Parent) write the name of registered workers\n");

            close(pipefmessage[1]);

            kill(child_b,SIGTERM);
            

            pause();
            fflush(NULL); 	// flushes all write buffers (not necessary)
            wait(NULL);		// waiting for parent process (not necessary) 

            printf("Signal %d arrived to BOSS \n",SIGTERM);      
            printf("Boss (Parent) finished!\n");          
       }

    }

    
}




//==================================================================================
//Function that prints daily list of employees based on given day
//==================================================================================
void printDaily(){
    char day[50];
    printf("Enter the day you want see registration list: \n");
    scanf(" %50[^\n]", &day);
    
    char line[255];
    FILE *file = fopen("./register_list.txt","a+");

if(file == NULL){
    printf("%s","Error! File could not be opened!");
    exit(-1);
}


int cnt = 0;
 while (fgets(line, sizeof(line), file)){
  char *token;
  token = strtok (line,",");
  char* firstToken = token;
  char* lastToken;
    while (token != NULL)
    {
         lastToken = token ;
         token = strtok (NULL, ",");
    }
     

      if(strstr(lastToken,day)){
          printf("%s\n",firstToken);
          cnt++;
      }
 } 

  if(cnt == 0){
      printf("%s","No employee on duty in this day!\n");
  }


}

//==================================================================================
//Deleting employee data, deletes original registry file line using temp file 
//==================================================================================

void deleteEmployee(){
   char employee_name[50];
   char line[255];
   FILE *orig_file = fopen("register_list.txt","r");
   
   if(orig_file == NULL){
    printf("%s","Error! Original File could not be opened!");
    exit(-1);
   }

   FILE *tmp_file = fopen("tmp.txt","w");
   
   if(tmp_file == NULL){
    printf("%s","Error! Temproray File could not be opened!");
    exit(-1);
   }


    printf("Enter employee you want to remove from list: \n");
    scanf(" %30[^\n]", &employee_name);
    int line_counter = 0;
    int atleast = 0;
    while (fgets(line, sizeof(line), orig_file)){
         char *token;
         token = strtok(line, ",");
         line_counter++;
        if(strcmp(token, employee_name) == 0){
           atleast++;
           break;
        }
    }

    if(atleast == 0){
        line_counter = 0;
        printf("%s","No such employee exists in database! \n");
    }
        
    rewind(orig_file);

   // printf("%d",line_counter);

    int cnt = 1;
     while (fgets(line, sizeof(line), orig_file)){
       if(cnt != line_counter){
        fprintf(tmp_file, "%s", line); 
       }
       else 
         printf("%s","Employee data deleted successfully \n");
       cnt++;
    }

    fclose(orig_file);
    fclose(tmp_file);
    remove("register_list.txt");
    rename("tmp.txt","register_list.txt");
}


//==================================================================================
//Modifying employee data, updates original registry file line using temp file 
//==================================================================================
void modifyEmployee(){
   char employee_name[50];
   char line[255];
   FILE *orig_file = fopen("register_list.txt","r");
   
   if(orig_file == NULL){
    printf("%s","Error! Original File could not be opened!");
    exit(-1);
   }

   FILE *tmp_file = fopen("tmp.txt","w");
   
   if(tmp_file == NULL){
    printf("%s","Error! Temproray File could not be opened!");
    exit(-1);
   }


    printf("Enter employee you want change data: \n");
    scanf(" %30[^\n]", &employee_name);
    int line_counter = 0;
    int atleast = 0;
    while (fgets(line, sizeof(line), orig_file)){
         char *token;
         token = strtok(line, ",");
         line_counter++;
        if(strcmp(token, employee_name) == 0){
           atleast++;
           break;
        }
    }

    if(atleast == 0){
        line_counter = 0;
        printf("%s","No such employee exists in database!");
    }
        
    rewind(orig_file);

    printf("Modify employee name: \n");
    scanf(" %30[^\n]",&employee.name);

    printf("Modify street address of employee: \n");
    scanf(" %50[^\n]",&employee.address);

    printf("Modify employee's workday(s): \n");
    scanf(" %50[^\n]",&employee.weekday);

    int cnt = 1;
     while (fgets(line, sizeof(line), orig_file)){
       if(cnt != line_counter){
        fprintf(tmp_file, "%s", line); 
       }
       else {
         printf("%s","Employee Data modified successfully.\n");
         fprintf(tmp_file, "%s,%s,%s \n", employee.name, employee.address, employee.weekday);
         }
       cnt++;
    }

    fclose(orig_file);
    fclose(tmp_file);
    remove("register_list.txt");
    rename("tmp.txt","register_list.txt");
}



//==================================================================================
//Registering employee data, and checking validation based on requiremnets 
//==================================================================================
void registerEmployee(){

printf("Enter the new employee name: \n");
scanf(" %30[^\n]",&employee.name);

printf("Enter the street address of employee: \n");
scanf(" %50[^\n]",&employee.address);

printf("Enter employee's workday(s): \n");
scanf(" %50[^\n]",&employee.weekday);


////-------------------------------------check validity of data
int mon_cnt = 0;
int tue_cnt = 0;
int wed_cnt = 0;
int thu_cnt = 0;
int fri_cnt = 0;
int sat_cnt = 0;

char line[255];

FILE *file = fopen("./register_list.txt","a+");

if(file == NULL){
    printf("%s","Error! File could not be opened!");
    exit(-1);
}


//count how often each day exists
while (fgets(line, sizeof(line), file)){
    if(strstr(line, "Monday") && strstr(employee.weekday, "Monday"))
       mon_cnt++;
    if(strstr(line, "Tuesday") && strstr(employee.weekday, "Tuesday"))
       tue_cnt++;
    if(strstr(line, "Wednesday") && strstr(employee.weekday, "Wednesday"))
       wed_cnt++;
    if(strstr(line, "Thursday") && strstr(employee.weekday, "Thursday"))
       thu_cnt++;
    if(strstr(line, "Friday") && strstr(employee.weekday, "Friday"))
       fri_cnt++;
    if(strstr(line, "Saturday") && strstr(employee.weekday, "Saturday"))
       sat_cnt++;
} 

//ALL-SEARCH algorithm
  int flag = 1;
  char *ch;
  char temp[50];
  strcpy(temp, employee.weekday);
  ch = strtok(temp, " ");
 while (ch != NULL && flag) {
        printf("%s---->\n",ch);
   if(strcmp(ch, "Monday") == 0){
        if(mon_cnt >= MONDAY_LIMIT)
           flag = 0;
   }
       
    else if(strcmp(ch, "Tuesday") == 0){
       if(tue_cnt >= TUESDAY_LIMIT)
           flag = 0;
    }
    else if(strcmp(ch, "Wednesday") == 0){
       if(wed_cnt >= WEDNESDAY_LIMIT)
           flag = 0;
    }
     else if(strcmp(ch, "Thursday") == 0){
       if(thu_cnt >= THURSDAY_LIMIT)
           flag = 0;
    }
     else if(strcmp(ch, "Friday") == 0){
       if(fri_cnt >= FRIDAY_LIMIT)
           flag = 0;
    }
     else if(strcmp(ch, "Saturday") == 0){
       if(sat_cnt >= SATURDAY_LIMIT)
           flag = 0;
    }
    
    else {
        printf("%s", "Wrong input entered! Registration failed! \n");
        flag = -1;
    }

   ch = strtok(NULL, " ,");
 }  


 if(flag){
    printf("%s ---> %s", employee.name , "was registered successfully! \n");
    fprintf(file, "%s,%s,%s \n", employee.name, employee.address, employee.weekday);
 }

 else if (flag == 0)
 {
    printf("%s", "Registration rejected! No availbale space for this day! \n");
 }
 
 

fclose(file);   
}
//=====================================================================================
//Function to print the list of employees data / file
//=====================================================================================

void printList(){

 FILE *file = fopen("./register_list.txt","a+"); 
 if(file == NULL){
    printf("%s","Error! File could not be opened!");
    exit(-1);
}

char line[255];

while (fgets(line, sizeof(line), file)){
   printf("%s",line);
}
fclose(file); 

}



//==================================================================================
//Function to handle continuous menu
//==================================================================================
void menu(){

int menu_item;


printf("\nWelcome to the \"Fine Wine Good Health\" vineyard registration! \n");
do{
        printf("***************************************************\n");
        printf("Choose one of the options below:  \n");
        printf("1 - Register a new employee \n");
        printf("2 - Modify an employee's data \n");
        printf("3 - Delete an employee's data \n");
        printf("4 - Print Registration list \n");
        printf("5 - Print daily employee list \n");
        printf("6 - Send Message with pipes \n");
        printf("7 - Exit \n");
        printf("***************************************************\n:");
        scanf("%d", &menu_item);

    switch(menu_item){
    case 1:
        registerEmployee();
        break;
    case 2:
        modifyEmployee();
        break;
    case 3:
        deleteEmployee();
        break;
    case 4:
        printList();
        break;
    case 5:
        printDaily();
        break;
    case 6:
        pipeWork();
        break;
    case 7:
        exit(0);
        break;
    default:
        printf("Invalid Input! Try again! \n");

    }

}while(menu_item != 0);

}

//==================================================================================
//==================================================================================

int main() {
  menu();
  return 0;
}



























