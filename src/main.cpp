#include "../include/Simulator.h"
#include "../include/PCB.h"
#include "../include/Scheduler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


//includes process parameters and resource requests
std::vector<PCB> loadJobs(const std::string& filename) { //loads job definitions from a file
    std::vector<PCB> jobs;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);

        int pid, arrival, burst, memory, priority;
        ss >> pid >> arrival >> burst >> memory >> priority;

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

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--policy") policyStr = argv[++i];
        else if (arg == "--quantum") quantum = std::stoi(argv[++i]);
        else if (arg == "--memory") memory = std::stoi(argv[++i]);
        else if (arg == "--jobs") jobsFile = argv[++i];
    }
//loads jobs form the file and starts the simulator
    Policy policy = parsePolicy(policyStr);

    std::vector<PCB> jobs = loadJobs(jobsFile);

    Simulator sim(jobs, policy, quantum, memory);
    sim.run();

    return 0;
}
