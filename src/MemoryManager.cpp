#include "../include/MemoryManager.h"

MemoryManager::MemoryManager(int totalMemory) : totalMemory(totalMemory) {
    blocks.push_back({0, totalMemory, true});
}

bool MemoryManager::allocate(int size, int& startAddress) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= size) {
            startAddress = blocks[i].start;

            if (blocks[i].size == size) {
                blocks[i].free = false;
            } else {
                MemoryBlock allocated = {blocks[i].start, size, false};
                MemoryBlock remaining = {blocks[i].start + size, blocks[i].size - size, true};

                blocks[i] = allocated;
                blocks.insert(blocks.begin() + i + 1, remaining);
            }
            return true;
        }
    }
    return false;
}

void MemoryManager::freeBlock(int startAddress) {
    for (auto& block : blocks) {
        if (block.start == startAddress) {
            block.free = true;
            mergeFreeBlocks();
            return;
        }
    }
}

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

const std::vector<MemoryBlock>& MemoryManager::getBlocks() const {
    return blocks;
}