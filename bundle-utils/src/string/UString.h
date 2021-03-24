//
// Created by Duzhong Chen on 2021/3/23.
//

#ifndef ROCKET_BUNDLE_USTRING_H
#define ROCKET_BUNDLE_USTRING_H

#include <memory>
#include <cinttypes>
#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <locale>
#include <codecvt>

#include "CommonArrayOps.h"

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

template <typename T>
inline constexpr bool qIsRelocatable =  std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

template <class T>
struct QTypedArrayData
        : QArrayData
{
    struct AlignmentDummy { QArrayData header; T data; };

    [[nodiscard]] static std::pair<QTypedArrayData *, T *> allocate(uint32_t capacity, AllocationOption option = QArrayData::KeepSize)
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QArrayData *d;
        void *result = QArrayData::allocate(&d, sizeof(T), capacity, option);
        return std::make_pair(static_cast<QTypedArrayData *>(d), static_cast<T *>(result));
    }

    static std::pair<QTypedArrayData *, T *>
    reallocateUnaligned(QTypedArrayData *data, T *dataPointer, uint32_t capacity, AllocationOption option)
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        std::pair<QArrayData *, void *> pair =
                QArrayData::reallocateUnaligned(data, dataPointer, sizeof(T), capacity, option);
        return std::make_pair(static_cast<QTypedArrayData *>(pair.first), static_cast<T *>(pair.second));
    }

    static void deallocate(QArrayData *data) noexcept
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QArrayData::deallocate(data, sizeof(T), alignof(AlignmentDummy));
    }

    static T *dataStart(QArrayData *data, uint32_t alignment) noexcept
    {
        // Alignment is a power of two
        assert(alignment >= uint32_t(alignof(QArrayData)) && !(alignment & (alignment - 1)));
        void *start =  reinterpret_cast<void *>(
                (uintptr_t(data) + sizeof(QArrayData) + alignment - 1) & ~(alignment - 1));
        return static_cast<T *>(start);
    }
};

