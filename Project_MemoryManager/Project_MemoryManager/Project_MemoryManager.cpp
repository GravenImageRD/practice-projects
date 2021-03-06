// Project_MemoryManager.cpp : Defines the entry point for the console application.
//
#include "MemoryManager.h"
#include <iostream>

int main()
{
	{
		std::cout << "Test 1 -- Allocate Too Much" << std::endl;
		MemoryManager* memMan = new MemoryManager(256);
		memMan->ReportBlocks();

		// Allocate too much.
		std::cout << "Allocating 512 bytes.." << std::endl;
		if (memMan->Allocate(512) != NULL)
		{
			std::cout << "Allocating more than pool size didn't return null?!" << std::endl;
		}
		else
		{
			std::cout << "Failed to allocate, as expected!" << std::endl;
		}

		delete memMan;
	}

	{
		std::cout << "Test 2 -- Allocate the Whole Thing" << std::endl;
		MemoryManager* memMan = new MemoryManager(256);

		std::cout << "Allocating 256 bytes.." << std::endl;
		void* memory = memMan->Allocate(256);
		if (!memory)
		{
			std::cout << "Couldn't allocate max pool size." << std::endl;
		}
		else if (memMan->Allocate(1))
		{
			std::cout << "Somehow allocated a byte even with the pool empty?!" << std::endl;
		}
		else
		{
			std::cout << "Allocated!" << std::endl;
		}

		memMan->ReportBlocks();

		// Deallocate.
		std::cout << "Deallocating...";
		memMan->Deallocate(memory);
		memMan->ReportBlocks();

		delete memMan;
	}

	{
		std::cout << "Test 3 -- Multiple Allocations/Deallocations with Memory Consolidation" << std::endl;
		MemoryManager* memMan = new MemoryManager(256);

		std::cout << "Allocating 32 bytes.." << std::endl;
		void* memory1 = memMan->Allocate(32);

		std::cout << "Allocating 128 bytes.." << std::endl;
		void* memory2 = memMan->Allocate(128);
		memMan->ReportBlocks();

		// Deallocate to create fragmentation.
		std::cout << "Deallocating 32 bytes at " << (int)memory1 << ".." << std::endl;
		memMan->Deallocate(memory1);
		memMan->ReportBlocks();

		// Allocate too much for the first block.
		std::cout << "Allocating 48 bytes.." << std::endl;
		memory1 = memMan->Allocate(48);
		memMan->ReportBlocks();

		std::cout << "Deallocating 128 bytes at " << (int)memory2 << ".." << std::endl;
		memMan->Deallocate(memory2);
		memMan->ReportBlocks();

		delete memMan;
	}

    return 0;
}

