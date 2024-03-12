#include "ThreadCache.h"

void *ThreadCache::Allocate(size_t size)
{
    assert(size <= MAX_BYTES);

    size_t alignSize = SizeClass::RoundUp(size);
    size_t index = SizeClass::Index(size);

    if(!freelists_[index].Empty())
    {
        return freelists_[index].Pop();
    }
    else
    {
        return FetchFromCentralCache(index, alignSize);
    }

}

void ThreadCache::Deallocate(void *obj, size_t size)
{
    assert(obj);
    assert(size <= MAX_BYTES);

    size_t index = SizeClass::Index(size);
    freelists_[index].Push(obj);
}

void *ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
    // TODO
    return nullptr;
}