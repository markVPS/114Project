#include "../include/Simulator.h"
#include <chrono>
#include <iostream>
#include <sstream>

Simulator::Simulator(const std::vector<PCB>& jobs_, Policy policy, int quantum, int totalMemory)
    : jobs(jobs_), scheduler(policy, quantum), memoryManager(totalMemory) {}

void Simulator::log(const std::string& message) {
    std::cout << "[t=" << currentTime << "] " << message << std::endl;
}

void Simulator::admitNewJobs() {
    for (auto& job : jobs) {
        if (job.arrivalTime == currentTime && job.state == ProcessState::NEW) {
            int startAddr;
            if (memoryManager.allocate(job.memoryNeeded, startAddr)) {
                job.memoryStart = startAddr;
                job.state = ProcessState::READY;
                readyQueue.push_back(&job);
                log("PID " + std::to_string(job.pid) +
                    " admitted, memory allocated at " + std::to_string(startAddr) +
                    ", state=READY");
            } else {
                job.state = ProcessState::WAITING_MEMORY;
                waitingMemoryQueue.push_back(&job);
                log("PID " + std::to_string(job.pid) +
                    " blocked for memory, state=WAITING_MEMORY");
            }
        }
    }
}

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
            log("PID " + std::to_string(job->pid) +
                " memory allocated at " + std::to_string(startAddr) +
                ", state=READY");
        } else {
            waitingMemoryQueue.push_back(job);
        }
    }
}

void Simulator::dispatchIfNeeded() {
    if (runningProcess == nullptr) {
        runningProcess = scheduler.selectProcess(readyQueue);
        currentQuantumUsed = 0;

        if (runningProcess) {
            runningProcess->state = ProcessState::RUNNING;
            dispatchedThisTick = true;

            std::string message = "Scheduler selected PID " +
                                  std::to_string(runningProcess->pid) +
                                  " using policy " +
                                  policyToString(scheduler.getPolicy());

            if (scheduler.getPolicy() == Policy::RR) {
                message += " with quantum=" + std::to_string(scheduler.getQuantum());
            }

            message += ", state=RUNNING";
            log(message);
        }
    }
}

