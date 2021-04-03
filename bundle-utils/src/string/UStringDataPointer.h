//
// Created by Duzhong Chen on 2021/3/24.
//

#ifndef ROCKET_BUNDLE_USTRINGDATAPOINTER_H
#define ROCKET_BUNDLE_USTRINGDATAPOINTER_H

#include "UStringData.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#define QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(Class) \
    Class &operator=(Class &&other) noexcept { \
        Class moved(std::move(other)); \
        swap(moved); \
        return *this; \
    }

#ifdef Q_CC_MSVC
#  define Q_NEVER_INLINE __declspec(noinline)
#  define Q_ALWAYS_INLINE __forceinline
#elif defined(Q_CC_GNU)
#  define Q_NEVER_INLINE __attribute__((noinline))
#  define Q_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#  define Q_NEVER_INLINE
#  define Q_ALWAYS_INLINE inline
#endif

struct UStringArrayOps;

template<class DataOps>
struct QArrayDataPointer
{
private:
    typedef UStringData Data;

public:
    typedef char16_t parameter_type;

    constexpr QArrayDataPointer() noexcept
            : d(nullptr), ptr(nullptr), size(0)
    {
    }

    QArrayDataPointer(const QArrayDataPointer &other) noexcept
            : d(other.d), ptr(other.ptr), size(other.size)
    {
        ref();
    }

    constexpr QArrayDataPointer(Data *header, char16_t *adata, uint32_t n = 0) noexcept
            : d(header), ptr(adata), size(n)
    {
    }

    explicit QArrayDataPointer(std::pair<UStringData *, char16_t*> adata, uint32_t n = 0) noexcept
            : d(adata.first), ptr(adata.second), size(n)
    {
    }

    static QArrayDataPointer fromRawData(const char16_t *rawData, uint32_t length) noexcept
    {
        assert(rawData || !length);
        return { nullptr, const_cast<char16_t*>(rawData), length };
    }

    QArrayDataPointer &operator=(const QArrayDataPointer &other) noexcept
    {
        QArrayDataPointer tmp(other);
        this->swap(tmp);
        return *this;
    }

