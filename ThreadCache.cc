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

    // 这里其实可能还要考虑将一些块还给CentralCache
}

void *ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
    // batchNum: ThreadCache请求的内存块数
    size_t batchNum = std::min(freelists_[index].MaxSize(), SizeClass::NumMoveSize(alignSize));
    if(batchNum == freelists_[index].MaxSize())
    {
        freelists_[index].MaxSize()++;
    }

    void *start = nullptr;
    void *end = nullptr;

    // CentralCache实际返回的内存块数
    size_t actulNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, alignSize);

    assert(actulNum >= 1);
    if(actulNum == 1)  
    {
        assert(start == end);
        return start;
    }
    else 
    {
        // 如果CentralCache给ThreadCache的内存块数大于1，则将第一个分配给线程，其它的push到ThreadCache中的空闲链表中
        freelists_[index].PushRange(ObjNext(start), end);
        return start;
    }
}