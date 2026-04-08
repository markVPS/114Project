#include "../include/Scheduler.h"
#include <stdexcept>

Scheduler::Scheduler(Policy policy, int quantum) : policy(policy), quantum(quantum) {}

PCB* Scheduler::selectProcess(std::deque<PCB*>& readyQueue) {
    if (readyQueue.empty()) return nullptr;

    if (policy == Policy::FCFS || policy == Policy::RR) {
        PCB* p = readyQueue.front();
        readyQueue.pop_front();
        return p;
    }

    if (policy == Policy::PRIORITY) {
        auto bestIt = readyQueue.begin();
        for (auto it = readyQueue.begin(); it != readyQueue.end(); ++it) {
            if ((*it)->priority < (*bestIt)->priority) {
                bestIt = it;
            }
        }
        PCB* p = *bestIt;
        readyQueue.erase(bestIt);
        return p;
    }

    return nullptr;
}

Policy Scheduler::getPolicy() const {
    return policy;
}

int Scheduler::getQuantum() const {
    return quantum;
}

Policy parsePolicy(const std::string& policyStr) {
    if (policyStr == "FCFS") return Policy::FCFS;
    if (policyStr == "RR") return Policy::RR;
    if (policyStr == "PRIORITY") return Policy::PRIORITY;
    throw std::runtime_error("Invalid policy");
}