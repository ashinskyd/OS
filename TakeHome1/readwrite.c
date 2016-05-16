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

pthread_mutex_t lock; //
pthread_cond_t okToRead; //CV used by readers
pthread_cond_t okToWrite; //CV used by writer

int starvedReaders = 0; //keeps track of starved readers. That is, whenever a reader has to wait 3 times for a writer, it is "starved"

int main()
{
    //Initialize the mutex/cv
    pthread_mutex_init(&lock,NULL);
    pthread_cond_init(&okToWrite,NULL);
    pthread_cond_init(&okToRead,NULL);

    //sample test code
    pthread_t reader1,reader2,writer1,writer2,writer3,writer4,writer5,writer6,writer7;
    pthread_create(&writer1, NULL, Writer,NULL);
    pthread_create(&writer2, NULL, Writer,NULL);
    pthread_create(&writer3, NULL, Writer,NULL);
    pthread_create(&writer4, NULL, Writer,NULL);

    pthread_create(&reader1, NULL, Reader,NULL);
    pthread_create(&reader2, NULL, Reader,NULL);
    
    pthread_join(reader1,NULL);
    pthread_join(reader2,NULL);
    
    pthread_join(writer1,NULL);
    pthread_join(writer2,NULL);
    pthread_join(writer3,NULL);
    pthread_join(writer4,NULL);
    
    pthread_create(&writer5, NULL, Writer,NULL);
    pthread_create(&writer6, NULL, Writer,NULL);
    pthread_create(&writer7, NULL, Writer,NULL);

   
    pthread_join(writer5,NULL);
    pthread_join(writer6,NULL);
    pthread_join(writer7,NULL);
    
    //destroy the mutex/cv
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&okToRead);
    pthread_cond_destroy(&okToWrite);

    return 0;
}

void *Reader()
{
    delay(rand()%5000);

    pthread_mutex_lock(&lock);
    
    delay(rand()%5000);
    
    int numberOfWaits = 0; //thread local variable to track the number of times this thread has to wait for a writer
    while ((activeWriters + waitingWriters > 0) && numberOfWaits != 3)
    {
        //while the reader is blocked by an active/waiting writer AND it is not yet starved, go to sleep
        waitingReaders ++;
        numberOfWaits ++;
        if (numberOfWaits == 3)
        {
            //If this thread has been a sleep for 3 times, it's time to increment the # starved readers
            starvedReaders ++;
        }
        pthread_cond_wait(&okToRead, &lock);
        printf("Reader waking with waiting writers %i activeWriters %i numberOf Waits %i %i\n",waitingWriters,activeWriters,numberOfWaits,starvedReaders);
        fflush(stdout);
        
        waitingReaders --;
        if (numberOfWaits != 3 && waitingWriters + activeWriters >0)
        {
            pthread_cond_signal(&okToWrite);   
        }
    }
    activeReaders ++;
    pthread_mutex_unlock(&lock);
    
    printf("Reading Stuff Here...\n");
    fflush(stdout);
    
    pthread_mutex_lock(&lock);
    activeReaders --;
    if (numberOfWaits == 3)
    {
        //mark this reader as not starved if it previously was
        starvedReaders --;
    } 

    if (waitingWriters > 0 && starvedReaders==0)
    {
        //If there are waiting writers AND no starved readers, tell the writers to proceed
        //This block will get called when the last starved reader has finished reading
        pthread_cond_signal(&okToWrite);
    }else if (starvedReaders != 0)
    {
        //if there are just starved readers, tell them to go
        pthread_cond_broadcast(&okToRead);
    }

    pthread_mutex_unlock(&lock);
    
    return (void *)0;
}

void *Writer()
{
    delay(rand()%5000);

    pthread_mutex_lock(&lock);
    
    delay(rand()%5000);
    
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
    
    //once reading is complete, if there are starved readers or waiting readers, wake them up
    //This serves 2 purposes: 1- If there are starved readers, they will run
    //And 2 - if there are waiting readers which are not yet starved, they will increment their # waits, and then wake up the next writer if there is one
    if (starvedReaders != 0 || waitingReaders >0)
    {
        pthread_cond_broadcast(&okToRead);   
    }else if(waitingWriters > 0)
    {
        //If there's no starved readers nor waiting readers, wake up the next writer
        pthread_cond_signal(&okToWrite);
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