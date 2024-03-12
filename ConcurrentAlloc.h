#pragma once

#include "ThreadCache.h"
#include <thread>

void *ConcurrentAlloc(size_t size)
{
    // 打印thread_local变量，看每个线程的是否不同
    cout << std::this_thread::get_id() << " " << pTLSThreadCache << endl;

    if(pTLSThreadCache == nullptr)
    {
        pTLSThreadCache = new ThreadCache;
    }

    return pTLSThreadCache->Allocate(size);
}

void ConcurrentFree(void *obj, size_t size)
{   
    assert(obj);

    pTLSThreadCache->Deallocate(obj, size);

}