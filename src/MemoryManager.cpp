#include "../include/MemoryManager.h"
MemoryManager::MemoryManager(int totalMemory) : totalMemory(totalMemory) { //total memory is stored 
    blocks.push_back({0, totalMemory, true});
}
//scnas "blocks" for the first free block large enough

bool MemoryManager::allocate(int size, int& startAddress) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= size) {
            startAddress = blocks[i].start;
            if (blocks[i].size == size) { //if it is an exact fit, the block is alloocated and is no longer free
                blocks[i].free = false;
            } else { //if the free block is larger, split the block into an allocated and remaining free block for the leftover space
                MemoryBlock allocated = {blocks[i].start, size, false};
                MemoryBlock remaining = {blocks[i].start + size, blocks[i].size - size, true};

                blocks[i] = allocated;
                blocks.insert(blocks.begin() + i + 1, remaining);
            }
            return true;
        }
    }
    return false; // there is no suitable free block found 
}

void MemoryManager::freeBlock(int startAddress) { //after marking the block that begins at startAddress as free, call mergerFreeBlocks() to coalesce the adjacent free blocks reduces fragmentation
    for (auto& block : blocks) {
        if (block.start == startAddress) {
            block.free = true;
            mergeFreeBlocks();
            return;
        }
    }
}

void MemoryManager::mergeFreeBlocks() { //merge adjacent free blocks into a single larger free block and modifies the vector 
    for (size_t i = 0; i + 1 < blocks.size();) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;
            blocks.erase(blocks.begin() + i + 1);
        } else {
            ++i;
        }
    }
}

const std::vector<MemoryBlock>& MemoryManager::getBlocks() const { //returns a const reference to the list of memory blocks
    return blocks;
}