template <class T>
struct QPodArrayOps
        : public QArrayDataPointer<T>
{
    static_assert (std::is_nothrow_destructible_v<T>, "Types with throwing destructors are not supported in Qt containers.");

protected:
    typedef QTypedArrayData<T> Data;
    using DataPointer = QArrayDataPointer<T>;

public:
    typedef typename QArrayDataPointer<T>::parameter_type parameter_type;

    void appendInitialize(uint32_t newSize) noexcept
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(!this->isShared());
        Q_ASSERT(newSize > this->size);
        Q_ASSERT(newSize - this->size <= this->freeSpaceAtEnd());

        T *where = this->end();
        this->size = uint32_t(newSize);
        const T *e = this->end();
        while (where != e)
            *where++ = T();
    }

    void copyAppend(const T *b, const T *e) noexcept
    {
        assert(this->isMutable() || b == e);
        assert(!this->isShared() || b == e);
        assert(b <= e);
        assert((e - b) <= this->freeSpaceAtEnd());

        if (b == e)
            return;

        ::memcpy(static_cast<void *>(this->end()), static_cast<const void *>(b), (e - b) * sizeof(T));
        this->size += (e - b);
    }

    void copyAppend(uint32_t n, parameter_type t) noexcept
    {
        assert(!this->isShared() || n == 0);
        assert(this->freeSpaceAtEnd() >= n);
        if (!n)
            return;

        T *where = this->end();
        this->size += uint32_t(n);
        while (n--)
            *where++ = t;
    }

    void moveAppend(T *b, T *e) noexcept
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

    T *createHole(QArrayData::GrowthPosition pos, uint32_t where, uint32_t n)
    {
        Q_ASSERT((pos == QArrayData::GrowsAtBeginning && n <= this->freeSpaceAtBegin()) ||
                 (pos == QArrayData::GrowsAtEnd && n <= this->freeSpaceAtEnd()));

        T *insertionPoint = this->ptr + where;
        if (pos == QArrayData::GrowsAtEnd) {
            if (where < this->size)
                ::memmove(static_cast<void *>(insertionPoint + n), static_cast<void *>(insertionPoint), (this->size - where) * sizeof(T));
        } else {
            if (where > 0)
                ::memmove(static_cast<void *>(this->ptr - n), static_cast<const void *>(this->ptr), where * sizeof(T));
            this->ptr -= n;
            insertionPoint -= n;
        }
        this->size += n;
        return insertionPoint;
    }

    void insert(uint32_t i, const T *data, uint32_t n)
    {
        typename Data::GrowthPosition pos = Data::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = Data::GrowsAtBeginning;
        DataPointer oldData;
        this->detachAndGrow(pos, n, &oldData);
        Q_ASSERT((pos == Data::GrowsAtBeginning && this->freeSpaceAtBegin() >= n) ||
                 (pos == Data::GrowsAtEnd && this->freeSpaceAtEnd() >= n));

        T *where = createHole(pos, i, n);
        ::memcpy(static_cast<void *>(where), static_cast<const void *>(data), n * sizeof(T));
    }

    void insert(uint32_t i, uint32_t n, parameter_type t)
    {
        T copy(t);

        typename Data::GrowthPosition pos = Data::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = Data::GrowsAtBeginning;
        this->detachAndGrow(pos, n);
        Q_ASSERT((pos == Data::GrowsAtBeginning && this->freeSpaceAtBegin() >= n) ||
                 (pos == Data::GrowsAtEnd && this->freeSpaceAtEnd() >= n));

        T *where = createHole(pos, i, n);
        while (n--)
            *where++ = copy;
    }

    template<typename... Args>
    void emplace(uint32_t i, Args &&... args)
    {
        bool detach = this->needsDetach();
        if (!detach) {
            if (i == this->size && this->freeSpaceAtEnd()) {
                new (this->end()) T(std::forward<Args>(args)...);
                ++this->size;
                return;
            }
            if (i == 0 && this->freeSpaceAtBegin()) {
                new (this->begin() - 1) T(std::forward<Args>(args)...);
                --this->ptr;
                ++this->size;
                return;
            }
        }
        T tmp(std::forward<Args>(args)...);
        typename QArrayData::GrowthPosition pos = QArrayData::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = QArrayData::GrowsAtBeginning;
        if (detach ||
            (pos == QArrayData::GrowsAtBeginning && !this->freeSpaceAtBegin()) ||
            (pos == QArrayData::GrowsAtEnd && !this->freeSpaceAtEnd()))
            this->reallocateAndGrow(pos, 1);

        T *where = createHole(pos, i, 1);
        new (where) T(std::move(tmp));
    }

    void erase(T *b, uint32_t n)
    {
        T *e = b + n;
        Q_ASSERT(this->isMutable());
        Q_ASSERT(b < e);
        Q_ASSERT(b >= this->begin() && b < this->end());
        Q_ASSERT(e > this->begin() && e <= this->end());

        // Comply with std::vector::erase(): erased elements and all after them
        // are invalidated. However, erasing from the beginning effectively
        // means that all iterators are invalidated. We can use this freedom to
        // erase by moving towards the end.
        if (b == this->begin())
            this->ptr = e;
        else if (e != this->end())
            ::memmove(static_cast<void *>(b), static_cast<void *>(e), (static_cast<T *>(this->end()) - e) * sizeof(T));
        this->size -= n;
    }

    void eraseFirst() noexcept
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(this->size);
        ++this->ptr;
        --this->size;
    }

    void eraseLast() noexcept
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(this->size);
        --this->size;
    }

    void assign(T *b, T *e, parameter_type t) noexcept
    {
        Q_ASSERT(b <= e);
        Q_ASSERT(b >= this->begin() && e <= this->end());

        while (b != e)
            ::memcpy(static_cast<void *>(b++), static_cast<const void *>(&t), sizeof(T));
    }

    bool compare(const T *begin1, const T *begin2, size_t n) const
    {
        // only use memcmp for fundamental types or pointers.
        // Other types could have padding in the data structure or custom comparison
        // operators that would break the comparison using memcmp
        if constexpr (QArrayDataPointer<T>::pass_parameter_by_value) {
            return ::memcmp(begin1, begin2, n * sizeof(T)) == 0;
        } else {
            const T *end1 = begin1 + n;
            while (begin1 != end1) {
                if (*begin1 == *begin2) {
                    ++begin1;
                    ++begin2;
                } else {
                    return false;
                }
            }
            return true;
        }
    }

    void reallocate(uint32_t alloc, QArrayData::AllocationOption option)
    {
        auto pair = Data::reallocateUnaligned(this->d, this->ptr, alloc, option);
        assert(pair.first != nullptr);
        this->d = pair.first;
        this->ptr = pair.second;
    }
};

