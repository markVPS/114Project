#include "../include/ResourceManager.h"

ResourceManager::ResourceManager() {
    capacity["PRINTER"] = 1;
    inUse["PRINTER"] = 0;
}

bool ResourceManager::requestResource(const std::string& name) {
    if (inUse[name] < capacity[name]) {
        inUse[name]++;
        return true;
    }
    return false;
}

void ResourceManager::releaseResource(const std::string& name) {
    if (inUse[name] > 0) {
        inUse[name]--;
    }
}

int ResourceManager::available(const std::string& name) const {
    auto cap = capacity.find(name);
    auto used = inUse.find(name);

    if (cap == capacity.end() || used == inUse.end()) return 0;

    return cap->second - used->second;
}