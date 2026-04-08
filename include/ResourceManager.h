#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <unordered_map>

class ResourceManager {
private:
    std::unordered_map<std::string, int> capacity;
    std::unordered_map<std::string, int> inUse;

public:
    ResourceManager();

    bool requestResource(const std::string& name);
    void releaseResource(const std::string& name);
    int available(const std::string& name) const;
};

#endif