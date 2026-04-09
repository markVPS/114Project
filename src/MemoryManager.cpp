#include "../include/MemoryManager.h"
//total memory is stored 
MemoryManager::MemoryManager(int totalMemory) : totalMemory(totalMemory) {
    blocks.push_back({0, totalMemory, true});
}
//scnas "blocks" for the first free block large enough

bool MemoryManager::allocate(int size, int& startAddress) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= size) {
            startAddress = blocks[i].start;
//if it is an exact fit, the block is alloocated and is no longer free
            if (blocks[i].size == size) {
                blocks[i].free = false;
//if the free block is larger, split the block into an allocated and 
                //remaining free block for the leftover space
            } else {
                MemoryBlock allocated = {blocks[i].start, size, false};
                MemoryBlock remaining = {blocks[i].start + size, blocks[i].size - size, true};

                blocks[i] = allocated;
                blocks.insert(blocks.begin() + i + 1, remaining);
            }
            return true;
        }
    }
// there is no suitable free block found 
    return false;
}
//after marking the block that begins at startAddress as free, call mergerFreeBlocks()
//  to coalesce the adjacent free blocks 
//reduces fragmentation
void MemoryManager::freeBlock(int startAddress) {
    for (auto& block : blocks) {
        if (block.start == startAddress) {
            block.free = true;
            mergeFreeBlocks();
            return;
        }
    }
}
//merge adjacent free blocks into a single larger free block
//modifies the vector 
void MemoryManager::mergeFreeBlocks() {
    for (size_t i = 0; i + 1 < blocks.size();) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;
            blocks.erase(blocks.begin() + i + 1);
        } else {
            ++i;
        }
    }
}
//returns a const reference to the list of memory blocks
const std::vector<MemoryBlock>& MemoryManager::getBlocks() const {
    return blocks;
}
