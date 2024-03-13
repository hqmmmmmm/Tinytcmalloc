// 每个线程都有一个threadcache，结构是一个哈希桶数组，每个桶中对应不同大小的freelist
#pragma once
#include <cstdio>
#include "CentralCache.h"






class ThreadCache
{
public:
    void *Allocate(size_t size);

    void Deallocate(void *obj, size_t size);

    void *FetchFromCentralCache(size_t index, size_t alignSize);

private:
    FreeList freelists_[FREE_LIST_NUM];
};


static thread_local ThreadCache *pTLSThreadCache = nullptr;







