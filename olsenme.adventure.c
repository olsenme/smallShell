#include<sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
 #include <errno.h> 
 #include <string.h> 

#define NUM_ROOMS  7

pthread_t threads[2]; 
pthread_mutex_t lock;
pthread_cond_t is_zero;

struct Room{
char name[20];
char connections[7][20];
int numConnections;
char type[20];
};

struct Room array[7];
char working_dir[100];
int flag=0;


void openDirectory();
void readContents();
void playGame(); 
int findStartRoom();
int findNextRoom(const char *room);
void* writeTime(void *param);
void readTime();

/*opens the project directory,finds rooms directory,chooses rooms directory with most recently created files*/
void openDirectory(){

char *fd = "olsenme.rooms.";
char currentDir[100];
DIR *d;
int lastModified;
struct dirent *dp;
struct stat *buffer;
int newest = 0;
buffer = malloc(sizeof(struct stat));
dp = malloc(sizeof(struct dirent));   
memset(currentDir, '\0', sizeof(currentDir));
memset(working_dir, '\0', sizeof(working_dir));


getcwd(currentDir, sizeof(currentDir));  //get project directory

d = opendir(currentDir); //open it

//make sure it opened
if(d==NULL){
printf("Directory Error:\n");
}
   while ((dp= readdir(d))) {  //read from it
    //make sure it's not empty
	 if (strstr(dp->d_name,fd) != NULL){
     //run stat
	  stat(dp->d_name, buffer);
      //get time
	  lastModified = (int)buffer->st_mtime;
      //clear array
    //  memset(working_dir, '\0', sizeof(working_dir));
    //reset
    //  strcpy(working_dir,dp->d_name);

       //check for latest modification date
      if(lastModified> newest)
      {
         //set newest modification date
          newest = lastModified;
          memset(working_dir, '\0', sizeof(working_dir));
          //save directory as most recent
          strcpy(working_dir,dp->d_name);
      }
  }
}
  free(buffer);
  
  if(closedir(d)<0)
  {
  printf("Error closing directory.");  
  }

}
/*opens the rooms files,extracts the info, 
and imports it into the program*/
void readContents(){
     
     DIR *d;
     struct dirent *dp;
     char fName[20];
	   int hasName;
     char filePath[45];
     ssize_t read;
     int i;
   	 FILE* file_desc;
     size_t len = 0;
     char* line;
     char* token;
     token=malloc(20*sizeof(char));    
     line=malloc(20*sizeof(char)); 
     dp = malloc(sizeof(struct dirent)); 
     i=0;
     
     
    //open directory
		d = opendir(working_dir);
		if (d == NULL) {
			fprintf(stderr, "Could not open directory %s\n", working_dir);
			perror(working_dir);
			exit(1);
		}
		//read all the files
	  while((dp= readdir(d))) {
		  memset(fName, '\0', sizeof(fName));
		  sprintf(fName,"%s",dp->d_name);
		  if(fName[0] == '.') {
			  continue;
		  }
		  hasName=0;
		  memset(filePath, '\0', sizeof(filePath));
		  sprintf(filePath, "%s/%s",working_dir, fName);
		  //open the file
      file_desc = fopen(filePath, "r");
		  if (file_desc == NULL) {
			  fprintf(stderr, "Could not open file");
			  exit(1);
		  }
        
		  array[i].numConnections =0;
		  memset(array[i].type, '\0', 20);

		  while(read=(getline(&line, &len,file_desc))!= -1) {
			  sprintf(token,strtok(line, ":"));
			  sprintf(token,strtok(NULL, "\n"));
			  if(hasName == 1) {
				  if(line[0]=='C') { 
					  strcpy(array[i].connections[array[i].numConnections], token+1);
					  array[i].numConnections++;
				  }
				  else {
					  strcpy(array[i].type,token+1);
				  }
			  }
			  else {
				  memset(array[i].name,'\0', sizeof(array[i]));
				  strcpy(array[i].name,token+1);
				  hasName=1;
			  }
		  }
		  fclose(file_desc);
		  i++;
	 
  }
  free(line);
  free(token);
  if(closedir(d)<0)
  {
  printf("Error closing directory.");  
  }
}
/*plays the game*/
void playGame(){
/*********************************GAME LOGIC****************************************************************/
 int i,j,c;
 int nextRoomIndex;
 int numCharsEntered;
 int found;
 int startRoomIndex; 
 struct Room cur_room;
 char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
 size_t bufferSize; // Holds how large the allocated buffer is 
 int num_steps;
 int steps[8];
 char path_to_vic[8][20]={{0}};
 char* end;
 
  memset(steps, '\0', sizeof(steps));
	numCharsEntered= -5; // How many chars we entered
  j=0;
  startRoomIndex = findStartRoom(); 
  cur_room = array[startRoomIndex];
  bufferSize=0;  // how large the allocated buffer is
  end= malloc(20 * sizeof(char));
  sprintf(end,"END_ROOM");
  num_steps=0; 
  lineEntered = malloc(20 * sizeof(char));
   
	while(strcmp(cur_room.type,"END_ROOM"))
	{
   //print current location and info
	 printf("CURRENT LOCATION: %s\n", cur_room.name);
 	 printf("POSSIBLE CONNECTIONS: ");
	 for (i = 0; i < cur_room.numConnections; i++)
	{
       if(i+1!=cur_room.numConnections){
			 printf("%s,", cur_room.connections[i]);
        }
        else {
         printf("%s.\n", cur_room.connections[i]);
        }
	}
  //get input from user
	printf("WHERE TO? >");
	numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
 //remove newline
	lineEntered[numCharsEntered - 1] = '\0';   
  if(strcmp(lineEntered,"time")==0){
    //release the mutex
    pthread_mutex_unlock(&lock);
     usleep(50);
     pthread_mutex_lock(&lock);
    // pthread_cond_signal(&is_zero);
     readTime();
  }
  //check if it's valid
  else if(inConnections(cur_room,cur_room.numConnections,lineEntered)<0){
    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    continue;
    }
    
  else 
    {
    	// find index of next room
     nextRoomIndex = findNextRoom(lineEntered);
     if(nextRoomIndex<0){
     continue;
     }
      else {
     //change to next room
     cur_room = array[nextRoomIndex]; 
     strcpy(path_to_vic[num_steps++],cur_room.name);
     printf("\n");
    //printf("Path to vic:%s",path_to_vic[j]);
            }
    
   }
  free(lineEntered);
  lineEntered = NULL;
  bufferSize=0;
  }
      printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
      printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
	   // Print path taken
     for (i = 0; i < num_steps; i++)
	  {
	   printf("%s\n", path_to_vic[i]);
	  }
   free(end);
   flag = 1; 

}
/*thread function to write to text file*/
void* writeTime(void *arg){

time_t t; 
struct tm *timeInfo;  

char buff[50];
char fileName[50];
const char* file = "currentTime.txt";
memset(fileName, '\0', sizeof(fileName));
//int *flag = (int*)flag;

 while(flag==0){
  //pthread_cond_wait(&is_zero, &lock); 
  pthread_mutex_lock(&lock);
  if(flag ==1){
  break;
  }
//}
// while(flag ==1){
 //grab the mutex
  
 //open up file
  //create name of text file
  // sprintf(fileName,"%s",file);
  memset(fileName, '\0', sizeof(fileName));
  getcwd(fileName, sizeof(fileName));  //get project directory
  sprintf(fileName,"%s",file);
  FILE * file_desc = fopen("currentTime.txt","w+");
 if(file_desc == NULL){
 perror("Error opening time file\n");
 exit(-1);
 }

 
   //get the time
  time(&t);
  timeInfo = localtime(&t);
  strftime(buff,50,"%I:%M,%p %A, %B %d, %Y",timeInfo);
   
   // write formatted time to file then close file
   fputs(buff, file_desc);
   fclose(file_desc);
   memset(buff, '\0', sizeof(buff));
   pthread_mutex_unlock( &lock );
   usleep(50);
   }
   //release the mutex
  // flag =1;
   
return NULL;

}
/* read from text file*/
void readTime(){
       
      char buff[50];
      
      char fileName[50];
      //char currentDir[50];
      const char* file = "currentTime.txt";
      //name text file
      //sprintf(fileName,"%s",file);
      memset(buff, '\0', sizeof(buff));
      memset(fileName, '\0', sizeof(fileName));
      getcwd(fileName, sizeof(fileName));  //get project directory
      sprintf(fileName,"%s",file);
      FILE* file_desc = fopen("currentTime.txt", "r");
		  if (file_desc == NULL) {
			   perror( "Error opening file" );
         printf( "Error code opening file: %d\n", errno );
         printf( "Error opening file: %s\n", strerror( errno ) );
        exit(-1);
		  }
        // read current time into buffer
      fgets(buff, 50, file_desc);
       printf("\n%s\n", buff);
      // close file
       fclose(file_desc);
      
}

