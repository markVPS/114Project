#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>

struct MemoryBlock {
    int start;
    int size;
    bool free;
};

class MemoryManager {
private:
    int totalMemory;
    std::vector<MemoryBlock> blocks;

public:
    MemoryManager(int totalMemory = 1024);

    bool allocate(int size, int& startAddress);
    void freeBlock(int startAddress);
    void mergeFreeBlocks();

    const std::vector<MemoryBlock>& getBlocks() const;
};

#endif