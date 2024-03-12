// 每个线程都有一个threadcache，结构是一个哈希桶数组，每个桶中对应不同大小的freelist
#include "Common.h"
#include <cstdio>


// 对齐规则具体看博客
static const size_t FREE_LIST_NUM = 208;     // 哈希表中自由链表个数
static const size_t MAX_BYTES = 256 * 1024;  // ThreadCache单次申请的最大字节数

static void *&ObjNext(void *obj)
{
    return *(void **)obj;
}

class FreeList
{
public:
    void Push(void *obj)  // 回收空间
    {
        assert(obj);

        ObjNext(obj) = freelist_;
        freelist_ = obj;
    }

    void *Pop()   // 请求空间
    {
        assert(freelist_);

        void *obj = freelist_;
        freelist_ = ObjNext(obj);
        return obj;
    }

    bool Empty()
    {
        return freelist_ == nullptr;
    }

private:
    void *freelist_ = nullptr;   // 空闲链表，每个桶里都有一个空闲链表
};

class SizeClass
{
public:
    static size_t RoundUp_(size_t size, size_t alignNum)  // 子函数，根据所需字节数和所在分区对齐数计算对齐后的字节数
    {
        size_t res = 0;
        if(size % alignNum != 0)
        {
            res = (size / alignNum + 1) * alignNum;
        }
        else 
        {
            res = size;
        }

        return res;
    }

    static size_t RoundUp(size_t size)  // 计算对齐后的字节数
    {
        if(size <= 128)
        {
            return RoundUp_(size, 8);
        }
        else if(size <= 1024)
        {
            return RoundUp_(size, 16);
        }
        else if(size <= 8 * 1024)
        {
            return RoundUp_(size, 128);
        }
        else if(size <= 64 * 1024)
        {
            return RoundUp_(size, 1024);
        }
        else if(size <= 256 * 1024)
        {
            return RoundUp_(size, 8 * 1024);
        }
        else 
        {
            assert(false);
            return -1;
        }
    }


    static inline size_t Index_(size_t size, size_t align_shift)
    {
        return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
    }

    static inline size_t Index(size_t size)
    {
        assert(size < MAX_BYTES);

        static int group_array[4] = {16, 56, 56, 56};
        if(size <= 128)
        {
            return Index_(size, 3);
        }
        else if(size <= 1024)
        {
            return Index_(size - 128, 4) + group_array[0];
        }
        else if(size <= 8 * 1024)
        {
            return Index_(size - 1024, 7) + group_array[0] + group_array[1];
        }
        else if(size <= 64 * 1024)
        {
            return Index_(size, 10) + group_array[0] + group_array[1]
                + group_array[2];
        }
        else if(size <= 256 * 1024)
        {
            return Index_(size, 16) + + group_array[0] + group_array[1]
                + group_array[2] + group_array[3];
        }
        else
        {
            assert(false);
            return -1;
        }
    }
};



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







