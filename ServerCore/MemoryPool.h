#pragma once

enum {
	SLIST_ALLIGNMENT = 16
};

/*------------------
	MemoryHeader
-------------------*/

DECLSPEC_ALIGN(SLIST_ALLIGNMENT)
struct MemoryHeader : public SLIST_ENTRY{
	// [MemoryHeader][Data]

	MemoryHeader(int32 size) : allocSize(size) {
	}

	static void* AttachHeader(MemoryHeader* header, int32 size) {
		new(header) MemoryHeader(size); //placement new
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr) {
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr)-1;
		return header;
	}

	int32 allocSize;
};

/*------------------
	MemoryPool
-------------------*/

DECLSPEC_ALIGN(SLIST_ALLIGNMENT)
class MemoryPool {

public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void			Push(MemoryHeader* ptr);
	MemoryHeader*	Pop();

private:
	SLIST_HEADER	_header;
	int32			_allocSize = 0;
	atomic<int32>	_useCount = 0;
	atomic<int32>	_reserveCount = 0;

};

