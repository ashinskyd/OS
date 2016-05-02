//file created by David Ashinsky

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**Function Declarations **/
void *Reader();
void *Writer();
void delay(int);

/**Synchronization variables **/
int activeWriters = 0; //Number of active writers
int activeReaders = 0; //Number of active readers
int waitingWriters = 0; //Number of waiting writers
int waitingReaders = 0; //Number of Waiting readers
int numWritesInARow = 0; //Number of writers who went in a row


pthread_mutex_t lock; //
pthread_cond_t okToRead; //CV used by readers
pthread_cond_t okToWrite; //CV used by writer

int main()
{
    //Initialize the mutex/cv
    pthread_mutex_init(&lock,NULL);
    pthread_cond_init(&okToWrite,NULL);
    pthread_cond_init(&okToRead,NULL);
    
    //sample test code
    pthread_t mathProf1,mathProf2,csProf1,csProf2;
    /*pthread_create(&mathProf1, NULL, mathProfArrive,NULL);
    pthread_create(&mathProf2, NULL, mathProfArrive,NULL);
    pthread_create(&csProf1, NULL, csProfArrive,NULL);
    pthread_create(&csProf1, NULL, csProfArrive,NULL);
    
    
    pthread_join(mathProf1,NULL);
    pthread_join(mathProf2,NULL);
    pthread_join(csProf1,NULL);
    pthread_join(csProf2,NULL);
    */
    
    //destroy the mutex/cv
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&okToRead);
    pthread_cond_destroy(&okToWrite);
    return 0;
}

void *Reader()
{
    pthread_mutex_lock(&lock);
    while (activeWriters + waitingWriters > 0)
    {
        waitingReaders ++;
        pthread_cond_wait(&okToRead, &lock);
        waitingReaders --;
    }
    activeReaders ++;
    pthread_mutex_unlock(&lock);
    
    printf("Reading Stuff Here...\n");
    fflush(stdout);
    
    pthread_mutex_lock(&lock);
    activeReaders --;
    pthread_mutex_unlock(&lock);
    
    return (void *)0;
}

void *Writer()
{
    pthread_mutex_lock(&lock);
    while (activeWriters + activeReaders > 0)
    {
        waitingWriters ++;
        pthread_cond_wait(&okToWrite, &lock);
        waitingWriters --;
    }
    activeWriters ++;
    pthread_mutex_unlock(&lock);
    
    printf("Writing Stuff Here...\n");
    fflush(stdout);
    
    pthread_mutex_lock(&lock);
    activeWriters --;
    numWritesInARow ++;
    
    if (numWritesInARow %% 3 == 0 && waitingReaders > 0)
    {
        //if 3 writes occured in a row, then signal a reader instead and reset the writesInARow var
        numWritesInARow = 0;
        pthread_cond_signal(&okToRead);
    }
    else if(waitingWriters > 0)
    {
        pthread_cond_signal(&okToWrite);
    }
    else if (waitingReaders > 0)
    {
        pthread_cond_broadcast(&okToRead);
    }
    pthread_mutex_unlock(&lock);
    
    return (void *)0;
}

void delay(int n)
{
    for (int i=0; i<n; i++)
    {
        for (int j=0; j<n; j++)
        {
            
        }
    }
}