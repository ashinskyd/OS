//
//  H2SO4.c
//
//
//  Created by David Ashinsky on 4/23/16.
//
//

#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>

#include "H2SO4.h"

//semaphores
sem_t* hydro_sem;
sem_t* hydro_leave_sem;
sem_t* hydro_left_sem;
sem_t* oxygen_sem;
sem_t* oxygen_leave_sem;
sem_t* oxygen_left_sem;
sem_t* leaving_inprogress_sem;

void* oxygen(void*a)
{
    printf("Oxygen Produced\n");
    fflush(stdout);
    
    sem_post(oxygen_sem);
    
    sem_wait(oxygen_leave_sem);
    
    printf("Oxygen leaving\n");
    
    sem_post(oxygen_left_sem);
    
    return (void *)0;
}
void* hydrogen(void*a)
{
    printf("Hydrogen Produced\n");
    fflush(stdout);
    
    sem_post(hydro_sem);
    
    sem_wait(hydro_leave_sem);
    printf("Hydrogen leaving\n");
    
    sem_post(hydro_left_sem);
    return (void *)0;
}
void* sulfur(void*a)
{
    printf("Sulfer Produced\n");
    fflush(stdout);
    
    int err = 0;
    for (int i=0; i<2; i++)
    {
        err =sem_wait(hydro_sem);
        if (err==-1){printf("Error waiting for hydrogen");}
    }
    
    for (int i=0; i<4; i++)
    {
        err =sem_wait(oxygen_sem);
        if (err==-1){printf("Error waiting for oxygen");}
    }
    sem_wait(leaving_inprogress_sem);
    
    printf("Molecule Produced\n");
    fflush(stdout);
    //signal  threads to depart
    
    
    for (int i=0; i<2; i++)
    {
        sem_post(hydro_leave_sem);
    }
    for (int i=0; i<2; i++)
    {
        sem_wait(hydro_left_sem);
    }
    printf("Sulpher leaving\n");
    fflush(stdout);
    
    for (int i=0; i<4; i++)
    {
        sem_post(oxygen_leave_sem);
    }
    
    for (int i=0; i<4; i++)
    {
        sem_wait(oxygen_left_sem);
    }
    sem_post(leaving_inprogress_sem);
    return (void *)0;
}
void openSems()
{
    hydro_sem = sem_open("hydrosmphr", O_CREAT|O_EXCL, 0466, 0);
    while (hydro_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore hydrosmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("hydrosmphr");
            hydro_sem = sem_open("hydrosmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    oxygen_sem = sem_open("oxygsmphr", O_CREAT|O_EXCL, 0466, 0);
    while (oxygen_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore oxygsmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("oxygsmphr");
            oxygen_sem = sem_open("oxygsmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    oxygen_leave_sem = sem_open("oxygleavesmphr", O_CREAT|O_EXCL, 0466, 0);
    while (oxygen_leave_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore oxygleavesmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("oxygleavesmphr");
            oxygen_leave_sem = sem_open("oxygleavesmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    hydro_leave_sem = sem_open("hydroleavesmphr", O_CREAT|O_EXCL, 0466, 0);
    while (hydro_leave_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore hydroleavesmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("hydroleavesmphr");
            hydro_leave_sem = sem_open("hydroleavesmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    hydro_left_sem = sem_open("hydroleftsmphr", O_CREAT|O_EXCL, 0466, 0);
    while (hydro_left_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore hydroleftsmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("hydroleftsmphr");
            hydro_left_sem = sem_open("hydroleavesmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    oxygen_left_sem = sem_open("oxygleftsmphr", O_CREAT|O_EXCL, 0466, 0);
    while (oxygen_left_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore oxygleftsmphr already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("oxygleftsmphr");
            oxygen_left_sem = sem_open("oxygleftsmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
    
    leaving_inprogress_sem = sem_open("leaving_inprogress_sem", O_CREAT|O_EXCL, 0466, 1);
    while (leaving_inprogress_sem==SEM_FAILED) {
        if (errno == EEXIST) {
            printf("semaphore leaving_inprogress_sem already exists, unlinking and reopening\n");
            fflush(stdout);
            sem_unlink("leaving_inprogress_sem");
            oxygen_left_sem = sem_open("oxygleftsmphr", O_CREAT|O_EXCL, 0466, 0);
        }
        else {
            printf("semaphore could not be opened, error # %d\n", errno);
            fflush(stdout);
            exit(1);
        }
    }
}
void closeSems()
{
    sem_close(hydro_sem);
    sem_unlink("hydrosmphr");
    sem_close(oxygen_sem);
    sem_unlink("oxygsmphr");
    sem_close(hydro_leave_sem);
    sem_unlink("hydroleavesmphr");
    sem_close(oxygen_leave_sem);
    sem_unlink("oxygleavesmphr");
    sem_close(hydro_left_sem);
    sem_unlink("hydroleftsmphr");
    sem_close(oxygen_left_sem);
    sem_unlink("oxygleftsmphr");
    sem_close(leaving_inprogress_sem);
    sem_unlink("leaving_inprogress_sem");
}