template <class T>
struct QArrayDataOps
        : QCommonArrayOps<T>
{

};

template <class T>
inline void qSwap(QArrayDataPointer<T> &p1, QArrayDataPointer<T> &p2) noexcept
{
    p1.swap(p2);
}

template <class T>
struct QArrayDataPointer
{
private:
    typedef QTypedArrayData<T> Data;
    typedef QArrayDataOps<T> DataOps;

public:
    enum {
        pass_parameter_by_value =
        std::is_arithmetic<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value
    };

    typedef typename std::conditional<pass_parameter_by_value, T, const T &>::type parameter_type;

    constexpr QArrayDataPointer() noexcept
            : d(nullptr), ptr(nullptr), size(0)
    {
    }

    QArrayDataPointer(const QArrayDataPointer &other) noexcept
            : d(other.d), ptr(other.ptr), size(other.size)
    {
        ref();
    }

    constexpr QArrayDataPointer(Data *header, T *adata, uint32_t n = 0) noexcept
            : d(header), ptr(adata), size(n)
    {
    }

    explicit QArrayDataPointer(std::pair<QTypedArrayData<T> *, T *> adata, uint32_t n = 0) noexcept
            : d(adata.first), ptr(adata.second), size(n)
    {
    }

    static QArrayDataPointer fromRawData(const T *rawData, uint32_t length) noexcept
    {
        assert(rawData || !length);
        return { nullptr, const_cast<T *>(rawData), length };
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

    T *data() noexcept { return ptr; }
    const T *data() const noexcept { return ptr; }

    T *begin() noexcept { return data(); }
    T *end() noexcept { return data() + size; }
    const T *begin() const noexcept { return data(); }
    const T *end() const noexcept { return data() + size; }
    const T *constBegin() const noexcept { return data(); }
    const T *constEnd() const noexcept { return data() + size; }

    void swap(QArrayDataPointer &other) noexcept
    {
        std::swap(d, other.d);
        std::swap(ptr, other.ptr);
        std::swap(size, other.size);
    }

    void clear() noexcept(std::is_nothrow_destructible<T>::value)
    {
        QArrayDataPointer tmp;
        swap(tmp);
    }

    void detach(QArrayDataPointer *old = nullptr)
    {
        if (needsDetach())
            reallocateAndGrow(QArrayData::GrowsAtEnd, 0, old);
    }

    // pass in a pointer to a default constructed QADP, to keep it alive beyond the detach() call
    void detachAndGrow(QArrayData::GrowthPosition where, uint32_t n, QArrayDataPointer *old = nullptr)
    {
        if (!needsDetach()) {
            if (!n ||
                (where == QArrayData::GrowsAtBeginning && freeSpaceAtBegin() >= n) ||
                (where == QArrayData::GrowsAtEnd && freeSpaceAtEnd() >= n))
                return;
        }
        reallocateAndGrow(where, n, old);
    }

    Q_NEVER_INLINE void reallocateAndGrow(QArrayData::GrowthPosition where, uint32_t n, QArrayDataPointer *old = nullptr)
    {
        if constexpr (alignof(T) <= alignof(std::max_align_t)) {
            if (where == QArrayData::GrowsAtEnd && !old && !needsDetach() && n > 0) {
                (*this)->reallocate(constAllocatedCapacity() - freeSpaceAtEnd() + n, QArrayData::Grow); // fast path
                return;
            }
        }

        QArrayDataPointer dp(allocateGrow(*this, n, where));
        if (where == QArrayData::GrowsAtBeginning) {
            dp.ptr += n;
            Q_ASSERT(dp.freeSpaceAtBegin() >= n);
        } else {
            Q_ASSERT(dp.freeSpaceAtEnd() >= n);
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
            Q_ASSERT(dp.size == toCopy);
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
    void setFlag(QArrayData::ArrayOptions f) noexcept { Q_ASSERT(d); d->flags |= f; }
    void clearFlag(typename Data::ArrayOptions f) noexcept { if (d) d->flags &= ~f; }

    Data *d_ptr() noexcept { return d; }
    void setBegin(T *begin) noexcept { ptr = begin; }

    uint32_t freeSpaceAtBegin() const noexcept
    {
        if (d == nullptr)
            return 0;
        return this->ptr - Data::dataStart(d, alignof(typename Data::AlignmentDummy));
    }

    uint32_t freeSpaceAtEnd() const noexcept
    {
        if (d == nullptr)
            return 0;
        return d->constAllocatedCapacity() - freeSpaceAtBegin() - this->size;
    }

    // allocate and grow. Ensure that at the minimum requiredSpace is available at the requested end
    static QArrayDataPointer allocateGrow(const QArrayDataPointer &from, uint32_t n, QArrayData::GrowthPosition position)
    {
        // calculate new capacity. We keep the free capacity at the side that does not have to grow
        // to avoid quadratic behavior with mixed append/prepend cases

        // use qMax below, because constAllocatedCapacity() can be 0 when using fromRawData()
        uint32_t minimalCapacity = std::max(from.size, from.constAllocatedCapacity()) + n;
        // subtract the free space at the side we want to allocate. This ensures that the total size requested is
        // the existing allocation at the other side + size + n.
        minimalCapacity -= (position == QArrayData::GrowsAtEnd) ? from.freeSpaceAtEnd() : from.freeSpaceAtBegin();
        uint32_t capacity = from.detachCapacity(minimalCapacity);
        const bool grows = capacity > from.constAllocatedCapacity();
        auto [header, dataPtr] = Data::allocate(capacity, grows ? QArrayData::Grow : QArrayData::KeepSize);
        const bool valid = header != nullptr && dataPtr != nullptr;
        if (!valid)
            return QArrayDataPointer(header, dataPtr);

        // Idea: * when growing backwards, adjust pointer to prepare free space at the beginning
        //       * when growing forward, adjust by the previous data pointer offset

        // TODO: what's with CapacityReserved?
        dataPtr += (position == QArrayData::GrowsAtBeginning) ? std::max<intptr_t>(0, (header->alloc - from.size - n) / 2)
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
    T *ptr;
    uint32_t size;
};

class UString {
    typedef QTypedArrayData<char16_t> Data;
public:
    typedef QArrayDataPointer<char16_t> DataPointer;

    inline constexpr UString() noexcept {}
    UString(const char16_t *unicode, int64_t size = -1);
    UString(char16_t ch);
    inline UString(const UString &other) noexcept: d(other.d) {}
    inline UString(UString &&other) noexcept
    { qSwap(d, other.d); }

    inline uint32_t size() const { return d.size; }
    inline uint32_t count() const { return d.size; }
    inline uint32_t length() const { return d.size; }
    constexpr bool isNull() const noexcept { return !(const_cast<UString*>(this)->data()); }
    inline bool isEmpty() const { return !size(); }

    inline const char16_t at(uint32_t i) const
    { assert(size_t(i) < size_t(size())); return d.data()[i]; }

    void resize(uint32_t size);
    void resize(uint32_t size, char16_t fillChar);

    [[nodiscard]] UString mid(uint32_t position, uint32_t n = -1) const;

    inline uint32_t capacity() const { return uint32_t(d->constAllocatedCapacity()); }

    inline void push_back(char16_t c) { append(c); }
    inline void push_back(const UString &s) { append(s); }
    UString &append(char16_t c);
    UString &append(const char16_t *uc, int64_t len = -1);
    UString &append(const UString &s);

    inline UString &operator+=(char16_t c) { return append(c); }

    inline UString &operator+=(const UString &s) { return append(s); }

    UString &operator=(const UString &) noexcept;

    friend bool operator==(const UString &s1, const char16_t *s2) noexcept;
    friend bool operator==(const UString &s1, const UString& s2) noexcept;
    friend bool operator!=(const UString &s1, const UString &s2) noexcept { return !(s1 == s2); }

    static UString fromUtf8(const char *utf8, uint32_t size);
    static UString fromStdString(const std::string&);

    std::string toStdString() const;

    void truncate(uint32_t size);

    inline char16_t *data()
    {
        detach();
        assert(d.data());
        return reinterpret_cast<char16_t *>(d.data());
    }
    inline const char16_t *constData() const
    { return const_cast<UString*>(this)->data(); }

    inline void detach()
    { if (d->needsDetach()) reallocData(d.size, QArrayData::KeepSize); }

    std::int32_t toInt() const;

private:
    UString(uint32_t size, bool init) noexcept;
    explicit UString(DataPointer &&dd) : d(std::move(dd)) {}
    void reallocData(uint32_t alloc, QArrayData::AllocationOption option);
    void reallocGrowData(uint32_t n);

    DataPointer d;
    static const char16_t _empty;

};

inline UString FromCodePoint(char32_t cp) {
    UString result;
    if (cp < 0x10000) {
        result.push_back(static_cast<char16_t>(cp));
    } else {
        result.push_back(static_cast<char16_t>(0xD800 + ((cp - 0x10000) >> 10)));
        result.push_back(static_cast<char16_t>(0xdc00 + ((cp - 0x10000) & 1023)));
    }
    return result;
}

inline std::string To_UTF8(const std::u32string &s) {
#ifndef _WIN32
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
#else

    auto U32ToU16 = [](char32_t cUTF32, char16_t& h, char16_t& l) -> char32_t {
            if (cUTF32 < 0x10000)
            {
                h = 0;
                l = cUTF32;
                return cUTF32;
            }
            unsigned int t = cUTF32 - 0x10000;
            h = (((t << 12) >> 22) + 0xD800);
            l = (((t << 22) >> 22) + 0xDC00);
            char32_t ret = ((h << 16) | (l & 0x0000FFFF));
            return ret;
        };

        std::u16string u16;

        for (char32_t ch : s) {
            char16_t h = 0;
            char16_t l = 0;
            U32ToU16(ch, h, l);
            u16.push_back(h);
            u16.push_back(l);
        }

        return utils::To_UTF8(u16);
#endif
}

inline void AddU32ToUtf16(UString& target, char32_t code) {
    if (code < 0x10000) {
        target.push_back(code);
    }

    std::u32string tmp;
    tmp.push_back(code);

    auto utf8 = To_UTF8(tmp);
    auto utf16 = UString::fromUtf8(utf8.c_str(), utf8.size());

    target.append(utf16);
}

namespace UChar {

    inline bool IsDecimalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39);      // 0..9
    }

    inline bool IsLineTerminator(char32_t cp) {
        return (cp == 0x0A) || (cp == 0x0D) || (cp == 0x2028) || (cp == 0x2029);
    }

    static char32_t WHITE_SPACE[] = {0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202F, 0x205F, 0x3000, 0xFEFF};

    inline bool IsWhiteSpace(char32_t cp) {
        if (cp >= 0x1680) {
            std::size_t count = sizeof(WHITE_SPACE) / sizeof(char32_t);
            for (std::size_t i = 0; i < count; i++) {
                if (WHITE_SPACE[i] == cp) return true;
            }
        }
        return (cp == 0x20) || (cp == 0x09) || (cp == 0x0B) || (cp == 0x0C) || (cp == 0xA0);
    }

    inline bool IsIdentifierStart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); // && Regex.NonAsciiIdentifierStart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsIdentifierPart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp >= 0x30 && cp <= 0x39) ||         // 0..9
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); //&& Regex.NonAsciiIdentifierPart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsHexDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39) ||    // 0..9
               (cp >= 0x41 && cp <= 0x46) ||       // A..F
               (cp >= 0x61 && cp <= 0x66);         // a..f
    }

    inline bool IsOctalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x37);      // 0..7
    }

}

inline const UString operator+(const UString &s1, const UString &s2)
{ UString t(s1); t += s2; return t; }
inline const UString operator+(const UString &s1, char16_t s2)
{ UString t(s1); t += s2; return t; }
inline const UString operator+(char16_t s1, const UString &s2)
{ UString t(s1); t += s2; return t; }

size_t qHashBits(const void *p, size_t size, size_t seed = 0) noexcept;

namespace std {

    template <>
    struct hash<UString>
    {
        std::size_t operator()(const UString& k) const
        {
            using std::size_t;
            using std::hash;
            using std::string;

            return qHashBits(k.constData(), k.size() * sizeof(char16_t));
        }
    };

}

#endif //ROCKET_BUNDLE_USTRING_H
