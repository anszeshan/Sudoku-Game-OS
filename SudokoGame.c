//Ans Zeshan Shahid 20I-0543
//OS  Project 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

//declaration
#define ARRAY_SIZE 9
#define ROW_INDEX 9
#define COL_INDEX 10
#define NUM_THREADS 11

#define LESSER_THAN_0 -1
#define GREATER_THAN_9 -2
#define moves 7
#define ROW  1
#define COL  2
#define BOX  3

//making struct parameter which has number of rows and colomns and index which is used in game
struct parameters{
	int row;
	int col;
	int(*sudoku)[ARRAY_SIZE];
	int index;
};
//making struct which has number of rows and colomns and dimensions which is used for invalid entry in sudoku puzzle
struct InvalidEntry{
	int row;
	int col;
	int reason;
	int dimension;
};
//making struct for checking cancelled thread 
struct CancelledPair{
	int cancellor;
	int cancelled;
};

//initialization for valid row, column
int isRowValid[ARRAY_SIZE] = {0};
int isColValid[ARRAY_SIZE] = {0};
int isValid[NUM_THREADS] = {0};

//declaration and initialization of variable
int invalidEntryCounter = 0;
int cancelledCounter = 0;
struct InvalidEntry* invalidEntries[ARRAY_SIZE * ARRAY_SIZE] = {};
struct CancelledPair* cancelledPairs[ARRAY_SIZE] = {};

//for mutex
pthread_mutex_t mutex;
//number of threads
pthread_t threadIDs[NUM_THREADS];


//function for checking rows
void * check_rows(void * _args){
	struct parameters* args = (struct parameters*) _args;
	  //declaration and initialization of variable
	int validRow = 0;
	int allValidRows = 1;
	       //for loop till array size checking row with invalid entry
	for (int i = 0; i< ARRAY_SIZE; i++){
		int count[10] = {};
		
		validRow = 1;
		for(int j = 0;j < ARRAY_SIZE;j++){
			if((args->sudoku[i][j] < 0) || (args->sudoku[i][j] > 9) || (count[args->sudoku[i][j]] > 0)){
				validRow = 0;
				
				pthread_mutex_lock(&mutex);
				
				bool match = false;
				for(int k = 0;k<invalidEntryCounter; k++){
					if (invalidEntries[k]->row == i && invalidEntries[k]->col == j){
						match = true;
						break;
					}
				}
				if(!match){
					invalidEntries[invalidEntryCounter] = (struct InvalidEntry*) malloc(1 * sizeof(struct InvalidEntry));
					invalidEntries[invalidEntryCounter]->row = i;
					invalidEntries[invalidEntryCounter]->col = j;
					
					if(args->sudoku[i][j] < 0){
						invalidEntries[invalidEntryCounter]->reason = LESSER_THAN_0; 
					}
					else if(args->sudoku[i][j] > 9){
						invalidEntries[invalidEntryCounter]->reason = GREATER_THAN_9; 
					}
					else{
						invalidEntries[invalidEntryCounter]->reason = args->sudoku[i][j]; 
					}
					
					invalidEntries[invalidEntryCounter]->dimension = ROW;
					invalidEntryCounter++;
					
					
					
					int start = floor(i/3) * 3;
					
					for(int k = 0; k < 3; k++){
						cancelledPairs[cancelledCounter] = (struct CancelledPair*) malloc (1 * sizeof(struct CancelledPair));
						cancelledPairs[cancelledCounter]->cancellor = ROW_INDEX;
						cancelledPairs[cancelledCounter]->cancelled = start + k;
						cancelledCounter++;
						//pthread_cancel(threadIDs[start + k]);
					}
					
				}
				
				pthread_mutex_unlock(&mutex);
				//break;
			}
			else{
				count[args->sudoku[i][j]] = 1;
			}
			//printf("%d ",args->sudoku[i][j]);
		}

		//printf("\n");
		isRowValid[i] = validRow;
		allValidRows *= validRow;
		//printf("%d",allValidRows);
		/*if(validRow){
			isValid[args->index] = 1; 
		}*/
	}
	isValid[args->index] = allValidRows;
	pthread_exit(NULL);
}
//function for checking columns
void * check_cols(void * _args){
	struct parameters* args = (struct parameters*) _args;
	  	  //declaration and initialization of variable
	int validCol = 0;
	int allValidCol = 1;
	  	       //for loop till array size checking columns with invalid entry
	for (int i = 0; i< ARRAY_SIZE; i++){
		int count[10] = {0};
		validCol = 1;
		
		for(int j = 0;j < ARRAY_SIZE;j++){
		
			if((args->sudoku[j][i] < 0) || (args->sudoku[j][i] > 9) || (count[args->sudoku[j][i]] > 0)){
				validCol = 0;
				
				pthread_mutex_lock(&mutex);
				
				bool match = false;
				for(int k = 0;k<invalidEntryCounter; k++){
					if (invalidEntries[k]->row == j && invalidEntries[k]->col == i){
						match = true;
						break;
					}
				}
				
				if(!match){
					invalidEntries[invalidEntryCounter] = (struct InvalidEntry*) malloc(1 * sizeof(struct InvalidEntry));
					invalidEntries[invalidEntryCounter]->row = j;
					invalidEntries[invalidEntryCounter]->col = i;
					
					if(args->sudoku[j][i] < 0){
						invalidEntries[invalidEntryCounter]->reason = LESSER_THAN_0; 
					}
					else if(args->sudoku[j][i] > 9){
						invalidEntries[invalidEntryCounter]->reason = GREATER_THAN_9; 
					}
					else{
						invalidEntries[invalidEntryCounter]->reason = args->sudoku[j][i]; 
					}
					
					invalidEntries[invalidEntryCounter]->dimension = COL;
					invalidEntryCounter++;
					
					int start = floor(i/3);
					
					for(int k = 0; k < 3; k++){
						cancelledPairs[cancelledCounter] = (struct CancelledPair*) malloc (1 * sizeof(struct CancelledPair));
						cancelledPairs[cancelledCounter]->cancellor = COL_INDEX;
						cancelledPairs[cancelledCounter]->cancelled = start + 3*k;
						cancelledCounter++;
						//pthread_cancel(threadIDs[start + 3*k]);
					}
				}
				
				pthread_mutex_unlock(&mutex);
				//break;
			}
			else{
				count[args->sudoku[j][i]] = 1;
			}
		}
		
		isColValid[i] = validCol;
		allValidCol *= validCol;
		
		/*if(validRow){
			isValid[args->index] = 1; 
		}*/
	}
	
	isValid[args->index] = allValidCol;
	pthread_exit(NULL);
}

