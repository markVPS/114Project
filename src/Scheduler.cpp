#include "../include/Scheduler.h"
#include <stdexcept>

//initializes the scheduler with a policy and quantum
Scheduler::Scheduler(Policy policy, int quantum) : policy(policy), quantum(quantum) {}

//selects the next process that will run based on the scheduling process
//removes the process from the ready queue
// returns nullptr if the queue is empty
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
//return the scheduling policy
Policy Scheduler::getPolicy() const {
    return policy;
}
//return the time quantum 
int Scheduler::getQuantum() const {
    return quantum;
}

//parses a policy string into an enum
// runtime error if the string doesnt match a known policy
Policy parsePolicy(const std::string& policyStr) {
    if (policyStr == "FCFS") return Policy::FCFS;
    if (policyStr == "RR") return Policy::RR;
    if (policyStr == "PRIORITY") return Policy::PRIORITY;
    throw std::runtime_error("Invalid policy");
}
