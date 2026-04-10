#include "../include/Scheduler.h"
#include <stdexcept>

Scheduler::Scheduler(Policy policy, int quantum) : policy(policy), quantum(quantum) {} //initializes the scheduler with a policy and quantum

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

Policy Scheduler::getPolicy() const { //return the scheduling policy
    return policy;
}

int Scheduler::getQuantum() const { //return the time quantum 
    return quantum;
}


Policy parsePolicy(const std::string& policyStr) { //parses a policy string into an enum
    if (policyStr == "FCFS") return Policy::FCFS;
    if (policyStr == "RR") return Policy::RR;
    if (policyStr == "PRIORITY") return Policy::PRIORITY;
    throw std::runtime_error("Invalid policy"); // runtime error if the string doesnt match a known policy
}
