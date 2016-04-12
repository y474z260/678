/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"
float waiting, turnaround, respons;


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
scheduler_t* sched;
int jobdown;


int compare_FCFS()
{
    return 0;
}

int compare_SJF(const void* p1, const void* p2)
{
    int p1_runtime = ((job_t*)p1)->timeleft;
    int p2_runtime = ((job_t*)p2)->timeleft;
    
    return p1_runtime - p2_runtime;
}

int compare_PSJF(const void* p1, const void* p2)
{
    int p1_remaintime = ((job_t*)p1)->timeleft;
    int p2_remaintime = ((job_t*)p2)->timeleft;
    
    return p1_remaintime - p2_remaintime;
}

int compare_PRI(const void* p1, const void* p2)
{
    int p1_pri = ((job_t*)p1)->priority;
    int p2_pri = ((job_t*)p2)->priority;
    
    return p1_pri - p2_pri; 
}

int compare_PPRI(const void* p1, const void* p2)
{
    int p1_pri = ((job_t*)p1)->priority;
    int p2_pri = ((job_t*)p2)->priority;
    
    return p1_pri - p2_pri; 
}


void coreUpdata(int t)
{
    for(int i = 0; i<sched->core_count; i++){
        if(sched->core_status[i] != NULL){
            sched->core_status[i]->timeleft -= (t - sched->core_status[i]->timerenew);
            sched->core_status[i]->timerenew = t;
        }
    }
}

int preempt(job_t* currentjob, job_t* newjob, scheme_t scheme)
{
    int diff = -1;
    if(scheme == PSJF){
        diff = currentjob->timeleft - newjob->timeleft;
        if(diff < 0){
            return 0;
        }
        if(diff > 0){
            return 1;
        }
        return currentjob->arriv_time > newjob->arriv_time;
    }
    if(scheme == PPRI){
        diff = currentjob->priority - newjob->priority;
        if(diff < 0){
            return 0;
        }
        if(diff > 0){
            return 1;
        }
        return currentjob->arriv_time > newjob->arriv_time;
    }
    return diff;
}

void record(job_t* job, int t)
{
    waiting = (waiting*(float)jobdown + (float)(job->wait))/((float)jobdown + 1.0f);
    turnaround = (turnaround*(float)jobdown + (float)t - (float)(job->arriv_time))/((float)jobdown + 1.0f);
    respons = (respons*(float)jobdown + (float)(job->responded))/((float)jobdown + 1.0f);
    jobdown++;
}
/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
    sched = malloc(sizeof(scheduler_t));
    sched->scheme = scheme;
    
        switch(scheme)
        {
            case FCFS:
                priqueue_init(&sched->queues, &compare_FCFS);
                break;
            
            case SJF:
                priqueue_init(&sched->queues, &compare_SJF);
                break;
                
            case PSJF:
                priqueue_init(&sched->queues, &compare_PSJF);
                break;
                
            case PRI:
                priqueue_init(&sched->queues, &compare_PRI);
                break;
                
            case PPRI:
                priqueue_init(&sched->queues, &compare_PPRI);
                break;
                
            case RR:
                priqueue_init(&sched->queues, &compare_FCFS);
                break;
                    
            default:
                break;
        }
    sched->core_count = cores;
    sched->core_status = malloc(sizeof(job_t*)*(sched->core_count));
    for(int i=0; i<cores; i++){
        sched->core_status[i] = NULL;
        jobdown = 0;
        waiting = 0;
        turnaround = 0; 
        respons = 0;
    }
    
}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
    job_t* job = malloc(sizeof(job_t));
    
    int index;
    
    job->jobnum = job_number;
    
    job->arriv_time = time;
    
    job->timeleft = running_time;
    
    job->priority = priority;
    
    job->responded = -1;
    
    job->wait = 0;
    
    job->timerenew = time;
    
    job->respondedto = -1;
    coreUpdata(time);
    for(int i=0; i<sched->core_count; i++){
        if(sched->core_status[i] == NULL){
            job->responded = 0;
            job->respondedto = time;
            sched->core_status[i] = job;
            return i;
        }
    }
    if(sched->scheme == PPRI || sched->scheme == PPRI){
        int preempted = -1;
        for(int i=0; i<sched->core_count; i++){
            if(preempt(sched->core_status[i], job, sched->scheme)){
                if(preempted == -1 || preempt(sched->core_status[i], sched->core_status[preempted], sched->scheme))
                    preempted = i;
            }
                
            }
        
        if(preempted != 1){
            if(sched->core_status[preempted]->respondedto == time){
                sched->core_status[preempted]->respondedto = -1;
                sched->core_status[preempted]->responded = -1;
            }
            priqueue_offer(&sched->queues, sched->core_status[preempted]);
            sched->core_status[preempted] = job;
            job->responded = 0;
            job->respondedto = time;
            return preempted;
        }
    }
    priqueue_offer(&sched->queues, job);
	return -1;
}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int time)
{
    coreUpdata(time);
    record(sched->core_status[core_id], time);
    free(sched->core_status[core_id]);
    if(priqueue_size(&sched->queues)!=0){
        sched->core_status[core_id] = priqueue_poll(&sched->queues);
        sched->core_status[core_id]->wait += (time-sched->core_status[core_id]->timerenew);
        if(sched->core_status[core_id]->responded == -1){
            sched->core_status[core_id]->responded = (time-sched->core_status[core_id]->arriv_time);
            sched->core_status[core_id]->respondedto = time;
        }
        sched->core_status[core_id]->timerenew = time;
        return sched->core_status[core_id]->jobnum;
    }
    sched->core_status[core_id] = NULL;
	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
    sched->core_status[core_id]->timeleft -= time-sched->core_status[core_id]->timerenew;
    sched->core_status[core_id]->timerenew = time;
    if(sched->core_status[core_id]->timeleft <= 0){
        record(sched->core_status[core_id], time);
        free(sched->core_status[core_id]);
        sched->core_status[core_id] = NULL;
    }else{
        priqueue_offer(&sched->queues, sched->core_status[core_id]);
        sched->core_status[core_id] = NULL;
    }
    if(priqueue_size(&sched->queues)!=0){
        job_t* job2 = priqueue_poll(&sched->queues);
        job2->wait += time-job2->timerenew;
        if(job2->responded == -1){
            job2->responded = time-job2->arriv_time;
            job2->respondedto = time;
        }
        job2->timerenew = time;
        sched->core_status[core_id] = job2;
        return job2->jobnum;
    }
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return waiting;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return turnaround;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return respons;
}


/**
  Free any memory associated with your scheduler.
 
  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
    priqueue_destroy(&sched->queues);
    free(sched->core_status);
    free(sched);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
    priqueue_node_t* node = sched->queues.root;
    for(int i=0; i<sched->queues.size; i++){
        if(sched->scheme == PRI || sched->scheme == PPRI){
            printf("%d(priority%d)", ((job_t*)(node->content))->jobnum, ((job_t*)(node->content))->priority);
            
        }else{
            printf("%d(timeleft%d)", ((job_t*)(node->content))->jobnum, ((job_t*)(node->content))->timeleft);
            node = node->next;
        }
    }
}
