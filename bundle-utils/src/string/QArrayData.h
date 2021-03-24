//
// Created by Duzhong Chen on 2021/3/24.
//

#ifndef ROCKET_BUNDLE_QARRAYDATA_H
#define ROCKET_BUNDLE_QARRAYDATA_H

#include <cinttypes>
#include <utility>
#include <atomic>

class QFlag
{
    int i;
public:
    constexpr inline QFlag(int value) noexcept : i(value) {}
    constexpr inline operator int() const noexcept { return i; }

#if !defined(Q_CC_MSVC)
    // Microsoft Visual Studio has buggy behavior when it comes to
    // unsigned enums: even if the enum is unsigned, the enum tags are
    // always signed
#  if !defined(__LP64__) && !defined(Q_CLANG_QDOC)
    constexpr inline QFlag(long value) noexcept : i(int(value)) {}
    constexpr inline QFlag(ulong value) noexcept : i(int(long(value))) {}
#  endif
    constexpr inline QFlag(uint32_t value) noexcept : i(int(value)) {}
    constexpr inline QFlag(short value) noexcept : i(int(value)) {}
    constexpr inline QFlag(uint16_t value) noexcept : i(int(uint16_t (value))) {}
#endif
};

template<typename Enum>
class QFlags
{
    static_assert((sizeof(Enum) <= sizeof(int)),
                  "QFlags uses an int as storage, so an enum with underlying "
                  "long long will overflow.");
    static_assert((std::is_enum<Enum>::value), "QFlags is only usable on enumeration types.");

public:
#if defined(Q_CC_MSVC) || defined(Q_CLANG_QDOC)
    // see above for MSVC
    // the definition below is too complex for qdoc
    typedef int Int;
#else
    typedef typename std::conditional<
            std::is_unsigned<typename std::underlying_type<Enum>::type>::value,
            unsigned int,
            signed int
    >::type Int;
#endif
    typedef Enum enum_type;
    // compiler-generated copy/move ctor/assignment operators are fine!
#ifdef Q_CLANG_QDOC
    constexpr inline QFlags(const QFlags &other);
    constexpr inline QFlags &operator=(const QFlags &other);
#endif
    constexpr inline QFlags() noexcept : i(0) {}
    constexpr inline QFlags(Enum flags) noexcept : i(Int(flags)) {}
    constexpr inline QFlags(QFlag flag) noexcept : i(flag) {}

    constexpr inline QFlags(std::initializer_list<Enum> flags) noexcept
            : i(initializer_list_helper(flags.begin(), flags.end())) {}

    constexpr inline QFlags &operator&=(int mask) noexcept { i &= mask; return *this; }
    constexpr inline QFlags &operator&=(uint32_t mask) noexcept { i &= mask; return *this; }
    constexpr inline QFlags &operator&=(Enum mask) noexcept { i &= Int(mask); return *this; }
    constexpr inline QFlags &operator|=(QFlags other) noexcept { i |= other.i; return *this; }
    constexpr inline QFlags &operator|=(Enum other) noexcept { i |= Int(other); return *this; }
    constexpr inline QFlags &operator^=(QFlags other) noexcept { i ^= other.i; return *this; }
    constexpr inline QFlags &operator^=(Enum other) noexcept { i ^= Int(other); return *this; }

    constexpr inline operator Int() const noexcept { return i; }

    constexpr inline QFlags operator|(QFlags other) const noexcept { return QFlags(QFlag(i | other.i)); }
    constexpr inline QFlags operator|(Enum other) const noexcept { return QFlags(QFlag(i | Int(other))); }
    constexpr inline QFlags operator^(QFlags other) const noexcept { return QFlags(QFlag(i ^ other.i)); }
    constexpr inline QFlags operator^(Enum other) const noexcept { return QFlags(QFlag(i ^ Int(other))); }
    constexpr inline QFlags operator&(int mask) const noexcept { return QFlags(QFlag(i & mask)); }
    constexpr inline QFlags operator&(uint32_t mask) const noexcept { return QFlags(QFlag(i & mask)); }
    constexpr inline QFlags operator&(Enum other) const noexcept { return QFlags(QFlag(i & Int(other))); }
    constexpr inline QFlags operator~() const noexcept { return QFlags(QFlag(~i)); }

    constexpr inline void operator+(QFlags other) const noexcept = delete;
    constexpr inline void operator+(Enum other) const noexcept = delete;
    constexpr inline void operator+(int other) const noexcept = delete;
    constexpr inline void operator-(QFlags other) const noexcept = delete;
    constexpr inline void operator-(Enum other) const noexcept = delete;
    constexpr inline void operator-(int other) const noexcept = delete;

    constexpr inline bool operator!() const noexcept { return !i; }

    constexpr inline bool testFlag(Enum flag) const noexcept { return (i & Int(flag)) == Int(flag) && (Int(flag) != 0 || i == Int(flag) ); }
    constexpr inline QFlags &setFlag(Enum flag, bool on = true) noexcept
    {
        return on ? (*this |= flag) : (*this &= ~Int(flag));
    }

private:
    constexpr static inline Int initializer_list_helper(typename std::initializer_list<Enum>::const_iterator it,
                                                        typename std::initializer_list<Enum>::const_iterator end)
    noexcept
    {
        return (it == end ? Int(0) : (Int(*it) | initializer_list_helper(it + 1, end)));
    }

    Int i;
};

#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef QFlags<Enum> Flags;


struct QArrayData
{
    enum AllocationOption {
        Grow,
        KeepSize
    };

    enum GrowthPosition {
        GrowsAtEnd,
        GrowsAtBeginning
    };

    enum ArrayOption {
        ArrayOptionDefault = 0,
        CapacityReserved     = 0x1  //!< the capacity was reserved by the user, try to keep it
    };
    Q_DECLARE_FLAGS(ArrayOptions, ArrayOption)

    std::atomic<int32_t> ref_;
    uint32_t flags;
    uint32_t alloc;

    uint32_t allocatedCapacity() noexcept
    {
        return alloc;
    }

    uint32_t constAllocatedCapacity() const noexcept
    {
        return alloc;
    }

    /// Returns true if sharing took place
    bool ref() noexcept
    {
        ++ref_;
        return true;
    }

    /// Returns false if deallocation is necessary
    bool deref() noexcept
    {
        return --ref_ != 0;
    }

    bool isShared() const noexcept
    {
        return ref_.load(std::memory_order_relaxed) != 1;
    }

    // Returns true if a detach is necessary before modifying the data
    // This method is intentionally not const: if you want to know whether
    // detaching is necessary, you should be in a non-const function already
    bool needsDetach() const noexcept
    {
        return ref_.load(std::memory_order_relaxed) > 1;
    }

    uint32_t detachCapacity(uint32_t newSize) const noexcept
    {
        if (flags & CapacityReserved && newSize < constAllocatedCapacity())
            return constAllocatedCapacity();
        return newSize;
    }

    static void *allocate(QArrayData **pdata, uint32_t objectSize, uint32_t alignment,
                          uint32_t capacity, AllocationOption option = QArrayData::KeepSize) noexcept;
    [[nodiscard]] static std::pair<QArrayData *, void *> reallocateUnaligned(QArrayData *data, void *dataPointer,
                                                                             uint32_t objectSize, uint32_t newCapacity, AllocationOption option) noexcept;
    static void deallocate(QArrayData *data, uint32_t objectSize,
                           uint32_t alignment) noexcept;
};


#endif //ROCKET_BUNDLE_QARRAYDATA_H
