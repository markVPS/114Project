#include "../include/ResourceManager.h"
//records total units available
//tracks how many units are allocated
ResourceManager::ResourceManager() {
    capacity["PRINTER"] = 1;
    inUse["PRINTER"] = 0;
}
// request a resource, if there is available capacity the inUser counter will be
//incremented
// returns true when resource is granted, false if the request must wait
bool ResourceManager::requestResource(const std::string& name) {
    if (inUse[name] < capacity[name]) {
        inUse[name]++;
        return true;
    }
    return false;
}
//release the prevoisly granted resource and decrement inUse coutner
//checks that inUse coutner is not negative before decrementing
void ResourceManager::releaseResource(const std::string& name) {
    if (inUse[name] > 0) {
        inUse[name]--;
    }
}
//return the number of availbale untis for the resouce
// computes capacity - inUse
int ResourceManager::available(const std::string& name) const {
    auto cap = capacity.find(name);
    auto used = inUse.find(name);

    if (cap == capacity.end() || used == inUse.end()) return 0;

    return cap->second - used->second;
}
