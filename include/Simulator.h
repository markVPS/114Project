#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <deque>
#include <string>
#include <vector>
#include "PCB.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include "ResourceManager.h"

class Simulator {
private:
    int currentTime = 0;
    std::vector<PCB> jobs;
    std::deque<PCB*> readyQueue;
    std::deque<PCB*> waitingMemoryQueue;
    std::deque<PCB*> waitingResourceQueue;

    Scheduler scheduler;
    MemoryManager memoryManager;
    ResourceManager resourceManager;

    PCB* runningProcess = nullptr;
    int currentQuantumUsed = 0;
    bool dispatchedThisTick = false;

public:
    Simulator(const std::vector<PCB>& jobs, Policy policy, int quantum, int totalMemory);
    void run();

private:
    void admitNewJobs();
    void tryAllocateWaitingMemory();
    void dispatchIfNeeded();
    void executeOneTick();
    void handleResourceReleases();
    void grantWaitingResource(const std::string& resourceName);
    void releaseOwnedResource(PCB* job, const std::string& reason);
    void checkTerminations();
    void printStateSummary() const;
    bool allFinished() const;
    void log(const std::string& message);
};

#endif
