// 每个线程都有一个threadcache，结构是一个哈希桶数组，每个桶中对应不同大小的freelist
#include "Common.h"
#include <cstdio>


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
};

// 对齐规则具体看博客
static const size_t FREE_LIST_NUM = 208;     // 哈希表中自由链表个数
static const size_t MAX_BYTES = 256 * 1024;  // ThreadCache单次申请的最大字节数

class ThreadCache
{
public:
    void *Allocate(size_t size);

    void Deallocate(void *obj, size_t size);

private:
    FreeList freelists_[FREE_LIST_NUM];
};








