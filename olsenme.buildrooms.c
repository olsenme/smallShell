#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NUM_ROOMS  7
#define MAX_CONNECTIONS 6
#define MIN_OUT 3

char ROOM_NAMES[10][16] =
{
  "Great Hall",
  "Great Chamber",
  "Cabinet",
  "Place of Arms",
  "Buttery",
  "Chapel",
  "Pantry",
  "Kitchen",
  "Bathroom",
  "Oratory"
};
char* ROOM_TYPE[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"}; 
struct Room{         
    int number;
    char* type;
    int connections[MAX_CONNECTIONS];
    int currNumConnections;   
};
char roomDirName[64];
struct Room rooms[NUM_ROOMS];
int chosen[NUM_ROOMS];

void initRooms();
void makeDirectory();
void setRooms();
void makeConnections();
void writeFiles();
int isConnected(const int *connections,int count,int value);


/*initializes the connection array for each of the rooms,sets current and random number of connections to 0*/
void initRooms()
{
   int i; 
   int k; 
    
   for(i = 0; i < NUM_ROOMS; i++)
   {
    memset(rooms[i].connections, '\0', sizeof(rooms[i].connections));
    rooms[i].currNumConnections = 0;     
    
   }
}
/*creates rooms directory*/
void makeDirectory()
{
  //create directory
  int result;
  pid_t myPID = getpid();
  const char* myONID = "olsenme";
  memset(roomDirName, '\0', sizeof(roomDirName));
  sprintf(roomDirName, "%s.rooms.%d", myONID, myPID);
  result = mkdir(roomDirName, S_IRWXU | S_IRWXG | S_IRWXO);
}
/* assigns room names, types, and a random number of connections for each room*/
void setRooms()
{
  int i,j,k,flag,roomIndex,connectionsPerRoom;   
  for(j = 0; j < NUM_ROOMS; j++){ 
     flag = 1;
     while(flag){
        roomIndex = rand() % 10;    
        flag = 0;
        for (k = 0; k < NUM_ROOMS; k++){
            if(rooms[k].number == roomIndex){
                flag = 1;
            }
        }
    }
  rooms[j].number = roomIndex;
  chosen[j] = roomIndex; 
  
  if(j == 0){                          //Assign the type of room per assignment specs
    rooms[j].type = ROOM_TYPE[0];
  }
  else if(j == 1){
    rooms[j].type = ROOM_TYPE[1];
  }
  else{
    rooms[j].type = ROOM_TYPE[2];
  }
 
 }
 makeConnections();
}

/*******************************************************************
Function: isConnected
Arguments: pointer to interger array, number element in integer array, check value
Description: searches array, checks for check value
*********************************************************************/
int isConnected(const int *connections,int count,int value){
  int i=0;
  for(; i <count; i++){
   //printf("Connections array:%d",connections[i]);
    if(connections[i] == value){
       return 1;}
  }
  return 0;   
}

/*Description: makes connections for the room*/

void makeConnections(){
  int randIndexConnectingRoom; 
  int k;
  int j;
  int connections;
  j=0;
  for(j = 0; j < NUM_ROOMS; j++){    
       
      while (rooms[j].currNumConnections < MIN_OUT) {
      
      // if rooms out_count is less than min, get a random number of connectins
      connections = 1 + rand() % MAX_CONNECTIONS;
      // ensure random connections is in range 3 <= connection <= 6
       while (connections < MIN_OUT) {connections = 1 + rand() % 6;}
      // subtract random number of connections from total connections already set
       connections -= rooms[j].currNumConnections;
       for(k=0;k<connections;k++){
       randIndexConnectingRoom = rand() % NUM_ROOMS; //generate random index
       while(isConnected(rooms[j].connections,rooms[j].currNumConnections,rooms[randIndexConnectingRoom].number) || 
       rooms[j].number == rooms[randIndexConnectingRoom].number || !isConnected(chosen,NUM_ROOMS,rooms[randIndexConnectingRoom].number)){            //check for duplicate or existing connection
               randIndexConnectingRoom = rand() % NUM_ROOMS;   
          }                     
          rooms[j].connections[rooms[j].currNumConnections++] = rooms[randIndexConnectingRoom].number;                     //connect
          if(isConnected(rooms[randIndexConnectingRoom].connections,rooms[randIndexConnectingRoom].currNumConnections,j)==0)
              rooms[randIndexConnectingRoom].connections[rooms[randIndexConnectingRoom].currNumConnections++] = rooms[j].number;
            
            }
            
      }
 }
         
}                                            

/*creates rooms files and writes room data to them*/
void writeFiles()
{
        FILE* file_desc;
        char roomFileName[256];
        memset(roomFileName, '\0', sizeof(roomFileName));
        int l,m,k;
        
        //change into the directory to write the files
        chdir(roomDirName);
       
         for (l = 0; l < NUM_ROOMS; l++)
         {
              k=1;
             //create name of text file
             sprintf(roomFileName,"%s.txt",ROOM_NAMES[rooms[l].number]);
             
             //open up file for writing
             file_desc = fopen(roomFileName,"w");
             if(file_desc<0)
             {
                 fprintf(stderr, "Could not open %s\n", file_desc);
                 exit(1);

             }
             
             //create name string to write to file
             sprintf(roomFileName, "ROOM NAME: %s\n",ROOM_NAMES[rooms[l].number]);
          
             //write to file
             fputs(roomFileName, file_desc);
             //write outgoing connections to file
             for(m=0;m<rooms[l].currNumConnections;m++)
             {
                 sprintf(roomFileName,"CONNECTION %d: %s\n",k,ROOM_NAMES[rooms[l].connections[m]]);
                 fputs(roomFileName, file_desc);
                 k++;
                 
             }
               
             sprintf(roomFileName,"ROOM TYPE: %s\n",rooms[l].type);
             fputs(roomFileName, file_desc);
             fclose(file_desc);
         }  
}
void test(){
int l; 
int m; 
for (l = 0; l < NUM_ROOMS; l++){
 printf("room Number: %d\n",rooms[l].number);
 printf("currNumConnections:%d\n",rooms[l].currNumConnections);

  for(m=0;m<rooms[l].currNumConnections;m++){
   printf("%d",rooms[l].connections[m]);
    }
  }
}

int main(){
         // seed random number generator
         srand(time(0));
         initRooms();
         makeDirectory();
         setRooms();
          //test();
         writeFiles();
}
  
