#pragma once
#include "Common.h"
#include <algorithm>


class CentralCache
{
public:
    static CentralCache *GetInstance()
    {
        static CentralCache inst; // scott meyers' singleton
        return &inst;
    }

    size_t FetchRangeObj(void *&start, void *&end, size_t n, size_t alignSize);

    Span *GetOneSpan(SpanList &list, size_t size);

private:

};