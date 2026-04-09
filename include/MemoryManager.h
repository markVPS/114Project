#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
//region of memory shared. 
//start = the starting address of this block in the simulated memory space
//size = length of block
//free = true if block is available for mem allocation, false if already allocated
struct MemoryBlock {
    int start;
    int size;
    bool free;
};

class MemoryManager {
// total size of the memory space
//vector is the list of memory blocks. allocated and free blocks
private:
    int totalMemory;
    std::vector<MemoryBlock> blocks;

public:
    MemoryManager(int totalMemory = 1024);
// try to allocate a block of 'size', returns true and sets startAddress to the address of allocated region
    bool allocate(int size, int& startAddress);
    void freeBlock(int startAddress);
//helps reduce fragementation
    void mergeFreeBlocks();

    const std::vector<MemoryBlock>& getBlocks() const;
};

#endif
