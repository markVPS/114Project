#include "../include/Simulator.h"
#include <iostream>

//initializes the simulators w/a list of jobs, policy, quantum, and total memory
// simulator manages scheduling, allocation/dealloaction, and resource management
Simulator::Simulator(const std::vector<PCB>& jobs_, Policy policy, int quantum, int totalMemory)
    : jobs(jobs_), scheduler(policy, quantum), memoryManager(totalMemory) {}

//prints a timestamp
void Simulator::log(const std::string& message) {
    std::cout << "[t=" << currentTime << "] " << message << std::endl;
}
//check for new jobs that arrive at the current time
//attempt to allocate memory, if succesful job state is changed to READY
// otherwise job state is changed to WAITING  unitl memory is available
void Simulator::admitNewJobs() {
    for (auto& job : jobs) {
        if (job.arrivalTime == currentTime && job.state == ProcessState::NEW) {
            int startAddr;
            if (memoryManager.allocate(job.memoryNeeded, startAddr)) {
                job.memoryStart = startAddr;
                job.state = ProcessState::READY;
                readyQueue.push_back(&job);
                log("PID " + std::to_string(job.pid) + " admitted, memory allocated at " + std::to_string(startAddr) + ", state=READY");
            } else {
                job.state = ProcessState::WAITING_MEMORY;
                waitingMemoryQueue.push_back(&job);
                log("PID " + std::to_string(job.pid) + " blocked for memory, state=WAITING_MEMORY");
            }
        }
    }
}
//allocate memory for the jobs in the WAITING state
//re-checks if memory is available
//if so, job state is READY, if not job is still WAITING
void Simulator::tryAllocateWaitingMemory() {
    size_t count = waitingMemoryQueue.size();

    for (size_t i = 0; i < count; ++i) {
        PCB* job = waitingMemoryQueue.front();
        waitingMemoryQueue.pop_front();

        int startAddr;
        if (memoryManager.allocate(job->memoryNeeded, startAddr)) {
            job->memoryStart = startAddr;
            job->state = ProcessState::READY;
            readyQueue.push_back(job);
            log("PID " + std::to_string(job->pid) + " memory allocated at " + std::to_string(startAddr) + ", state=READY");
        } else {
            waitingMemoryQueue.push_back(job);
        }
    }
}
//next process will run if no other process is currently running
//process is chosen from the ready queue
//resets the quantm counter and changes process state to RUNNING
void Simulator::dispatchIfNeeded() {
    if (runningProcess == nullptr) {
        runningProcess = scheduler.selectProcess(readyQueue);
        currentQuantumUsed = 0;

        if (runningProcess) {
            runningProcess->state = ProcessState::RUNNING;
            log("PID " + std::to_string(runningProcess->pid) + " dispatched, state=RUNNING");
        }
    }
}
//execute only one UPU tick for running process
// decrement process's remaining time and increment its exected ticks
//checks if any other resource requests are due at this specific tick

void Simulator::executeOneTick() {
    if (!runningProcess) return;

    runningProcess->remainingTime--;
    runningProcess->executedTicks++;
    currentQuantumUsed++;

    log("PID " + std::to_string(runningProcess->pid) + " executed one tick, remaining=" + std::to_string(runningProcess->remainingTime));

    for (auto& req : runningProcess->requests) {
        if (!req.granted && runningProcess->executedTicks == req.requestTick) {
            if (resourceManager.requestResource(req.name)) {
                req.granted = true;
                if (req.name == "PRINTER") {
                    runningProcess->ownsPrinter = true;
                    runningProcess->printerRemainingHold = req.holdDuration;
                }
                log("PID " + std::to_string(runningProcess->pid) + " acquired " + req.name);
            } else {
                runningProcess->state = ProcessState::WAITING_RESOURCE;
                waitingResourceQueue.push_back(runningProcess);
                log("PID " + std::to_string(runningProcess->pid) + " blocked for " + req.name + ", state=WAITING_RESOURCE");
                runningProcess = nullptr;
                return;
            }
            break;
        }
    }

    if (runningProcess &&
        scheduler.getPolicy() == Policy::RR &&
        currentQuantumUsed >= scheduler.getQuantum() &&
        runningProcess->remainingTime > 0) {
        runningProcess->state = ProcessState::READY;
        readyQueue.push_back(runningProcess);
        log("PID " + std::to_string(runningProcess->pid) + " quantum expired, returned to READY");
        runningProcess = nullptr;
    }
}
//checks if processes are holding resouces and decrements their hold timers
// release the resouce when a resouce hold expries, then tries to grant the resouces
// to a process in the waiting queue

void Simulator::handleResourceReleases() {
    for (auto& job : jobs) {
        if (job.ownsPrinter) {
            job.printerRemainingHold--;

            if (job.printerRemainingHold <= 0) {
                job.ownsPrinter = false;
                resourceManager.releaseResource("PRINTER");
                log("PID " + std::to_string(job.pid) + " released PRINTER");

                size_t count = waitingResourceQueue.size();
                bool grantedOne = false;

                for (size_t i = 0; i < count; ++i) {
                    PCB* blocked = waitingResourceQueue.front();
                    waitingResourceQueue.pop_front();

                    if (!grantedOne) {
                        for (auto& req : blocked->requests) {
                            if (!req.granted &&
                                req.name == "PRINTER" &&
                                blocked->executedTicks >= req.requestTick) {
                                if (resourceManager.requestResource("PRINTER")) {
                                    req.granted = true;
                                    blocked->ownsPrinter = true;
                                    blocked->printerRemainingHold = req.holdDuration;
                                    blocked->state = ProcessState::READY;
                                    readyQueue.push_back(blocked);
                                    log("PID " + std::to_string(blocked->pid) + " acquired PRINTER from waiting queue, state=READY");
                                    grantedOne = true;
                                }
                                break;
                            }
                        }
                    }

                    if (blocked->state == ProcessState::WAITING_RESOURCE) {
                        waitingResourceQueue.push_back(blocked);
                    }
                }
            }
        }
    }
}
//checks if the running process has finished
//if done, releases any resources it holds, frees irs memory, and changes state to TERMINATED

void Simulator::checkTerminations() {
    if (runningProcess && runningProcess->remainingTime <= 0) {
        if (runningProcess->ownsPrinter) {
            runningProcess->ownsPrinter = false;
            resourceManager.releaseResource("PRINTER");
            log("PID " + std::to_string(runningProcess->pid) + " released PRINTER on termination");
        }

        memoryManager.freeBlock(runningProcess->memoryStart);
        runningProcess->state = ProcessState::TERMINATED;
        log("PID " + std::to_string(runningProcess->pid) + " terminated, memory freed");
        runningProcess = nullptr;
    }
}
//will return true if all jobs are done
bool Simulator::allFinished() const {
    for (const auto& job : jobs) {
        if (job.state != ProcessState::TERMINATED) {
            return false;
        }
    }
    return true;
}
//the simultion loop basically

void Simulator::run() {
    while (!allFinished()) {
        admitNewJobs();
        tryAllocateWaitingMemory();
        dispatchIfNeeded();
        executeOneTick();
        handleResourceReleases();
        checkTerminations();
        currentTime++;
    }

    log("Simulation complete");
}
