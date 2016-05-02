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
    while ((activeWriters + waitingWriters > 0) && (numWritesInARow % 3 != 0))
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
    if (waitingWriters > 0 && numWritesInARow % 3 == 0)
    {
        pthread_cond_signal(&okToWrite);
    }
    pthread_mutex_unlock(&lock);
    
    return (void *)0;
}

void *Writer()
{
    delay(rand()%5000);
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
    
    if (numWritesInARow % 3 == 0 && waitingReaders > 0)
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