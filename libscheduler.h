/** @file libscheduler.h
 */

#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_
#include "../libpriqueue/libpriqueue.h"
/**
  Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;
typedef struct job_t
{
    int jobnum;
    int arriv_time;;
    int priority;
    int timeleft;
    int timerenew;
    int wait;
    int responded;
    int respondedto;

} job_t;
typedef struct _scheduler_t{
    scheme_t scheme;
    int core_count;
    priqueue_t queues;
    job_t** core_status;
}scheduler_t;
int compare_FCFS();
int compare_SJF(const void* p1, const void* p2);
int compare_PSJF(const void* p1, const void* p2);
int compare_PRI(const void* p1, const void* p2);
int compare_PPRI(const void* p1, const void* p2);
void record(job_t* job, int t);
void coreUpdata(int t);
int preempt(job_t* currentjob, job_t* newjob, scheme_t scheme);
void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

#endif /* LIBSCHEDULER_H_ */
