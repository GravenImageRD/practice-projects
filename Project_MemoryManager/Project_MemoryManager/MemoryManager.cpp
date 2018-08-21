#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "MemoryManager.h"

#define DEFAULT_POOL_SIZE 1024 * 1024 * 50 // 50MB

MemoryManager::MemoryManager()
{
	this->Initialize(DEFAULT_POOL_SIZE);
}

MemoryManager::MemoryManager(unsigned int poolSize)
{
	this->Initialize(poolSize);
}

MemoryManager::~MemoryManager()
{
	MemoryBlock* currentBlock = this->blockList;
	while (currentBlock)
	{
		MemoryBlock* nextBlock = currentBlock->nextBlock;
		delete currentBlock;
		currentBlock = nextBlock;
	}

	free(this->memPool);
}

void MemoryManager::Initialize(unsigned int poolSize)
{
	this->poolSize = poolSize;
	this->memPool = (char*)malloc(poolSize);
	this->blockList = new MemoryBlock(this->memPool, poolSize);
}

void* MemoryManager::Allocate(unsigned int size)
{
	MemoryBlock* freeBlock = this->blockList;
	while (freeBlock && (freeBlock->inUse || freeBlock->blockSize < size))
	{
		freeBlock = freeBlock->nextBlock;
	}

	if (freeBlock && !freeBlock->inUse)
	{
		// If we aren't using this entire block, make a new block for the remaining space.
		if (freeBlock->blockSize > size)
		{
			MemoryBlock* newBlock = new MemoryBlock(freeBlock->blockAddress + size, freeBlock->blockSize - size);
			newBlock->nextBlock = freeBlock->nextBlock;
			newBlock->prevBlock = freeBlock;
			freeBlock->nextBlock = newBlock;
			freeBlock->blockSize = size;

			if (newBlock->nextBlock)
			{
				newBlock->nextBlock->prevBlock = newBlock;
			}
		}

		freeBlock->inUse = true;
		return freeBlock->blockAddress;
	}

	return NULL;
}

void MemoryManager::Deallocate(void* memory)
{
	MemoryBlock* currentBlock = this->blockList;
	while (currentBlock && currentBlock->blockAddress != memory)
	{
		currentBlock = currentBlock->nextBlock;
	}

	if (currentBlock)
	{
		currentBlock->inUse = false;

		// Check for consolidation.
		if (currentBlock->nextBlock && !currentBlock->nextBlock->inUse)
		{
			MemoryBlock* nextBlock = currentBlock->nextBlock;
			currentBlock->blockSize += nextBlock->blockSize;
			currentBlock->nextBlock = nextBlock->nextBlock;
			delete nextBlock;
		}

		if (currentBlock->prevBlock && !currentBlock->prevBlock->inUse)
		{
			currentBlock->prevBlock->blockSize += currentBlock->blockSize;
			currentBlock->prevBlock->nextBlock = currentBlock->nextBlock;
			delete currentBlock;
		}
	}
}

void MemoryManager::ReportBlocks()
{
	MemoryBlock* currentBlock = this->blockList;
	while (currentBlock)
	{
		std::cout << (unsigned int)currentBlock->blockAddress << " (InUse: " << currentBlock->inUse << ") Size: " << currentBlock->blockSize << std::endl;
		currentBlock = currentBlock->nextBlock;
	}
}

MemoryBlock::MemoryBlock(void* blockAddress, unsigned int blockSize)
{
	this->blockAddress = (char*)blockAddress;
	this->blockSize = blockSize;
	this->nextBlock = this->prevBlock = NULL;
	this->inUse = false;
}