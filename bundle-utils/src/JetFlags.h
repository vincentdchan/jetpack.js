//
// Created by Duzhong Chen on 2021/4/11.
//

#ifndef ROCKET_BUNDLE_JETFLAGS_H
#define ROCKET_BUNDLE_JETFLAGS_H

class JetFlag
{
    int i;
public:
    constexpr inline JetFlag(int value) noexcept : i(value) {}
    constexpr inline operator int() const noexcept { return i; }

#if !defined(Q_CC_MSVC)
    // Microsoft Visual Studio has buggy behavior when it comes to
    // unsigned enums: even if the enum is unsigned, the enum tags are
    // always signed
#  if !defined(__LP64__) && !defined(Q_CLANG_QDOC)
    constexpr inline JetFlag(long value) noexcept : i(int(value)) {}
    constexpr inline JetFlag(unsigned long value) noexcept : i(int(long(value))) {}
#  endif
    constexpr inline JetFlag(uint32_t value) noexcept : i(int(value)) {}
    constexpr inline JetFlag(short value) noexcept : i(int(value)) {}
    constexpr inline JetFlag(uint16_t value) noexcept : i(int(uint16_t (value))) {}
#endif
};

template<typename Enum>
class JetFlags
{
    static_assert((sizeof(Enum) <= sizeof(int)),
                  "JetFlags uses an int as storage, so an enum with underlying "
                  "long long will overflow.");
    static_assert((std::is_enum<Enum>::value), "JetFlags is only usable on enumeration types.");

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
    constexpr inline JetFlags(const JetFlags &other);
    constexpr inline JetFlags &operator=(const JetFlags &other);
#endif
    constexpr inline JetFlags() noexcept : i(0) {}
    constexpr inline JetFlags(Enum flags) noexcept : i(Int(flags)) {}
    constexpr inline JetFlags(JetFlag flag) noexcept : i(flag) {}

    constexpr inline JetFlags(std::initializer_list<Enum> flags) noexcept
            : i(initializer_list_helper(flags.begin(), flags.end())) {}

    constexpr inline JetFlags &operator&=(int mask) noexcept { i &= mask; return *this; }
    constexpr inline JetFlags &operator&=(uint32_t mask) noexcept { i &= mask; return *this; }
    constexpr inline JetFlags &operator&=(Enum mask) noexcept { i &= Int(mask); return *this; }
    constexpr inline JetFlags &operator|=(JetFlags other) noexcept { i |= other.i; return *this; }
    constexpr inline JetFlags &operator|=(Enum other) noexcept { i |= Int(other); return *this; }
    constexpr inline JetFlags &operator^=(JetFlags other) noexcept { i ^= other.i; return *this; }
    constexpr inline JetFlags &operator^=(Enum other) noexcept { i ^= Int(other); return *this; }

    constexpr inline operator Int() const noexcept { return i; }

    constexpr inline JetFlags operator|(JetFlags other) const noexcept { return JetFlags(JetFlag(i | other.i)); }
    constexpr inline JetFlags operator|(Enum other) const noexcept { return JetFlags(JetFlag(i | Int(other))); }
    constexpr inline JetFlags operator^(JetFlags other) const noexcept { return JetFlags(JetFlag(i ^ other.i)); }
    constexpr inline JetFlags operator^(Enum other) const noexcept { return JetFlags(JetFlag(i ^ Int(other))); }
    constexpr inline JetFlags operator&(int mask) const noexcept { return JetFlags(JetFlag(i & mask)); }
    constexpr inline JetFlags operator&(uint32_t mask) const noexcept { return JetFlags(JetFlag(i & mask)); }
    constexpr inline JetFlags operator&(Enum other) const noexcept { return JetFlags(JetFlag(i & Int(other))); }
    constexpr inline JetFlags operator~() const noexcept { return JetFlags(JetFlag(~i)); }

    constexpr inline void operator+(JetFlags other) const noexcept = delete;
    constexpr inline void operator+(Enum other) const noexcept = delete;
    constexpr inline void operator+(int other) const noexcept = delete;
    constexpr inline void operator-(JetFlags other) const noexcept = delete;
    constexpr inline void operator-(Enum other) const noexcept = delete;
    constexpr inline void operator-(int other) const noexcept = delete;

    constexpr inline bool operator!() const noexcept { return !i; }

    constexpr inline bool testFlag(Enum flag) const noexcept { return (i & Int(flag)) == Int(flag) && (Int(flag) != 0 || i == Int(flag) ); }
    constexpr inline JetFlags &setFlag(Enum flag, bool on = true) noexcept
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

#define JET_DECLARE_FLAGS(Flags, Enum)\
typedef JetFlags<Enum> Flags;


#endif //ROCKET_BUNDLE_JETFLAGS_H
