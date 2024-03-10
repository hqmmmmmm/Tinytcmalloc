#include <iostream>
#include <new>

/*
    1.申请空间：先判断空闲链表中是否存在内存块，如果有，直接用即可；
               否则再判断大块内存中剩余空间是否大于T类型的大小，如果大于，直接从大块内存申请一块内存；
               否则需要向操作系统申请一个大块内存。

               注意这里还要判断T类型的大小是否小于当前平台下指针的大小，如果小于则需要补齐。

    2.回收空间：直接将回收的内存块头插到freelist即可。
               内存块在freelist中时，因为不需要存数据，哪怕内存块大小刚好是一个指针的大小也是可以的，因为分配出去才需要存数据。
*/

template<typename T>
class ObjectPool
{
public:
    T *New();
    void Delete(T *obj);
private:
    char *memory_ = nullptr;   // 指向内存块的指针
    size_t leftBytes = 0;      // 大块内存剩余的字节数
    void *freelist_ = nullptr;  // 空闲链表，用来管理归还的内存
};


// 申请一个T类型大小的空间
template<typename T>
T *ObjectPool<T>::New()
{
    T *obj = nullptr;  // 最终返回的空间

    if(freelist_)  
    {
        void *next = *(void **)freelist_;  // 第一个内存块指向的下一个内存块
        obj = (T *)freelist_;
        freelist_ = next;
    }
    else 
    {
        if(leftBytes < sizeof(T))
        {
            leftBytes = 128 * 1024;  // 128K的空间

            memory_ = (char *)malloc(leftBytes);

            if(memory_ == nullptr)
            {
                throw std::bad_alloc();
            }
        }

        obj = (T *)memory_;
        size_t objSize = sizeof(T) < sizeof(void *) ? sizeof(void *) : sizeof(T);  // 必须要能放得下一个指针
        memory_ += objSize;
        leftBytes -= objSize;
    }

    new(obj) T;    // placement-new

    return obj;
}

// 回收一个T类型大小的空间
template<typename T>
void ObjectPool<T>::Delete(T *obj)
{
    obj->~T();   // 析构函数

    *(void **)obj = freelist_;     // *(void **)obj 这种做法是为了得到obj前n个字节的内存，n代表当前平台下指针大小。
    freelist_ = obj;
}