//matrix checking
void * check3x3Matrix(void * _args){
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	struct parameters* args = (struct parameters*) _args;
	int start_row = args->row;
	int start_col = args->col;
	
	bool validMatrix = true;
	int count[10] = {0};
	            	       //for loop checking matrix with invalid entry
	for (int i = start_row; i < start_row + 3; i++){
		for(int j = start_col;j < start_col + 3;j++){
			if((args->sudoku[i][j] < 0) || (args->sudoku[i][j] > 9) || (count[args->sudoku[i][j]] > 0)){
				validMatrix = false;
				
				pthread_mutex_lock(&mutex);
				
				bool match = false;
				for(int k = 0;k<invalidEntryCounter; k++){
					if (invalidEntries[k]->row == i && invalidEntries[k]->col == j){
						match = true;
						break;
					}
				}
				if(!match){
					invalidEntries[invalidEntryCounter] = (struct InvalidEntry*) malloc(1 * sizeof(struct InvalidEntry));
					invalidEntries[invalidEntryCounter]->row = i;
					invalidEntries[invalidEntryCounter]->col = j;
					
					if(args->sudoku[i][j] < 0){
						invalidEntries[invalidEntryCounter]->reason = LESSER_THAN_0; 
					}
					else if(args->sudoku[i][j] > 9){
						invalidEntries[invalidEntryCounter]->reason = GREATER_THAN_9; 
					}
					else{
						invalidEntries[invalidEntryCounter]->reason = args->sudoku[i][j]; 
					}
					
					invalidEntries[invalidEntryCounter]->dimension = BOX;
					invalidEntryCounter++;
				}
				
				pthread_mutex_unlock(&mutex);
				//break;
			}
			else{
				count[args->sudoku[i][j]] = 1;
			}
			pthread_testcancel();
		}
		
		if (!validMatrix){
			break;
		}		
	}
	
	if(validMatrix){
		isValid[args->index] = 1;
	}
	
	pthread_exit(NULL);
}
 