    QArrayDataPointer(QArrayDataPointer &&other) noexcept
            : d(other.d), ptr(other.ptr), size(other.size)
    {
        other.d = nullptr;
        other.ptr = nullptr;
        other.size = 0;
    }

    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QArrayDataPointer)

    DataOps &operator*() noexcept
    {
        return *static_cast<DataOps *>(this);
    }

    DataOps *operator->() noexcept
    {
        return static_cast<DataOps *>(this);
    }

    const DataOps &operator*() const noexcept
    {
        return *static_cast<const DataOps *>(this);
    }

    const DataOps *operator->() const noexcept
    {
        return static_cast<const DataOps *>(this);
    }

    ~QArrayDataPointer()
    {
        if (!deref()) {
            (*this)->destroyAll();
            Data::deallocate(d);
        }
    }

    bool isNull() const noexcept
    {
        return !ptr;
    }

    char16_t *data() noexcept { return ptr; }
    const char16_t *data() const noexcept { return ptr; }

    char16_t *begin() noexcept { return data(); }
    char16_t *end() noexcept { return data() + size; }
    const char16_t *begin() const noexcept { return data(); }
    const char16_t *end() const noexcept { return data() + size; }
    const char16_t *constBegin() const noexcept { return data(); }
    const char16_t *constEnd() const noexcept { return data() + size; }

    void swap(QArrayDataPointer &other) noexcept
    {
        std::swap(d, other.d);
        std::swap(ptr, other.ptr);
        std::swap(size, other.size);
    }

    void clear() noexcept(std::is_nothrow_destructible<char16_t>::value)
    {
        QArrayDataPointer tmp;
        swap(tmp);
    }

    void detach(QArrayDataPointer *old = nullptr)
    {
        if (needsDetach())
            reallocateAndGrow(UStringData::GrowsAtEnd, 0, old);
    }

    // pass in a pointer to a default constructed QADP, to keep it alive beyond the detach() call
    void detachAndGrow(UStringData::GrowthPosition where, uint32_t n, QArrayDataPointer *old = nullptr)
    {
        if (!needsDetach()) {
            if (!n ||
                (where == UStringData::GrowsAtBeginning && freeSpaceAtBegin() >= n) ||
                (where == UStringData::GrowsAtEnd && freeSpaceAtEnd() >= n))
                return;
        }
        reallocateAndGrow(where, n, old);
    }

    Q_NEVER_INLINE void reallocateAndGrow(UStringData::GrowthPosition where, uint32_t n, QArrayDataPointer *old = nullptr)
    {
        if (where == UStringData::GrowsAtEnd && !old && !needsDetach() && n > 0) {
            (*this)->reallocate(constAllocatedCapacity() - freeSpaceAtEnd() + n, UStringData::Grow); // fast path
            return;
        }

        QArrayDataPointer dp(allocateGrow(*this, n, where));
        if (where == UStringData::GrowsAtBeginning) {
            dp.ptr += n;
            assert(dp.freeSpaceAtBegin() >= n);
        } else {
            assert(dp.freeSpaceAtEnd() >= n);
        }
        if (size) {
            uint32_t toCopy = size;
            if (n < 0)
                toCopy += n;
            if (needsDetach() || old)
                dp->copyAppend(begin(), begin() + toCopy);
            else
                dp->moveAppend(begin(), begin() + toCopy);
            dp.d->flags = flags();
            assert(dp.size == toCopy);
        }

        swap(dp);
        if (old)
            old->swap(dp);
    }

    // forwards from QArrayData
    uint32_t allocatedCapacity() noexcept { return d ? d->allocatedCapacity() : 0; }
    uint32_t constAllocatedCapacity() const noexcept { return d ? d->constAllocatedCapacity() : 0; }
    void ref() noexcept { if (d) d->ref(); }
    bool deref() noexcept { return !d || d->deref(); }
    bool isMutable() const noexcept { return d; }
    bool isShared() const noexcept { return !d || d->isShared(); }
    bool isSharedWith(const QArrayDataPointer &other) const noexcept { return d && d == other.d; }
    bool needsDetach() const noexcept { return !d || d->needsDetach(); }
    uint32_t detachCapacity(uint32_t newSize) const noexcept { return d ? d->detachCapacity(newSize) : newSize; }
    const typename Data::ArrayOptions flags() const noexcept { return d ? typename Data::ArrayOption(d->flags) : Data::ArrayOptionDefault; }
    void setFlag(UStringData::ArrayOptions f) noexcept { assert(d); d->flags |= f; }
    void clearFlag(typename Data::ArrayOptions f) noexcept { if (d) d->flags &= ~f; }

    Data *d_ptr() noexcept { return d; }
    void setBegin(char16_t *begin) noexcept { ptr = begin; }

    uint32_t freeSpaceAtBegin() const noexcept
    {
        if (d == nullptr)
            return 0;
        return this->ptr - Data::dataStart(d);
    }

    uint32_t freeSpaceAtEnd() const noexcept
    {
        if (d == nullptr)
            return 0;
        return d->constAllocatedCapacity() - freeSpaceAtBegin() - this->size;
    }

    // allocate and grow. Ensure that at the minimum requiredSpace is available at the requested end
    static QArrayDataPointer allocateGrow(const QArrayDataPointer &from, uint32_t n, UStringData::GrowthPosition position)
    {
        // calculate new capacity. We keep the free capacity at the side that does not have to grow
        // to avoid quadratic behavior with mixed append/prepend cases

        // use qMax below, because constAllocatedCapacity() can be 0 when using fromRawData()
        uint32_t minimalCapacity = std::max(from.size, from.constAllocatedCapacity()) + n;
        // subtract the free space at the side we want to allocate. This ensures that the total size requested is
        // the existing allocation at the other side + size + n.
        minimalCapacity -= (position == UStringData::GrowsAtEnd) ? from.freeSpaceAtEnd() : from.freeSpaceAtBegin();
        uint32_t capacity = from.detachCapacity(minimalCapacity);
        const bool grows = capacity > from.constAllocatedCapacity();
        auto [header, dataPtr] = Data::allocate(capacity, grows ? UStringData::Grow : UStringData::KeepSize);
        const bool valid = header != nullptr && dataPtr != nullptr;
        if (!valid)
            return QArrayDataPointer(header, dataPtr);

        // Idea: * when growing backwards, adjust pointer to prepare free space at the beginning
        //       * when growing forward, adjust by the previous data pointer offset

        // TODO: what's with CapacityReserved?
        dataPtr += (position == UStringData::GrowsAtBeginning) ? std::max<intptr_t>(0, (header->alloc - from.size - n) / 2)
                                                               : from.freeSpaceAtBegin();
        header->flags = from.flags();
        return QArrayDataPointer(header, dataPtr);
    }

    friend bool operator==(const QArrayDataPointer &lhs, const QArrayDataPointer &rhs) noexcept
    {
        return lhs.data() == rhs.data() && lhs.size == rhs.size;
    }

    friend bool operator!=(const QArrayDataPointer &lhs, const QArrayDataPointer &rhs) noexcept
    {
        return lhs.data() != rhs.data() || lhs.size != rhs.size;
    }

    Data *d;
    char16_t* ptr;
    uint32_t size;
};

