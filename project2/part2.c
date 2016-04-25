//File created by David Ashinsky, Kiley, John Green

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>

pthread_mutex_t children_on_dest_mutex;
pthread_cond_t children_on_dest_cv;
int allPeopleCrossed = 0;

pthread_mutex_t less_than_2_children_mutex;
pthread_cond_t less_than_2_children_cv;
//stores the current location of the boat. 0=source Island 1=Dest island
int less_than_2_children = 0;

pthread_mutex_t boat_location_mutex;
pthread_cond_t boat_location_cv;
//stores the current location of the boat. 0=source Island 1=Dest island
int boat_location = 0;

//stores the passengers onboard the boat. 1 indicates 1 child onboard, 2=2 children onboard, 3=1 adult onboard
int boat_pax = 0;

//stores the number of children/adults on the destination island
int numChildrenOnDest = 0;
int numAdultsOnDest = 0;

//stores the number of children/adults on the source island
int numChildrenOnSource = 0;
int numAdultsOnSource = 0;

//condition variables used to signal/wait when a thread is first created
pthread_mutex_t thread_count_mutex;
pthread_cond_t all_ready_cv;

//Condition variables for signaling that all threads are completely setup and ready to process
pthread_mutex_t proceed_mutex;
pthread_cond_t proceed_cv;
int okToProceed = 0;


void *child(void*);
void *adult(void*);
void *childLogic();
void boatDepartingSource();
void childReturningToSource();
void initSync();
void delay( int limit );

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Enter numChildren, numAdults\n");
		exit(0);
	}

	initSync();

  	const int numChildren = atoi(argv[1]);
  	const int numAdults = atoi(argv[2]);
  	const int total = numChildren+numAdults; 

  	//create all the children/adult threads
  	pthread_t people[total];
  	for (int i=0; i<numChildren; i++) {
    	pthread_create(&people[i], NULL, child, NULL);
    }
    for (int i=numChildren;i<total;i++) {
    	pthread_create(&people[i], NULL, adult, NULL);	
    }

    //while there are not enough chilcren/adult threads created, wait
   	pthread_mutex_lock(&thread_count_mutex);
	while (numChildrenOnSource < numChildren || numAdultsOnSource < numAdults) {
     	printf("Waiting For threads to be created...\n");
      	pthread_cond_wait(&all_ready_cv, &thread_count_mutex);
    }
    //when all threads have been created, allow them to proceed
    pthread_mutex_unlock(&thread_count_mutex);
    
    printf("All Threads Created\n");
    pthread_mutex_lock(&proceed_mutex);
    okToProceed = 1;
    pthread_cond_broadcast(&proceed_cv);
    pthread_mutex_unlock(&proceed_mutex);

    //Temp loop until there's an exit strategy
    while (1){

    }
    return 0;
}

void initSync()
{
	pthread_cond_init (&all_ready_cv, NULL);
	pthread_mutex_init(&thread_count_mutex, NULL);

	pthread_cond_init (&proceed_cv, NULL);
	pthread_mutex_init(&proceed_mutex, NULL);

	pthread_cond_init (&boat_location_cv, NULL);
	pthread_mutex_init(&boat_location_mutex, NULL);

    pthread_cond_init(&less_than_2_children_cv,NULL);
    pthread_mutex_init(&less_than_2_children_mutex,NULL);

    pthread_cond_init(&children_on_dest_cv,NULL);
    pthread_mutex_init(&children_on_dest_mutex,NULL);
}

void *child(void* a){
    delay(rand()%5000);
	//once thread is created, signal that we've added a person to the source island before doing any processing
	printf("Child Thread Creating And Waiting...\n");
	pthread_mutex_lock(&thread_count_mutex);
	numChildrenOnSource ++;
	pthread_cond_signal(&all_ready_cv);
	pthread_mutex_unlock(&thread_count_mutex);
	
	//wait until all threads created before proceeding
 	pthread_mutex_lock(&proceed_mutex);
	while (okToProceed ==0) {
      	pthread_cond_wait(&proceed_cv, &proceed_mutex);
    }
    pthread_mutex_unlock(&proceed_mutex); 

    pthread_mutex_lock(&less_than_2_children_mutex);
    less_than_2_children = (numChildrenOnSource < 2);
    pthread_mutex_unlock(&less_than_2_children_mutex);
    return childLogic();
}

