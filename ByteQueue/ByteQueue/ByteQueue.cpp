// ByteQueue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Comment for testing
//One more time with feelign.
#define AVAILABLE_MEMORY			2048
#define AVAILABLE_Q_COUNT			64
#define Q_MEMORY_FOOTPRINT			3
#define TOTAL_Q_MEMORY_FOOTPRINT	AVAILABLE_Q_COUNT * Q_MEMORY_FOOTPRINT

// Value for flagging a Q as available.
#define Q_UNASSIGNED			255

// QBlocks have one byte for tracking the next block assigned to a given Q and 7 bytes for the bytes enqueued in the Q.
// Each Q takes 3 bytes which means we reserve 192 bytes for the Q definitions. This means we have 1856 bytes remaining
// for QBlocks which works out to 232 QBlocks, but since we need to track the head and tail QBlock for the free list we lose one
// byte meaning we can only safely create 231 QBlocks leave 6 bytes unusable. Any value over the safe number can be used
// as a sentinel for the end of the free list, but to make sure we don't conflict with the Q_UNASSIGNED value, we'll use 254.
#define QBLOCK_AVAILABLE_BYTES	7
#define QBLOCK_MEMORY_FOOTPRINT	QBLOCK_AVAILABLE_BYTES + 1
#define QBLOCK_AVAILABLE_COUNT	(AVAILABLE_MEMORY - TOTAL_Q_MEMORY_FOOTPRINT - 1) / QBLOCK_MEMORY_FOOTPRINT
#define QBLOCK_SENTINEL_INDEX	254
#define QBLOCK_FREE_LIST_HEAD	TOTAL_Q_MEMORY_FOOTPRINT
#define QBLOCK_FREE_LIST_TAIL	QBLOCK_FREE_LIST_HEAD + 1
#define QBLOCK_MEMORY_START		QBLOCK_FREE_LIST_TAIL + 1

unsigned char data[2048] = { 0 };

#define GetQBlockFromIndex(index) (QBlock*)(data + QBLOCK_MEMORY_START) + index

void on_out_of_memory() { throw; }
void on_illegal_operation() { throw; }

struct Q
{
	// Block index 255 is reserved to represent an unassigned Q.
	unsigned char startBlockIndex;
	unsigned char endBlockIndex;
	unsigned char startBlockByteIndex : 4;
	unsigned char endBlockByteIndex : 4;
};

struct QBlock
{
	// Block index 255 is reserved to represent this is a tail block.
	unsigned char nextBlockIndex;
	unsigned char blockMemory[QBLOCK_AVAILABLE_BYTES];
};

void initialize_queue_memory()
{
	// If we fill the memory for Qs with 255 we mark them all as unassigned without needing to iterate over them.
	memset(data, 0xff, TOTAL_Q_MEMORY_FOOTPRINT);

	// Build the free QBlock chain. First byte after the Qs is used to track the index of the head of the free list.
	data[QBLOCK_FREE_LIST_HEAD] = (unsigned char)0;
	auto curBlock = (QBlock*)(data + QBLOCK_MEMORY_START);
	for (int i = 1; i < (QBLOCK_AVAILABLE_COUNT - 1); ++i)
	{
		curBlock->nextBlockIndex = i;
		curBlock++;
	}

	// On the last block, set the sentinel index.
	curBlock->nextBlockIndex = QBLOCK_SENTINEL_INDEX;
	data[QBLOCK_FREE_LIST_TAIL] = (unsigned char)(QBLOCK_AVAILABLE_COUNT - 1);
}

Q* create_queue()
{
	auto newQ = (Q*)data;
	for (int index = 0; index < AVAILABLE_Q_COUNT; ++index)
	{
		if (newQ->startBlockIndex == Q_UNASSIGNED)
		{
			break;
		}

		newQ++;
	}

	if (newQ == nullptr)
	{ 
		// Couldn't find an unassigned Q and creating more than 64 Qs is illegal.
		on_illegal_operation();
	}

	newQ->startBlockIndex = newQ->endBlockIndex = QBLOCK_SENTINEL_INDEX;
	newQ->startBlockByteIndex = 0;
	newQ->endBlockByteIndex = QBLOCK_AVAILABLE_BYTES;

	return newQ;
}