/*Searches through all rooms and returns the index of the start room*/
int findStartRoom(){
int i=0;
int index;
for( ;i<NUM_ROOMS;i++){
 if(strcmp(array[i].type,"START_ROOM")==0){
 return i; 
}
 }
 return -4; 
}
/*Searches through all rooms and returns the index of the next room*/
int findNextRoom(const char *room){
int i=0;
int index;
for( ;i<NUM_ROOMS;i++){
 if(strcmp(array[i].name,room)==0){
 return i; 
}
 }
 return -4; 
}
/*checks if room entered is in the rooms list of connections, and if is, returns index where it found it*/
int inConnections(struct Room room, int count,char *val){
int i=0;
for( ;i<NUM_ROOMS;i++){
 if(strcmp(room.connections[i],val)==0)
 return i;
 }
 return -5;
}
 
void test(){
int l; 
int m; 
for (l = 0; l < NUM_ROOMS; l++){
 printf("room Name: %s\n",array[l].name);
 //printf("numConnections:%d\n",array[l].numConnections);
  printf("type:%s\n",array[l].type);
   for(m=0;m<NUM_ROOMS;m++){
    printf("connections:%s\n",array[l].connections[m]);
    }
  }
}

int main() {
      
     //initialize the condition
    // if(pthread_cond_init(&is_zero, NULL)!=0){
    //   printf("Failed to initialize condition");
    // }
     //initialize the mutex/unlock it
     if(pthread_mutex_init(&lock, NULL) != 0){
       printf("Failed to initialize mutex");  }
      //grab the mutex
      pthread_mutex_lock(&lock);
      //create the read thread
     if(pthread_create(&threads[0],NULL,&writeTime,NULL)!=0){
       fprintf(stderr, "Error creating thread\n");
     }
     openDirectory(); 
     readContents();
     //test(); 
     playGame(); 
  
     //join the thread back up
     pthread_join( threads[0], NULL);
  
     //destroy mutex
     pthread_mutex_destroy(&lock);

  

}
