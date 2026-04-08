#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

enum JobState { NEW, WAITING, READY, RUNNING, DONE };

                                    // JOBS
                                    //represents a job in the system, includes all info about the job 
struct jobs{
        int PID;
        int arrivalTime;
        int burstTimeNeeded; 
        int remainingTime;
        int printerRequestedTime
        int printerTimeDuration;
        int memoryNeeded;
// using bool to represnet whether a job has allocted memoory ot not, and whether it is requesting the printer now or not
    bool allocatedMemory = true;
    bool needsPrinterNow = false;
// job state is first new because it was just created
    JobState state = NEW;
 
bool NeedsPrinter(){
    return needsPrinterNow && (burstTimeNeeded - remainingTime == printerRequestedTime);
}
//using NeedsPrinter to return true is the job needs the printer and has reached the time it should request the printer


void runOneBurst(){
    remainingTime--;
}
bool donewithPrinter(){
    return remainingTime <= 0;
}
void setWaiting(){
    state = WAITING;
}

}



}





// MAIN

// main idea (pseudo-structure):
// 1) create jobs
// 2) create memory + resources
// 3) scheduler loop:
//    - admit jobs
//    - allocate memory (or block)
//    - pick next job
//    - attempt resource acquisition
//    - update states
//    - log events

while (time < MAX_TIME) {

    
    // memory gating: jobs cannot enter READY unless memory is allocated
    tryAllocateMemoryForWaitingJobs(time);
    
    Job* j = scheduler.pickNext();
    logger.logSelect(time, j);
    
    if (j == NULL) { time++; continue; }
    
    if (j->needsResource()) {
        bool ok = resourceManager.acquire(j->neededResource(), j);
        logger.logAcquire(time, j, ok);
        if (!ok) { j->setWaiting(); scheduler.requeue(j); time++; continue; }
    }
    
    j->runOneTick();
    logger.logState(time, j);
    
    if (j->done()) {
        resourceManager.releaseAll(j);
        memoryManager.free(j);
        logger.logDone(time, j);
    } else {
        scheduler.requeue(j);
    }
    
    time++;
}

ad™mitNewJobs(time);