void destroy_queue(Q* q)
{
	// Return blocks to the end of the list.
	auto firstQBlock = (QBlock*)(data + QBLOCK_MEMORY_START);
	auto curQBlockIndex = q->startBlockIndex;
	while (curQBlockIndex != QBLOCK_SENTINEL_INDEX)
	{
		(firstQBlock + data[QBLOCK_FREE_LIST_TAIL])->nextBlockIndex = curQBlockIndex;
		data[QBLOCK_FREE_LIST_TAIL] = curQBlockIndex;
		(firstQBlock + curQBlockIndex)->nextBlockIndex = QBLOCK_SENTINEL_INDEX;
		curQBlockIndex = (firstQBlock + curQBlockIndex)->nextBlockIndex;
	}

	q->startBlockIndex = Q_UNASSIGNED;
}

void enqueue_byte(Q* q, unsigned char b)
{
	if (q->startBlockIndex == Q_UNASSIGNED)
	{ 
		on_illegal_operation(); 
	}

	// If the end block byte index is at the total available bytes, assign a new block.
	if (q->endBlockByteIndex == QBLOCK_AVAILABLE_BYTES)
	{
		if (data[QBLOCK_FREE_LIST_HEAD] == QBLOCK_SENTINEL_INDEX)
		{
			on_out_of_memory();
		}

		auto newBlockIndex = data[QBLOCK_FREE_LIST_HEAD];
		QBlock* newBlock = GetQBlockFromIndex(newBlockIndex);
		data[QBLOCK_FREE_LIST_HEAD] = newBlock->nextBlockIndex;
		newBlock->nextBlockIndex = QBLOCK_SENTINEL_INDEX;

		// If this isn't the first block we'll need to update the end.
		if (q->endBlockIndex != QBLOCK_SENTINEL_INDEX)
		{
			QBlock* oldEnd = GetQBlockFromIndex(q->endBlockIndex);
			oldEnd->nextBlockIndex = newBlockIndex;
		}

		q->endBlockIndex = newBlockIndex;
		q->endBlockByteIndex = 0;

		// If this is the first block assigned, set the start as well.
		if (q->startBlockIndex == QBLOCK_SENTINEL_INDEX)
		{
			q->startBlockIndex = q->endBlockIndex;
		}
	}

	QBlock* block = GetQBlockFromIndex(q->endBlockIndex);
	block->blockMemory[q->endBlockByteIndex++] = b;
}

unsigned char dequeue_byte(Q* q)
{
	if (q->startBlockIndex == Q_UNASSIGNED || q->startBlockIndex == QBLOCK_SENTINEL_INDEX || (q->startBlockIndex == q->endBlockIndex && q->startBlockByteIndex == q->endBlockByteIndex))
	{
		on_illegal_operation();
	}

	QBlock* block = GetQBlockFromIndex(q->startBlockIndex);
	unsigned char b = block->blockMemory[q->startBlockByteIndex++];

	// Return block and update Q if this block is empty.
	if (q->startBlockByteIndex == QBLOCK_AVAILABLE_BYTES)
	{
		// Update tail of free list if there is one.
		if (data[QBLOCK_FREE_LIST_TAIL] != QBLOCK_SENTINEL_INDEX)
		{
			QBlock* oldTail = GetQBlockFromIndex(data[QBLOCK_FREE_LIST_TAIL]);
			oldTail->nextBlockIndex = q->startBlockIndex;
		}

		data[QBLOCK_FREE_LIST_TAIL] = q->startBlockIndex;
		auto block = GetQBlockFromIndex(q->startBlockIndex);
		q->startBlockIndex = block->nextBlockIndex;
		block->nextBlockIndex = QBLOCK_SENTINEL_INDEX;
		q->startBlockByteIndex = 0;
	}

	return b;
}

void test_queues()
{
	Q * q0 = create_queue();
	enqueue_byte(q0, 0);
	enqueue_byte(q0, 1);
	Q * q1 = create_queue();
	enqueue_byte(q1, 3);
	enqueue_byte(q0, 2);
	enqueue_byte(q1, 4);
	printf(" %d", dequeue_byte(q0));
	printf(" %d\n", dequeue_byte(q0));
	enqueue_byte(q0, 5);
	enqueue_byte(q1, 6);
	printf(" %d", dequeue_byte(q0));
	printf(" %d\n", dequeue_byte(q0));
	destroy_queue(q0);
	printf(" %d", dequeue_byte(q1));
	printf(" %d", dequeue_byte(q1));
	printf(" %d\n", dequeue_byte(q1));
	destroy_queue(q1);
}

int main()
{
	initialize_queue_memory();
	test_queues();

    return 1;
}

