#include "../include/ResourceManager.h"
ResourceManager::ResourceManager() { //records total units available and tracks how many units are allocated
    capacity["PRINTER"] = 1;
    inUse["PRINTER"] = 0;
}

bool ResourceManager::requestResource(const std::string& name) { // request a resource, if there is available capacity the inUser counter will be incremented
    if (inUse[name] < capacity[name]) {
        inUse[name]++;
        return true; // returns true when resource is granted, false if the request must wait
    }
    return false;
}

void ResourceManager::releaseResource(const std::string& name) {
    if (inUse[name] > 0) { //checks that inUse coutner is not negative before decrementing
        inUse[name]--; //release the prevoisly granted resource and decrement inUse coutner
    }
}
int ResourceManager::available(const std::string& name) const { //return the number of availbale untis for the resouce
    auto cap = capacity.find(name);
    auto used = inUse.find(name);

    if (cap == capacity.end() || used == inUse.end()) return 0;

    return cap->second - used->second; //computes capacity - inUse
}