template <typename Iterator>
using IfIsForwardIterator = typename std::enable_if<
        std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>::value,
        bool>::type;

struct UStringArrayOps : public QArrayDataPointer<UStringArrayOps>
{
protected:
    typedef UStringData Data;
    using DataPointer = QArrayDataPointer<UStringArrayOps>;

public:
    typedef typename QArrayDataPointer::parameter_type parameter_type;

    void appendInitialize(uint32_t newSize) noexcept
    {
        assert(this->isMutable());
        assert(!this->isShared());
        assert(newSize > this->size);
        assert(newSize - this->size <= this->freeSpaceAtEnd());

        char16_t *where = this->end();
        this->size = uint32_t(newSize);
        const char16_t *e = this->end();
        while (where != e)
            *where++ = char16_t();
    }

    void copyAppend(const char16_t *b, const char16_t *e) noexcept
    {
        assert(this->isMutable() || b == e);
        assert(!this->isShared() || b == e);
        assert(b <= e);
        assert((e - b) <= this->freeSpaceAtEnd());

        if (b == e)
            return;

        ::memcpy(static_cast<void *>(this->end()), static_cast<const void *>(b), (e - b) * sizeof(char16_t));
        this->size += (e - b);
    }

    void copyAppend(uint32_t n, parameter_type t) noexcept
    {
        assert(!this->isShared() || n == 0);
        assert(this->freeSpaceAtEnd() >= n);
        if (!n)
            return;

        char16_t *where = this->end();
        this->size += uint32_t(n);
        while (n--)
            *where++ = t;
    }

    void moveAppend(char16_t *b, char16_t *e) noexcept
    {
        copyAppend(b, e);
    }

    void truncate(size_t newSize) noexcept
    {
        assert(this->isMutable());
        assert(!this->isShared());
        assert(newSize < size_t(this->size));

        this->size = uint32_t(newSize);
    }

    void destroyAll() noexcept // Call from destructors, ONLY!
    {
        assert(this->d);
        assert(this->d->ref_.load(std::memory_order_relaxed) == 0);

        // As this is to be called only from destructor, it doesn't need to be
        // exception safe; size not updated.
    }

    char16_t *createHole(UStringData::GrowthPosition pos, uint32_t where, uint32_t n)
    {
        assert((pos == UStringData::GrowsAtBeginning && n <= this->freeSpaceAtBegin()) ||
               (pos == UStringData::GrowsAtEnd && n <= this->freeSpaceAtEnd()));

        char16_t *insertionPoint = this->ptr + where;
        if (pos == UStringData::GrowsAtEnd) {
            if (where < this->size)
                ::memmove(static_cast<void *>(insertionPoint + n), static_cast<void *>(insertionPoint), (this->size - where) * sizeof(char16_t));
        } else {
            if (where > 0)
                ::memmove(static_cast<void *>(this->ptr - n), static_cast<const void *>(this->ptr), where * sizeof(char16_t));
            this->ptr -= n;
            insertionPoint -= n;
        }
        this->size += n;
        return insertionPoint;
    }

    void insert(uint32_t i, const char16_t *data, uint32_t n)
    {
        typename Data::GrowthPosition pos = Data::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = Data::GrowsAtBeginning;
        DataPointer oldData;
        this->detachAndGrow(pos, n, &oldData);
        assert((pos == Data::GrowsAtBeginning && this->freeSpaceAtBegin() >= n) ||
               (pos == Data::GrowsAtEnd && this->freeSpaceAtEnd() >= n));

        char16_t *where = createHole(pos, i, n);
        ::memcpy(static_cast<void *>(where), static_cast<const void *>(data), n * sizeof(char16_t));
    }

