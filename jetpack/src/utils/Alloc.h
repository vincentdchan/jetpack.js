#pragma once

#include <cinttypes>

/***************************************************************************
NoReleaseAllocator - allocator that never releases until it is destroyed
***************************************************************************/
class NoReleaseAllocator
{
public:
    NoReleaseAllocator(int32_t cbFirst = 256, int32_t cbMax = 0x4000 /*16K*/);
    ~NoReleaseAllocator(void) { FreeAll(); }

    void *Alloc(int32_t cb);
    void FreeAll();
    void Clear() { FreeAll(); }

private:
    struct NraBlock
    {
        NraBlock * pblkNext;
        // ... DATA ...
    };
    NraBlock * m_pblkList;
    int32_t m_ibCur;
    int32_t m_ibMax;
    int32_t m_cbMinBlock;
    int32_t m_cbMaxBlock;

#if DEBUG
    int32 m_cbTotRequested;    // total bytes requested
    int32 m_cbTotAlloced;    // total bytes allocated including headers
    int32 m_cblk;            // number of blocks including big blocks
    int32 m_cpvBig;            // each generates its own big block
    int32 m_cpvSmall;        // put in a common block
#endif //DEBUG
};