//driven code
int main(){
           //9x9 matrix for sudoku
	int sudoku[9][9] = {
        {1, 2, 4, 5, 3, 9, 1, 8, 7},
        {5, 1, 9, 7, 2, 8, 6, 3, 4},
        {8, 3, 7, 6, 1, 4, 2, 9, 5},
        {1, 4, 3, 8, 6, 5, 7, 2, 9},
        {9, 5, 8, 2, 4, 7, 3, 6, 1},
        {7, 6, 2, 3, 9, 1, 4, 5, 8},
        {3, 7, 1, 9, 5, 6, 8, 4, 2},
        {4, 9, 6, 1, 8, 2, 5, 7, 3},
        {2, 8, 5, 4, 7, 3, 9, 1, 6}
	};
	static volatile size_t counting_threadID = 0;
	
	 // Create Thread arguments
	struct parameters* args  = (struct parameters*) malloc(NUM_THREADS * sizeof(struct parameters));
	
	int index = 0;
	for(int i = 0;i<ARRAY_SIZE;i = i + 3){
		for(int j = 0; j<ARRAY_SIZE;j = j + 3){
			args[index].row = i;
			args[index].col = j;
			args[index].sudoku = sudoku;
			args[index].index = index;
			index++;
		}
	}
	args[ROW_INDEX].sudoku = sudoku;
	args[COL_INDEX].sudoku = sudoku;
	args[ROW_INDEX].index = ROW_INDEX;
	args[COL_INDEX].index = COL_INDEX;
	
	//struct parameters* arg_rows =  (struct parameters*) malloc(1 * sizeof(struct parameters));
	 
	//struct parameters* arg_cols =  (struct parameters*) malloc(1 * sizeof(struct parameters));
	/*arg_rows->sudoku = sudoku;
	arg_rows->index = ROW_INDEX;
	
	arg_cols->sudoku = sudoku;
	arg_cols->index = COL_INDEX;*/
	
	  //Initialize mutex
	if(pthread_mutex_init(&mutex, NULL)){
		printf("Error: Mutex Initialization Failed\n");
		exit(1);
	}
	
	  //Create Threads
	for(int i = 0;i < ARRAY_SIZE; i++){
		if(pthread_create(&threadIDs[i], NULL, check3x3Matrix, (void*) &args[i])){
			printf("Error: Thread Creation Failed\n");
			exit(1);
		}
		//check3x3Matrix((void*) &args[i]);
	}
	
	if(pthread_create(&threadIDs[ROW_INDEX], NULL, check_rows, (void*) &args[ROW_INDEX])){
			printf("Thread Creation Failed\n");
			exit(1);
	}
	
	if(pthread_create(&threadIDs[COL_INDEX], NULL, check_cols, (void*) &args[COL_INDEX])){
			printf("Thread Creation Failed\n");
			exit(1);
	}
	
	// Wait for Threads to finish execution
	for(int i = 0; i < NUM_THREADS; i++){
		pthread_join(threadIDs[i], NULL);
	}
	
	pthread_mutex_destroy(&mutex);
	
	bool validState = true;
	for(int i = 0;i<ARRAY_SIZE; i++){
		if(!isValid[i]){
			validState = false;
			break;
		}
	}
	if(validState){
		printf("The Soduko Board is in a valid state\n");
		return 0;
	}
	int choice; 
	  //menu 
	printf("\n\n\t\t---------- WELCOME IN OPERATING SYSTEM PROJECT ----------\n");
	printf("\t\t\t  ---------- SEMESTER 5TH ----------\n");
	printf("\n\t\t ---------- MADE BY ANS & HAMMAD ----------\n");
	printf("\n\t\t   ---------- PROJECT STARTING ----------\n");
	printf("\nPress 1 for Phase 1\nPress 2 for Phase 2\nPress 0 for Exit\n");
	printf("\nEnter your choice which you want : ");
	scanf("%d", &choice);
	int valueCheck, a,b,c, tts;
	       
	   //display Outputs on screen of Phase 1
	 while(true){
	   if(choice == 1){
	   	printf("\n\n\t\t---------- WELCOME IN PHASE 1 OF PROJECT ----------\n");
	   	  //menu of phase 1
	       printf("\nPress 1 for Invalid Boxes\nPress 2 for Invalid Rows\nPress 3 for Invalid Columns\nPress 4 for Total Invalid Enteries\nPress 5 for Invalid Entries with Rows, Columns and Box with comparison\nPress 6 for Cancelled Thread Details\nPress 0 for Exit\n\n");
	printf("\nEnter your choice which you want : ");
	  scanf("%d", &valueCheck);
	     //press Press 1 for Invalid Boxes print
	  if(valueCheck == 1){
	printf("\n\n------------------------------------------- BOXES -------------------------------------------\n\n");
	for(int i = 0; i< ARRAY_SIZE; i++){
		//printf("%d\n", args[i].sudoku[args[i].row][args[i].col]);	
		if(isValid[i] == 0){
			int box_start_row = args[i].row;
			int box_start_col = args[i].col;
			for(int j = box_start_row; j < box_start_row + 3; j++){
				for(int k = box_start_col; k < box_start_col + 3; k++){
					printf("%d(%d,%d) ", sudoku[j][k], j, k);
				}
				printf("\n");
			}
			printf("Caluclated By: %ld\n", threadIDs[i]);
			printf("--------------------------\n");
		}
	}
	}
	    //Press 2 for Invalid rows
	else if(valueCheck == 2){
	printf("-------------------------------------------- ROWS -------------------------------------------\n");
	for(int i = 0; i < ARRAY_SIZE; i++){		
		if(!isRowValid[i]){
			printf("Row Number: %d, Calculate By: %ld\n", i, threadIDs[ROW_INDEX]);
			for(int j = 0;j < ARRAY_SIZE; j++){
				printf("%d ",sudoku[i][j]);
			}
			printf("\n--------------------------------------------\n");
		}
	}
	}
	   //Press 3 for Invalid Columns print
	 else if(valueCheck == 3){
	printf("-------------------------------------------- COLS -------------------------------------------\n");
	for(int i = 0; i < ARRAY_SIZE; i++){		
		if(!isColValid[i]){
			printf("Col Number: %d, Calculate By: %ld\n", i, threadIDs[COL_INDEX]);
			for(int j = 0;j < ARRAY_SIZE; j++){
				printf("%d ",sudoku[j][i]);
			}
			printf("\n--------------------------------------------\n");
		}
	}
	}
	   //Press 4 for Total Invalid Enteries on screen
	 else if(valueCheck == 4){
	  	printf("\n\n--------------------------------------Total Invalid Entries --------------------------------------\n\n\n");
	  		printf("Total Invalid Entries : %d\n", invalidEntryCounter);
	  		                tts = invalidEntryCounter*3;
	  	}
	  	//Press 5 for Total Invalid Enteries print on screen with comparison
	  	else if(valueCheck == 5){	
	printf("-------------------------------------- Invalid Entries --------------------------------------\n");

	for(int i = 0; i<invalidEntryCounter; i++){
		int rowNum = invalidEntries[i]->row;
		int colNum = invalidEntries[i]->col;
		int dimension = invalidEntries[i]->dimension;
		int reason = invalidEntries[i]->reason;
		
		 
		printf("Value: %d\n", sudoku[rowNum][colNum]);
		printf("Index: (%d, %d)\n", rowNum, colNum);
		printf("Reason: ");
		if(reason == LESSER_THAN_0){
			printf("< 0\n");
		}
		else if(reason == GREATER_THAN_9){
			printf("> 9\n");
		}
		else{
			printf("Repeated Value of %d\n", sudoku[rowNum][colNum]);
		}
		
		printf("In: ");
		if(dimension == ROW){
			printf("Row\n");
		}
		else if(dimension == COL){
			printf("Column\n");
		}
		else{
			printf("3x3 Matrix\n");
		}
		printf("\n--------------------------------------------\n");
	}
	}
	//Press 6 for Cancelled Thread Details print on screen
		  else if(valueCheck == 6){
	printf("------------------------------------ Cancelled Threads------------------------------------\n");
	for(int i = 0; i < cancelledCounter;i++){
		printf("Cancellor: %d, Cancelled: %d\n", cancelledPairs[i]->cancellor, cancelledPairs[i]->cancelled);
	}
	}
	//for exit
	else if(valueCheck == 0){
	                printf("\n\nProgram Ended. Thank You for Running and Checking !!!\n\n");
	                return 0;
	}
	//if user enter invalid input
	else {
	              		printf("\n\nInvalid Input. Try Again with correct Input !!!\n\n");
	}
	
	
	}
	  	   //display Outputs on screen of Phase 1
	else if(choice == 2){
	       int valueCheck2;
	         //menu of phase 2
	   	printf("\n\n\t\t---------- WELCOME IN PHASE 2 OF PROJECT ----------\n");
	       printf("\nPress 1 for Orginal and Resultant Matrix\nPress 2 for Total Invalid Entries\nPress 3 for Threads Created Again Each Entry\nPress 4 for No. of Moves of Each Invalid Enteries\nPress 5 for IDs of All the Threads with ith Index\nPress 6 for Location of Invalid Entry and its Position\nPress 0 for Exit\n\n");
	printf("\nEnter your choice which you want : ");
	  scanf("%d", &valueCheck2);
	    //Press 1 for Orginal and Resultant Matrix print on screen
	  if(valueCheck2 == 1){
	          	  printf("\n\n-------------------------------------- Original Matrix --------------------------------------\n\n\n");
	          	  
	          	  for(int i=0;i<9;++i){
			for(int j =0; j<9;++j)
			   printf(" %d", sudoku[i][j]);
		   printf("\n");
		}  
	           printf("\n\n-------------------------------------- Resultant Matrix --------------------------------------\n\n\n");
	           for(int i = 0; i<9;i++){
			a = sudoku[i][3-1];
			sudoku[i][3-1] = sudoku[i][4-1];
			sudoku[i][4-1] = a;
		}
		for(int i=0;i<9;++i){
			for(int j =0; j<9;++j)
			   printf(" %d", sudoku[i][j]);
		   printf("\n");
		}  
	  }
	  //Press 2 for Total Invalid Entries print on screen
	  else if(valueCheck2 ==2){
	  printf("\n\n--------------------------------------Total Invalid Entries --------------------------------------\n\n\n");
	  		printf("Total Invalid Entries : %d\n", invalidEntryCounter);
	  		 tts = invalidEntryCounter*3;
	  }
	  //Press 3 for Threads Created Again Each Entry print on screen
	  else if(valueCheck2 ==3){
	        printf("\n\n--------------------------------------Total Threads Created Again Each Entry --------------------------------------\n\n\n");
	  		printf("Total Invalid Entries : %d\n", invalidEntryCounter*3);
	  }
	  //Press 4 for printing of number of moves 
	  else if(valueCheck2 ==4){
	  	  printf("\n\n--------------------------------------Total No. of Moves --------------------------------------\n\n\n");
	  	  printf("Total Number of Moves : %d", moves);
	  }
	  //Press 5 for IDs of All the Threads with ith Index
	  else if(valueCheck2 ==5){
	  	        printf("\n\n-------------------------------------- IDS of All Threads with Index--------------------------------------\n\n\n");

	  int a;long b;
	  for(a=0;a<NUM_THREADS;a++){
		  long id; 
		  id = a;
		  printf("Thread %ld and ID of Thread %lu\n ", id, pthread_self());
		  printf("Index : %d\n", a);
		}
	/*  int id=0;
	           for(int i = 0; i<counting_threadID; i++){
                       printf("ThreadID with index :  %d\n" ,threadIDs);
		}*/
	  }
	  //Press 6 for Location of Invalid Entry and its Position on screen
	  else if(valueCheck2 ==6){
	      for(int i = 0; i<invalidEntryCounter; i++){
		int rowNum = invalidEntries[i]->row;
		int colNum = invalidEntries[i]->col;
		int dimension = invalidEntries[i]->dimension;
		int reason = invalidEntries[i]->reason;
		
		 
		printf("Value: %d\n", sudoku[rowNum][colNum]);
		printf("Index: (%d, %d)\n", rowNum, colNum);
		printf("Reason: ");
		if(reason == LESSER_THAN_0){
			printf("< 0\n");
		}
		else if(reason == GREATER_THAN_9){
			printf("> 9\n");
		}
		else{
			printf("Repeated Value of %d\n", sudoku[rowNum][colNum]);
		}
		
		printf("In: ");
		if(dimension == ROW){
			printf("Row\n");
		}
		else if(dimension == COL){
			printf("Column\n");
		}
		else{
			printf("3x3 Matrix\n");
		}
		printf("\n--------------------------------------------\n");
	}
	  }
	  //for exit
	  else if(valueCheck2 == 0){
	                printf("\n\nProgram Ended. Thank You for Running and Checking !!!\n\n");
	                return 0;
	}
	//invalid input
	  else{
              		printf("\n\nInvalid Input. Try Again with correct Input !!!\n\n");
    }
	}
	
	
	
	//for exit
	else if(choice == 0){
	                printf("\n\nProgram Ended. Thank You for Running and Checking !!!\n\n");
	                return 0;
	}
	//invalid input
	else{
              		printf("\n\nInvalid Input. Try Again with correct Input !!!\n\n");
    }
    }
}
