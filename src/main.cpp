#include "../include/Simulator.h"
#include "../include/PCB.h"
#include "../include/Scheduler.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

std::vector<PCB> loadJobs(const std::string& filename) {
    std::vector<PCB> jobs;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open jobs file: " + filename);
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);

        int pid, arrival, burst, memory, priority;
        if (!(ss >> pid >> arrival >> burst >> memory >> priority)) {
            throw std::runtime_error("Invalid job line: " + line);
        }

        PCB pcb(pid, arrival, burst, memory, priority);

        std::string token;
        while (ss >> token) {
            size_t atPos = token.find('@');
            size_t colonPos = token.find(':');

            if (atPos != std::string::npos && colonPos != std::string::npos) {
                ResourceRequest req;
                req.name = token.substr(0, atPos);
                req.requestTick = std::stoi(token.substr(atPos + 1, colonPos - atPos - 1));
                req.holdDuration = std::stoi(token.substr(colonPos + 1));
                pcb.requests.push_back(req);
            }
        }

        jobs.push_back(pcb);
    }

    return jobs;
}

int main(int argc, char* argv[]) {
    std::string policyStr = "RR";
    int quantum = 2;
    int memory = 1024;
    std::string jobsFile = "jobs.txt";

    try {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "--policy" && i + 1 < argc) {
                policyStr = argv[++i];
            } else if (arg == "--quantum" && i + 1 < argc) {
                quantum = std::stoi(argv[++i]);
            } else if (arg == "--memory" && i + 1 < argc) {
                memory = std::stoi(argv[++i]);
            } else if (arg == "--jobs" && i + 1 < argc) {
                jobsFile = argv[++i];
            } else if (i == 1 && arg.find("--") != 0) {
                jobsFile = arg;
            } else if (i == 2 && arg.find("--") != 0) {
                policyStr = arg;
            } else if (i == 3 && arg.find("--") != 0) {
                quantum = std::stoi(arg);
            } else {
                throw std::runtime_error("Invalid command-line argument: " + arg);
            }
        }

        Policy policy = parsePolicy(policyStr);
        std::vector<PCB> jobs = loadJobs(jobsFile);

        Simulator sim(jobs, policy, quantum, memory);
        sim.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        std::cerr << "Usage examples:" << std::endl;
        std::cerr << "  ./os_sim --policy RR --quantum 2 --memory 1024 --jobs jobs.txt" << std::endl;
        std::cerr << "  ./os_sim jobs.txt PRIORITY" << std::endl;
        return 1;
    }

    return 0;
}
