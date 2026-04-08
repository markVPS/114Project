#ifndef PCB_H
#define PCB_H

#include <string>
#include <vector>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING_MEMORY,
    WAITING_RESOURCE,
    TERMINATED
};

struct ResourceRequest {
    std::string name;
    int requestTick;
    int holdDuration;
    bool granted = false;
};

struct PCB {
    int pid;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int memoryNeeded;
    int priority;

    ProcessState state = ProcessState::NEW;

    int executedTicks = 0;

    int memoryStart = -1;
    int memorySize = 0;

    bool ownsPrinter = false;
    int printerRemainingHold = 0;

    std::vector<ResourceRequest> requests;

    PCB(int pid_, int arrival_, int burst_, int memory_, int priority_);
};

std::string stateToString(ProcessState state);

#endif