void *childLogic(){
    delay(rand()%5000);
    //wait until the boat is at the start location before boarding
    pthread_mutex_lock(&boat_location_mutex);
    while (boat_location==1){
        pthread_cond_broadcast(&boat_location_cv);
        pthread_cond_wait(&boat_location_cv,&boat_location_mutex);
    }

    delay(rand()%5000);
    //once the boat is at the source island, proceed with boarding
    if (numChildrenOnSource == 1){
        if (numAdultsOnSource!=0){
            pthread_mutex_unlock(&boat_location_mutex);
            return childLogic();
        }
        printf("Child Is boarding the boat\n");
        boat_pax = 1;
        boatDepartingSource();

        pthread_mutex_lock(&children_on_dest_mutex);
        allPeopleCrossed = 1;
        pthread_cond_broadcast(&children_on_dest_cv);

        pthread_mutex_unlock(&children_on_dest_mutex);
        pthread_mutex_unlock(&boat_location_mutex);
        return (void *)0;
    }else if (numChildrenOnSource == 2){
        if (boat_pax==0)
        {
            printf("Child is boarding the boat\n");
            boat_pax = 1;

            pthread_mutex_lock(&children_on_dest_mutex);
            while (!allPeopleCrossed){
                pthread_mutex_unlock(&boat_location_mutex);
                pthread_cond_wait(&children_on_dest_cv,&children_on_dest_mutex);    
                pthread_mutex_lock(&boat_location_mutex);
            }
            pthread_mutex_unlock(&boat_location_mutex);
            return (void *)0;
        }else if (boat_pax ==1){
            boat_pax = 2;
            printf("Child is boarding the boat\n");
            boatDepartingSource();
            childReturningToSource();
            pthread_mutex_unlock(&boat_location_mutex);
            return childLogic();
        }
    }else{
        if (boat_pax==0)
        {
            printf("Child is boarding the boat\n");
            boat_pax = 1;
            pthread_cond_wait(&boat_location_cv,&boat_location_mutex);
            pthread_mutex_unlock(&boat_location_mutex);
            return (void *)0;
        }else if (boat_pax ==1){
            boat_pax = 2;
            printf("Child is boarding the boat\n");
            boatDepartingSource();
            childReturningToSource();
            pthread_mutex_unlock(&boat_location_mutex);
            return childLogic();
        }
    }
    pthread_mutex_unlock(&boat_location_mutex);
    return (void *)0;
}

void *adult(void* a){
	//once thread is created, signal that we've added a person to the source island before doing any processing
	printf("Adult Thread Creating And Waiting...\n");
	pthread_mutex_lock(&thread_count_mutex);
	numAdultsOnSource ++;
	pthread_cond_signal(&all_ready_cv);
	pthread_mutex_unlock(&thread_count_mutex);
	
	//wait until all threads created before proceeding
	pthread_mutex_lock(&proceed_mutex);
	while (okToProceed ==0) {
      	pthread_cond_wait(&proceed_cv, &proceed_mutex);
    }
    pthread_mutex_unlock(&proceed_mutex);

    pthread_mutex_lock(&less_than_2_children_mutex);
    while (!less_than_2_children){
        pthread_cond_wait(&less_than_2_children_cv,&less_than_2_children_mutex);
    }
    printf("Adult boarding boat\n");
    boat_pax = 3;
    boatDepartingSource();
    
    pthread_cond_broadcast(&children_on_dest_cv);
    pthread_mutex_unlock(&less_than_2_children_mutex);
	return (void *)0;
}

void childReturningToSource()
{
    printf("Child boarding the boat on the destination\n");
    printf("Boat is crossing the river back to source\n");
    printf("Boat arrived at source\n");
    printf("Child Arrived at source\n");
    numChildrenOnSource ++;
    numChildrenOnDest --;
    boat_location = 0;
    boat_pax = 0;

    pthread_mutex_lock(&less_than_2_children_mutex);
    less_than_2_children = (numChildrenOnSource < 2);
    if (less_than_2_children)
    {
        pthread_cond_broadcast(&less_than_2_children_cv);
    }
    pthread_mutex_unlock(&less_than_2_children_mutex);
    pthread_cond_broadcast(&boat_location_cv);
}

void boatDepartingSource()
{
    printf("Boat is crossing the river\n");
    printf("Boat arrived at destination\n");
    if (boat_pax==1)
    {
        printf("Child Arrive at destination\n");
        numChildrenOnSource --;
        numChildrenOnDest ++;
    }
    if (boat_pax==2)
    {
        for (int i=0; i<2; i++){
            printf("Child Arrive at destination\n");
            numChildrenOnSource --;
            numChildrenOnDest ++;
        }
    }
    if (boat_pax ==3)
    {
        printf("Adult arrived at destination\n");
        numAdultsOnSource --;
        numAdultsOnDest ++;
    }
    boat_location = 1;
    boat_pax = 0;
    pthread_cond_broadcast(&boat_location_cv);
}

void delay( int limit )
{
  int j, k;

  for( j=0; j < limit; j++ )
    {
      for( k=0; k < limit; k++ )
        {
        }
    }
}