class MemoryBlock
{
public:
	MemoryBlock(void* blockAddress, unsigned int blockSize);

	MemoryBlock*	prevBlock;
	MemoryBlock*	nextBlock;
	char*			blockAddress;
	unsigned int	blockSize;
	bool			inUse;
};

class MemoryManager
{
public:
	MemoryManager();
	MemoryManager(unsigned int poolSize);
	~MemoryManager();

	void* Allocate(unsigned int size);
	void Deallocate(void* memory);
	 
	unsigned int	poolSize;
	char*			memPool;
	MemoryBlock*	blockList;

	void Initialize(unsigned int poolSize);
	void ReportBlocks();
};
