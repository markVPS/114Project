#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <string>
#include "PCB.h"

enum class Policy {
    FCFS,
    RR,
    PRIORITY
};

class Scheduler {
private:
    Policy policy;
    int quantum;

public:
    Scheduler(Policy policy, int quantum = 2);

    PCB* selectProcess(std::deque<PCB*>& readyQueue);
    Policy getPolicy() const;
    int getQuantum() const;
};

Policy parsePolicy(const std::string& policyStr);

#endif