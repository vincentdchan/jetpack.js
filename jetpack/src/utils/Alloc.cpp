#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include "Alloc.h"

#if DEBUG
#define DEBUG_TRASHMEM(pv, cb) memset(pv, 0xbc, cb)
#else
#define DEBUG_TRASHMEM(pv, cb)
#endif //DEBUG

#if TARGET_64
struct __ALIGN_FOO__ {
    int w1;
    double dbl;
};
#define ALIGN_FULL (offsetof(__ALIGN_FOO__, dbl))
#else
// Force check for 4 byte alignment to support Win98/ME
#define ALIGN_FULL 4
#endif  // TARGET_64

#define AlignFull(VALUE) (~(~((VALUE) + (ALIGN_FULL-1)) | (ALIGN_FULL-1)))
using byte = uint8_t;

NoReleaseAllocator::NoReleaseAllocator(int32_t cbFirst, int32_t cbMax)
        : m_pblkList(nullptr)
        , m_ibCur(0)
        , m_ibMax(0)
        , m_cbMinBlock(cbFirst)
        , m_cbMaxBlock(cbMax)
#if DEBUG
, m_cbTotRequested(0)
    , m_cbTotAlloced(0)
    , m_cblk(0)
    , m_cpvBig(0)
    , m_cpvSmall(0)
#endif
{
    // require reasonable ranges
    assert((0 < cbFirst) && (cbFirst < SHRT_MAX/2));
    assert((0 < cbMax  ) && (cbMax   < SHRT_MAX));
}

void * NoReleaseAllocator::Alloc(int32_t cb)
{
    assert(cb > 0);
    if (cb <= 0)
        return nullptr;

    const int32_t kcbHead = AlignFull(sizeof(NoReleaseAllocator::NraBlock));
    void * pv;

    if (cb > m_ibMax - m_ibCur)
    {
        int32_t cbBlock;
        int32_t cbAlloc;
        NraBlock * pblk;

        if (cb >= m_cbMaxBlock)
        {
            // check for integer overflow before allocating (See WindowsSE #88972)
            cbAlloc = cb + kcbHead;
            if (cbAlloc < cb)
            {
                assert(false); // too big!
                return nullptr;
            }

            // create a chunk just for this allocation
            pblk = (NraBlock *)malloc(cbAlloc);
            if (nullptr == pblk)
                return nullptr;
#if DEBUG
            m_cbTotAlloced   += cbAlloc;
            m_cbTotRequested += cb;
            m_cpvBig++;
            m_cblk++;
#endif //DEBUG
            if (m_ibCur < m_ibMax)
            {
                // There is still room in current block, so put the new block
                // after the current block.
                pblk->pblkNext = m_pblkList->pblkNext;
                m_pblkList->pblkNext = pblk;
            }
            else
            {
                // Link into front of the list.
                // Don't need to adjust m_ibCur and m_ibMax, because they
                // already have the correct relationship for this full block
                // (m_ibCur >= m_ibMax) and the actual values will not be
                // used.
                pblk->pblkNext = m_pblkList;
                m_pblkList = pblk;
            }
            DEBUG_TRASHMEM((byte *)pblk + kcbHead, cb);
            return (byte *)pblk + kcbHead;
        }

        cbBlock = cb;                 // requested size
        if (m_ibMax > cbBlock)        // at least current block size
            cbBlock = m_ibMax;
        cbBlock += cbBlock;           // *2 (can overflow, but checked below)
        if (m_cbMinBlock > cbBlock)   // at least minimum size
            cbBlock = m_cbMinBlock;
        if (cbBlock > m_cbMaxBlock)   // no larger than the max
            cbBlock = m_cbMaxBlock;
        if (cb > cbBlock)             // guarantee it's big enough
        {
            printf("Request too large\n");
            assert(false);
            return nullptr;
        }

        // check for integer overflow before allocating (See WindowsSE #88972)
        cbAlloc = cbBlock + kcbHead;
        if ((cbAlloc < cbBlock) || (cbAlloc < cb))
        {
            assert(false); // too big!
            return nullptr ;
        }

        // allocate a new block
        pblk = (NraBlock *)malloc(cbAlloc);
#ifdef MEM_TRACK
        RegisterAlloc((char*)pblk,cbAlloc);
#endif
        if (nullptr == pblk)
            return nullptr;
#if DEBUG
        m_cbTotAlloced += cbAlloc;
        m_cblk++;
#endif //DEBUG
        pblk->pblkNext = m_pblkList;
        m_pblkList = pblk;
        m_ibMax = cbBlock;
        m_ibCur = 0;
    }
    assert(m_ibCur + cb <= m_ibMax);

#if DEBUG
    m_cbTotRequested += cb;
    m_cpvSmall++;
#endif //DEBUG
    pv = (byte *)m_pblkList + kcbHead + m_ibCur;
    DEBUG_TRASHMEM(pv, cb);
    m_ibCur += (int32_t)AlignFull(cb);
    assert(m_ibCur >= 0);
    return pv;
}

void NoReleaseAllocator::FreeAll(void)
{
    // Free all of the allocated blocks
    while (nullptr != m_pblkList)
    {
        NraBlock * pblk = m_pblkList;
#pragma prefast(suppress:6001, "Not sure why it is complaining *m_plkList is uninitialized")
        m_pblkList = pblk->pblkNext;
        free(pblk);
    }

    // prepare for next round of allocations
    m_ibCur = m_ibMax = 0;
#if DEBUG
    m_cbTotRequested = 0;
    m_cbTotAlloced = 0;
    m_cblk = 0;
    m_cpvBig = 0;
    m_cpvSmall = 0;
#endif
}