    void insert(uint32_t i, uint32_t n, parameter_type t)
    {
        char16_t copy(t);

        typename Data::GrowthPosition pos = Data::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = Data::GrowsAtBeginning;
        this->detachAndGrow(pos, n);
        assert((pos == Data::GrowsAtBeginning && this->freeSpaceAtBegin() >= n) ||
               (pos == Data::GrowsAtEnd && this->freeSpaceAtEnd() >= n));

        char16_t *where = createHole(pos, i, n);
        while (n--)
            *where++ = copy;
    }

    template<typename... Args>
    void emplace(uint32_t i, Args &&... args)
    {
        bool detach = this->needsDetach();
        if (!detach) {
            if (i == this->size && this->freeSpaceAtEnd()) {
                new (this->end()) char16_t (std::forward<Args>(args)...);
                ++this->size;
                return;
            }
            if (i == 0 && this->freeSpaceAtBegin()) {
                new (this->begin() - 1) char16_t (std::forward<Args>(args)...);
                --this->ptr;
                ++this->size;
                return;
            }
        }
        char16_t tmp(std::forward<Args>(args)...);
        typename UStringData::GrowthPosition pos = UStringData::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = UStringData::GrowsAtBeginning;
        if (detach ||
            (pos == UStringData::GrowsAtBeginning && !this->freeSpaceAtBegin()) ||
            (pos == UStringData::GrowsAtEnd && !this->freeSpaceAtEnd()))
            this->reallocateAndGrow(pos, 1);

        char16_t *where = createHole(pos, i, 1);
        new (where) char16_t(std::move(tmp));
    }

    void erase(char16_t *b, uint32_t n)
    {
        char16_t *e = b + n;
        assert(this->isMutable());
        assert(b < e);
        assert(b >= this->begin() && b < this->end());
        assert(e > this->begin() && e <= this->end());

        // Comply with std::vector::erase(): erased elements and all after them
        // are invalidated. However, erasing from the beginning effectively
        // means that all iterators are invalidated. We can use this freedom to
        // erase by moving towards the end.
        if (b == this->begin())
            this->ptr = e;
        else if (e != this->end())
            ::memmove(static_cast<void *>(b), static_cast<void *>(e), (static_cast<char16_t *>(this->end()) - e) * sizeof(char16_t));
        this->size -= n;
    }

    void eraseFirst() noexcept
    {
        assert(this->isMutable());
        assert(this->size);
        ++this->ptr;
        --this->size;
    }

    void eraseLast() noexcept
    {
        assert(this->isMutable());
        assert(this->size);
        --this->size;
    }

    void assign(char16_t *b, char16_t *e, parameter_type t) noexcept
    {
        assert(b <= e);
        assert(b >= this->begin() && e <= this->end());

        while (b != e)
            ::memcpy(static_cast<void *>(b++), static_cast<const void *>(&t), sizeof(char16_t));
    }

    bool compare(const char16_t *begin1, const char16_t *begin2, size_t n) const
    {
        // only use memcmp for fundamental types or pointers.
        // Other types could have padding in the data structure or custom comparison
        // operators that would break the comparison using memcmp
        return ::memcmp(begin1, begin2, n * sizeof(char16_t )) == 0;
    }

    void reallocate(uint32_t alloc, UStringData::AllocationOption option)
    {
        auto pair = Data::reallocateUnaligned(this->d, this->ptr, alloc, option);
        assert(pair.first != nullptr);
        this->d = pair.first;
        this->ptr = pair.second;
    }

    template<typename It>
    void appendIteratorRange(It b, It e, IfIsForwardIterator<It> = true)
    {
        assert(this->isMutable() || b == e);
        assert(!this->isShared() || b == e);
        const uint32_t distance = std::distance(b, e);
        assert(distance >= 0 && distance <= this->allocatedCapacity() - this->size);

        char16_t *iter = this->end();
        for (; b != e; ++iter, ++b) {
            new (iter) char16_t(*b);
            ++this->size;
        }
    }
};

#endif //ROCKET_BUNDLE_USTRINGDATAPOINTER_H
