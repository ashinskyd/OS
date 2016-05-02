//file created by David Ashinsky

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**Function Declarations **/
void *mathProfArrive();
void *mathProfLeave();
void *csProfArrive();
void *csProfLeave();
void delay(int);

/**Synchronization variables **/
int sign_value = -1; //Sign value of -1="Avail", 0="CS" (Occupied by CS profs), 1="Math" (Occupied by math profs)
int numCsProfsInRoom = 0; //Only accessible by CS profs who have already entered the room, or are leaving
int numMathProfsInRoom = 0; //Only accessible by Math profs who have already entered the room, or are leaving
pthread_mutex_t sign_mutex; //lock for accessing the sign var and the num of profs vars
pthread_cond_t csProfsCv; //CV used by CS profs
pthread_cond_t mathProfsCv; //CV used by Math profs

int main()
{
    //Initialize the mutex/cv
    pthread_mutex_init(&sign_mutex,NULL);
    pthread_cond_init(&csProfsCv,NULL);
    pthread_cond_init(&mathProfsCv,NULL);
    
    //sample test code
    pthread_t mathProf1,mathProf2,csProf1,csProf2;
    pthread_create(&mathProf1, NULL, mathProfArrive,NULL);
    pthread_create(&mathProf2, NULL, mathProfArrive,NULL);
    pthread_create(&csProf1, NULL, csProfArrive,NULL);
    pthread_create(&csProf1, NULL, csProfArrive,NULL);
    
    
    pthread_join(mathProf1,NULL);
    pthread_join(mathProf2,NULL);
    pthread_join(csProf1,NULL);
    pthread_join(csProf2,NULL);
    
    //destroy the mutex/cv
    pthread_mutex_destroy(&sign_mutex);
    pthread_cond_destroy(&csProfsCv);
    pthread_cond_destroy(&mathProfsCv);
    return 0;
}

void *mathProfArrive()
{
    printf("Math prof arrives\n");
    fflush(stdout);
    
    //acquires the sign lock and check its value
    pthread_mutex_lock(&sign_mutex);
    while (sign_value == 0)
    {
        //if the room is occupied by CS faculty, wait until they have all left
        pthread_cond_wait(&mathProfsCv, &sign_mutex);
    }
    printf("Math prof enters lounge\n");
    fflush(stdout);
    //once in the critical section, set the sign value and increment the # profs in the room
    sign_value = 1;
    numMathProfsInRoom ++;
    pthread_mutex_unlock(&sign_mutex);
    
    //random delay before leaving the room
    delay(rand()%5000);
    mathProfLeave();
    return (void *)0;
}

void *csProfArrive()
{
    printf("CS prof arrives\n");
    fflush(stdout);
    
    //acquires the sign lock and check its value
    pthread_mutex_lock(&sign_mutex);
    while (sign_value == 1)
    {
        //if the room is occupied by Math faculty, wait until they have all left
        pthread_cond_wait(&csProfsCv, &sign_mutex);
    }
    printf("CS prof enters lounge\n");
    fflush(stdout);
    
    //once in the critical section, set the sign value and increment the # profs in the room
    sign_value = 0;
    numCsProfsInRoom ++;
    pthread_mutex_unlock(&sign_mutex);
    
    //random delay before leaving the room
    delay(rand()%5000);
    csProfLeave();
    return (void *)0;
}

void *mathProfLeave()
{
    pthread_mutex_lock(&sign_mutex); //get the sign mutex
    numMathProfsInRoom --;
    
    printf("Math prof leaves lounge\n");
    fflush(stdout);
    
    if (numMathProfsInRoom == 0)
    {
        // if the last one to leave, set the sign to available and notify any waiting CS faculty
        sign_value = -1;
        pthread_cond_broadcast(&csProfsCv);
    }
    pthread_mutex_unlock(&sign_mutex);
    return (void *) 0;
}

void *csProfLeave()
{
    pthread_mutex_lock(&sign_mutex); //get the sign mutex
    numMathProfsInRoom --;
    
    printf("CS prof leaves lounge\n");
    fflush(stdout);
    
    if (numCsProfsInRoom == 0)
    {
        // if the last one to leave, set the sign to available and notify any waiting CS faculty
        sign_value = -1;
        pthread_cond_broadcast(&mathProfsCv);
    }
    pthread_mutex_unlock(&sign_mutex);
    return (void *) 0;
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