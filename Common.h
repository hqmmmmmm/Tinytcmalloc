#pragma once
#include <iostream>
#include <vector>
#include <cassert>
using std::vector;
using std::cout;
using std::endl;

static void *&ObjNext(void *obj)
{
    return *(void **)obj;
}

class FreeList
{
public:
    void PushRange(void *start, void *end) // 插入多块空间
    {
        ObjNext(end) = freelist_;
        freelist_ = start;
    }

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

    size_t &MaxSize()
    {
        return maxSize;
    }

private:
    void *freelist_ = nullptr;   // 空闲链表，每个桶里都有一个空闲链表

    size_t maxSize = 1; // 当前自由链表申请未达上限时，能够申请的最大空间是多少。
};


// 对齐规则具体看博客
static const size_t FREE_LIST_NUM = 208;     // 哈希表中自由链表个数
static const size_t MAX_BYTES = 256 * 1024;  // ThreadCache单次申请的最大字节数

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

    static size_t NumMoveSize(size_t size)
    {
        assert(size > 0);

        int num = MAX_BYTES / size;
        if(num > 512)
        {
            num = 512;
        }
        if(num < 2)
        {
            num = 2;
        }
        return num;
    }
};

struct Span
{
    using PageID = size_t;
    PageID pageID_ = 0;   // 页号
    size_t n_ = 0;   // 当前span管理的页的数量

    void *freelist_ = nullptr;  // span下面挂的小块空间的头结点
    size_t use_count = 0;  // span分配出去了多少块的空间

    Span *prev = nullptr;  // 前一个span
    Span *next = nullptr;  // 后一个span
};

class SpanList
{

};