void Simulator::executeOneTick() {
    if (!runningProcess) return;
    if (dispatchedThisTick) return;

    runningProcess->remainingTime--;
    runningProcess->executedTicks++;
    currentQuantumUsed++;

    log("PID " + std::to_string(runningProcess->pid) +
        " executed one CPU tick, remaining=" +
        std::to_string(runningProcess->remainingTime));

    for (auto& req : runningProcess->requests) {
        if (!req.granted && runningProcess->executedTicks == req.requestTick) {
            if (!resourceManager.exists(req.name)) {
                log("PID " + std::to_string(runningProcess->pid) +
                    " requested unknown resource " + req.name +
                    "; request ignored");
                req.granted = true;
                break;
            }

            if (resourceManager.requestResource(req.name)) {
                req.granted = true;
                runningProcess->ownedResource = req.name;
                runningProcess->resourceRemainingHold = req.holdDuration;
                log("PID " + std::to_string(runningProcess->pid) +
                    " acquired " + req.name +
                    " for " + std::to_string(req.holdDuration) + " tick(s)");
            } else {
                runningProcess->state = ProcessState::WAITING_RESOURCE;
                waitingResourceQueue.push_back(runningProcess);
                log("PID " + std::to_string(runningProcess->pid) +
                    " blocked for " + req.name +
                    ", state=WAITING_RESOURCE");
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
        log("PID " + std::to_string(runningProcess->pid) +
            " quantum expired, returned to READY");
        runningProcess = nullptr;
    }
}

void Simulator::grantWaitingResource(const std::string& resourceName) {
    size_t count = waitingResourceQueue.size();
    bool grantedOne = false;

    for (size_t i = 0; i < count; ++i) {
        PCB* blocked = waitingResourceQueue.front();
        waitingResourceQueue.pop_front();

        if (!grantedOne) {
            for (auto& req : blocked->requests) {
                if (!req.granted &&
                    req.name == resourceName &&
                    blocked->executedTicks >= req.requestTick) {
                    if (resourceManager.requestResource(resourceName)) {
                        req.granted = true;
                        blocked->ownedResource = resourceName;
                        blocked->resourceRemainingHold = req.holdDuration;
                        blocked->state = ProcessState::READY;
                        readyQueue.push_back(blocked);
                        log("PID " + std::to_string(blocked->pid) +
                            " acquired " + resourceName +
                            " from waiting queue, state=READY");
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

void Simulator::releaseOwnedResource(PCB* job, const std::string& reason) {
    if (!job || job->ownedResource.empty()) return;

    std::string resourceName = job->ownedResource;
    job->ownedResource = "";
    job->resourceRemainingHold = 0;

    resourceManager.releaseResource(resourceName);

    if (reason.empty()) {
        log("PID " + std::to_string(job->pid) +
            " released " + resourceName);
    } else {
        log("PID " + std::to_string(job->pid) +
            " released " + resourceName + " " + reason);
    }

    grantWaitingResource(resourceName);
}

void Simulator::handleResourceReleases() {
    for (auto& job : jobs) {
        if (!job.ownedResource.empty()) {
            job.resourceRemainingHold--;

            if (job.resourceRemainingHold <= 0) {
                releaseOwnedResource(&job, "");
            }
        }
    }
}

void Simulator::checkTerminations() {
    if (runningProcess && runningProcess->remainingTime <= 0) {
        releaseOwnedResource(runningProcess, "on termination");

        memoryManager.freeBlock(runningProcess->memoryStart);
        runningProcess->state = ProcessState::TERMINATED;
        log("PID " + std::to_string(runningProcess->pid) +
            " terminated, memory freed, state=TERMINATED");
        runningProcess = nullptr;
    }
}

void Simulator::printStateSummary() const {
    std::ostringstream summary;
    summary << "STATE SUMMARY: ";

    for (const auto& job : jobs) {
        summary << "PID " << job.pid << "=" << stateToString(job.state);

        if (!job.ownedResource.empty()) {
            summary << "(" << job.ownedResource
                    << ":" << job.resourceRemainingHold << " ticks left)";
        }

        summary << " ";
    }

    std::cout << summary.str() << std::endl;
}

bool Simulator::allFinished() const {
    for (const auto& job : jobs) {
        if (job.state != ProcessState::TERMINATED) {
            return false;
        }
    }
    return true;
}

void Simulator::run() {
    auto startTime = std::chrono::high_resolution_clock::now();

    auto printTickHeader = [&]() {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - startTime).count();

        std::cout << "\n==============================" << std::endl;
        std::cout << "REAL TIME: " << elapsed << " seconds" << std::endl;
        std::cout << "SIMULATION TICK: " << currentTime << std::endl;
        std::cout << "==============================" << std::endl;
    };

    auto updateResourceTimers = [&]() {
        for (auto& job : jobs) {
            if (!job.ownedResource.empty() && job.resourceRemainingHold > 0) {
                job.resourceRemainingHold--;
            }
        }
    };

    auto terminateRunningIfDone = [&]() -> bool {
        if (runningProcess && runningProcess->remainingTime <= 0) {
            std::string message = "PID " + std::to_string(runningProcess->pid) +
                                  " terminated, memory freed";

            if (!runningProcess->ownedResource.empty()) {
                std::string resourceName = runningProcess->ownedResource;
                resourceManager.releaseResource(resourceName);
                runningProcess->ownedResource = "";
                runningProcess->resourceRemainingHold = 0;
                message += ", released " + resourceName;
            }

            memoryManager.freeBlock(runningProcess->memoryStart);
            runningProcess->state = ProcessState::TERMINATED;
            runningProcess = nullptr;

            log(message + ", state=TERMINATED");
            return true;
        }

        return false;
    };

    auto releaseOneExpiredResource = [&]() -> bool {
        for (auto& job : jobs) {
            if (!job.ownedResource.empty() && job.resourceRemainingHold <= 0) {
                std::string resourceName = job.ownedResource;
                job.ownedResource = "";
                job.resourceRemainingHold = 0;
                resourceManager.releaseResource(resourceName);

                log("PID " + std::to_string(job.pid) +
                    " released " + resourceName);
                return true;
            }
        }

        return false;
    };

    auto grantOneWaitingResource = [&]() -> bool {
        size_t count = waitingResourceQueue.size();

        for (size_t i = 0; i < count; ++i) {
            PCB* blocked = waitingResourceQueue.front();
            waitingResourceQueue.pop_front();

            bool granted = false;

            for (auto& req : blocked->requests) {
                if (!req.granted && blocked->executedTicks >= req.requestTick) {
                    if (resourceManager.requestResource(req.name)) {
                        req.granted = true;
                        blocked->ownedResource = req.name;
                        blocked->resourceRemainingHold = req.holdDuration;
                        blocked->state = ProcessState::READY;
                        readyQueue.push_back(blocked);

                        log("PID " + std::to_string(blocked->pid) +
                            " acquired " + req.name +
                            " from waiting queue, state=READY");
                        granted = true;
                    }
                    break;
                }
            }

            if (!granted && blocked->state == ProcessState::WAITING_RESOURCE) {
                waitingResourceQueue.push_back(blocked);
            }

            if (granted) return true;
        }

        return false;
    };

    auto expireQuantumIfNeeded = [&]() -> bool {
        if (runningProcess &&
            scheduler.getPolicy() == Policy::RR &&
            currentQuantumUsed >= scheduler.getQuantum() &&
            runningProcess->remainingTime > 0) {
            runningProcess->state = ProcessState::READY;
            readyQueue.push_back(runningProcess);

            log("PID " + std::to_string(runningProcess->pid) +
                " quantum expired, returned to READY");

            runningProcess = nullptr;
            return true;
        }

        return false;
    };

    auto admitOneNewJob = [&]() -> bool {
        for (auto& job : jobs) {
            if (job.arrivalTime <= currentTime && job.state == ProcessState::NEW) {
                int startAddr;
                if (memoryManager.allocate(job.memoryNeeded, startAddr)) {
                    job.memoryStart = startAddr;
                    job.state = ProcessState::READY;
                    readyQueue.push_back(&job);

                    log("PID " + std::to_string(job.pid) +
                        " admitted, memory allocated at " +
                        std::to_string(startAddr) +
                        ", state=READY");
                } else {
                    job.state = ProcessState::WAITING_MEMORY;
                    waitingMemoryQueue.push_back(&job);

                    log("PID " + std::to_string(job.pid) +
                        " blocked for memory, state=WAITING_MEMORY");
                }

                return true;
            }
        }

        return false;
    };

    auto allocateOneWaitingMemoryJob = [&]() -> bool {
        size_t count = waitingMemoryQueue.size();

        for (size_t i = 0; i < count; ++i) {
            PCB* job = waitingMemoryQueue.front();
            waitingMemoryQueue.pop_front();

            int startAddr;
            if (memoryManager.allocate(job->memoryNeeded, startAddr)) {
                job->memoryStart = startAddr;
                job->state = ProcessState::READY;
                readyQueue.push_back(job);

                log("PID " + std::to_string(job->pid) +
                    " memory allocated at " + std::to_string(startAddr) +
                    ", state=READY");
                return true;
            }

            waitingMemoryQueue.push_back(job);
        }

        return false;
    };

    auto dispatchOneProcess = [&]() -> bool {
        if (runningProcess == nullptr) {
            runningProcess = scheduler.selectProcess(readyQueue);
            currentQuantumUsed = 0;

            if (runningProcess) {
                runningProcess->state = ProcessState::RUNNING;

                std::string message = "Scheduler selected PID " +
                                      std::to_string(runningProcess->pid) +
                                      " using policy " +
                                      policyToString(scheduler.getPolicy());

                if (scheduler.getPolicy() == Policy::RR) {
                    message += " with quantum=" + std::to_string(scheduler.getQuantum());
                }

                message += ", state=RUNNING";
                log(message);
                return true;
            }
        }

        return false;
    };

    auto handleOneResourceRequest = [&]() -> bool {
        if (!runningProcess) return false;

        for (auto& req : runningProcess->requests) {
            if (!req.granted && runningProcess->executedTicks >= req.requestTick) {
                if (!resourceManager.exists(req.name)) {
                    req.granted = true;
                    log("PID " + std::to_string(runningProcess->pid) +
                        " requested unknown resource " + req.name +
                        "; request ignored");
                    return true;
                }

                if (resourceManager.requestResource(req.name)) {
                    req.granted = true;
                    runningProcess->ownedResource = req.name;
                    runningProcess->resourceRemainingHold = req.holdDuration;

                    log("PID " + std::to_string(runningProcess->pid) +
                        " acquired " + req.name +
                        " for " + std::to_string(req.holdDuration) +
                        " tick(s)");
                } else {
                    runningProcess->state = ProcessState::WAITING_RESOURCE;
                    waitingResourceQueue.push_back(runningProcess);

                    log("PID " + std::to_string(runningProcess->pid) +
                        " blocked for " + req.name +
                        ", state=WAITING_RESOURCE");

                    runningProcess = nullptr;
                }

                return true;
            }
        }

        return false;
    };

    auto executeOneCpuTick = [&]() -> bool {
        if (!runningProcess) return false;

        runningProcess->remainingTime--;
        runningProcess->executedTicks++;
        currentQuantumUsed++;

        log("PID " + std::to_string(runningProcess->pid) +
            " executed one CPU tick, remaining=" +
            std::to_string(runningProcess->remainingTime));

        return true;
    };

    while (!allFinished()) {
        printTickHeader();
        updateResourceTimers();

        bool didOneAction =
            terminateRunningIfDone() ||
            releaseOneExpiredResource() ||
            grantOneWaitingResource() ||
            handleOneResourceRequest() ||
            expireQuantumIfNeeded() ||
            admitOneNewJob() ||
            allocateOneWaitingMemoryJob() ||
            dispatchOneProcess() ||
            executeOneCpuTick();

        if (!didOneAction) {
            log("No runnable event this tick, system idle");
        }

        printStateSummary();
        currentTime++;
    }

    log("Simulation complete");
}
