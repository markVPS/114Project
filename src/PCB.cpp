#include "../include/PCB.h"

PCB::PCB(int pid_, int arrival_, int burst_, int memory_, int priority_)
    : pid(pid_),
      arrivalTime(arrival_),
      burstTime(burst_),
      remainingTime(burst_),
      memoryNeeded(memory_),
      priority(priority_),
      memorySize(memory_) {}

std::string stateToString(ProcessState state) {
    switch (state) {
        case ProcessState::NEW: return "NEW";
        case ProcessState::READY: return "READY";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::WAITING_MEMORY: return "WAITING_MEMORY";
        case ProcessState::WAITING_RESOURCE: return "WAITING_RESOURCE";
        case ProcessState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}