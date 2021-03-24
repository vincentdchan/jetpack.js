//
// Created by Duzhong Chen on 2021/3/23.
//

#ifndef ROCKET_BUNDLE_COMMONARRAYOPS_H
#define ROCKET_BUNDLE_COMMONARRAYOPS_H

#include "QArrayData.h"
#include <cassert>
#include <memory>

template <class T> struct QArrayDataPointer;
template <class T> struct QTypedArrayData;

template <class T>
struct QGenericArrayOps
        : public QArrayDataPointer<T>
{
    static_assert (std::is_nothrow_destructible_v<T>, "Types with throwing destructors are not supported in Qt containers.");

protected:
    typedef QTypedArrayData<T> Data;
    using DataPointer = QArrayDataPointer<T>;

public:
    typedef typename QArrayDataPointer<T>::parameter_type parameter_type;

    void appendInitialize(uint32_t newSize)
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(!this->isShared());
        Q_ASSERT(newSize > this->size);
        Q_ASSERT(newSize - this->size <= this->freeSpaceAtEnd());

        T *const b = this->begin();
        do {
            new (b + this->size) T;
        } while (++this->size != newSize);
    }

    void copyAppend(const T *b, const T *e)
    {
        Q_ASSERT(this->isMutable() || b == e);
        Q_ASSERT(!this->isShared() || b == e);
        Q_ASSERT(b <= e);
        Q_ASSERT((e - b) <= this->freeSpaceAtEnd());

        if (b == e) // short-cut and handling the case b and e == nullptr
            return;

        T *data = this->begin();
        while (b < e) {
            new (data + this->size) T(*b);
            ++b;
            ++this->size;
        }
    }

    void copyAppend(uint32_t n, parameter_type t)
    {
        Q_ASSERT(!this->isShared() || n == 0);
        Q_ASSERT(this->freeSpaceAtEnd() >= n);
        if (!n)
            return;

        T *data = this->begin();
        while (n--) {
            new (data + this->size) T(t);
            ++this->size;
        }
    }

    void moveAppend(T *b, T *e)
    {
        Q_ASSERT(this->isMutable() || b == e);
        Q_ASSERT(!this->isShared() || b == e);
        Q_ASSERT(b <= e);
        Q_ASSERT((e - b) <= this->freeSpaceAtEnd());

        if (b == e)
            return;

        T *data = this->begin();
        while (b < e) {
            new (data + this->size) T(std::move(*b));
            ++b;
            ++this->size;
        }
    }

    void truncate(size_t newSize)
    {
        assert(this->isMutable());
        assert(!this->isShared());
        assert(newSize < size_t(this->size));

        std::destroy(this->begin() + newSize, this->end());
        this->size = newSize;
    }

    void destroyAll() // Call from destructors, ONLY
    {
        assert(this->d);
        // As this is to be called only from destructor, it doesn't need to be
        // exception safe; size not updated.

        assert(this->d->ref_.load(std::memory_order_relaxed) == 0);

        std::destroy(this->begin(), this->end());
    }

    struct Inserter
    {
        QArrayDataPointer<T> *data;
        uint32_t increment = 1;
        T *begin;
        uint32_t size;

        uint32_t sourceCopyConstruct, nSource, move, sourceCopyAssign;
        T *end, *last, *where;

        Inserter(QArrayDataPointer<T> *d, QArrayData::GrowthPosition pos)
                : data(d), increment(pos == QArrayData::GrowsAtBeginning ? -1 : 1)
        {
            begin = d->ptr;
            size = d->size;
            if (increment < 0)
                begin += size - 1;
        }
        ~Inserter() {
            if (increment < 0)
                begin -= size - 1;
            data->ptr = begin;
            data->size = size;
        }

        void setup(uint32_t pos, uint32_t n)
        {

            if (increment > 0) {
                end = begin + size;
                last = end - 1;
                where = begin + pos;
                uint32_t dist = size - pos;
                sourceCopyConstruct = 0;
                nSource = n;
                move = n - dist; // smaller 0
                sourceCopyAssign = n;
                if (n > dist) {
                    sourceCopyConstruct = n - dist;
                    move = 0;
                    sourceCopyAssign -= sourceCopyConstruct;
                }
            } else {
                end = begin - size;
                last = end + 1;
                where = end + pos;
                sourceCopyConstruct = 0;
                nSource = -n;
                move = pos - n; // larger 0
                sourceCopyAssign = -n;
                if (n > pos) {
                    sourceCopyConstruct = pos - n;
                    move = 0;
                    sourceCopyAssign -= sourceCopyConstruct;
                }
            }
        }

        void insert(uint32_t pos, const T *source, uint32_t n)
        {
            uint32_t oldSize = size;

            setup(pos, n);
            if (increment < 0)
                source += n - 1;

            // first create new elements at the end, by copying from elements
            // to be inserted (if they extend past the current end of the array)
            for (uint32_t i = 0; i != sourceCopyConstruct; i += increment) {
                new (end + i) T(source[nSource - sourceCopyConstruct + i]);
                ++size;
            }
            assert(size <= oldSize + n);

            // now move construct new elements at the end from existing elements inside
            // the array.
            for (uint32_t i = sourceCopyConstruct; i != nSource; i += increment) {
                new (end + i) T(std::move(*(end + i - nSource)));
                ++size;
            }
            // array has the new size now!
            assert(size == oldSize + n);

            // now move assign existing elements towards the end
            for (uint32_t i = 0; i != move; i -= increment)
                last[i] = std::move(last[i - nSource]);

            // finally copy the remaining elements from source over
            for (uint32_t i = 0; i != sourceCopyAssign; i += increment)
                where[i] = source[i];
        }

        void insert(uint32_t pos, const T &t, uint32_t n)
        {
            uint32_t oldSize = size;

            setup(pos, n);

            // first create new elements at the end, by copying from elements
            // to be inserted (if they extend past the current end of the array)
            for (uint32_t i = 0; i != sourceCopyConstruct; i += increment) {
                new (end + i) T(t);
                ++size;
            }
            assert(size <= oldSize + n);

            // now move construct new elements at the end from existing elements inside
            // the array.
            for (uint32_t i = sourceCopyConstruct; i != nSource; i += increment) {
                new (end + i) T(std::move(*(end + i - nSource)));
                ++size;
            }
            // array has the new size now!
            assert(size == oldSize + n);

            // now move assign existing elements towards the end
            for (uint32_t i = 0; i != move; i -= increment)
                last[i] = std::move(last[i - nSource]);

            // finally copy the remaining elements from source over
            for (uint32_t i = 0; i != sourceCopyAssign; i += increment)
                where[i] = t;
        }

        void insertOne(uint32_t pos, T &&t)
        {
            setup(pos, 1);

            if (sourceCopyConstruct) {
                uint32_t(sourceCopyConstruct == increment);
                new (end) T(std::move(t));
                ++size;
            } else {
                // create a new element at the end by move constructing one existing element
                // inside the array.
                new (end) T(std::move(*(end - increment)));
                ++size;

                // now move assign existing elements towards the end
                for (uint32_t i = 0; i != move; i -= increment)
                    last[i] = std::move(last[i - increment]);

                // and move the new item into place
                *where = std::move(t);
            }
        }
    };

    void insert(uint32_t i, const T *data, uint32_t n)
    {
        typename Data::GrowthPosition pos = Data::GrowsAtEnd;
        if (this->size != 0 && i <= (this->size >> 1))
            pos = Data::GrowsAtBeginning;
        DataPointer oldData;
        this->detachAndGrow(pos, n, &oldData);
        Q_ASSERT((pos == Data::GrowsAtBeginning && this->freeSpaceAtBegin() >= n) ||
                 (pos == Data::GrowsAtEnd && this->freeSpaceAtEnd() >= n));

        Inserter(this, pos).insert(i, data, n);
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

        Inserter(this, pos).insert(i, copy, n);
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

        Inserter(this, pos).insertOne(i, std::move(tmp));
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
        if (b == this->begin()) {
            this->ptr = e;
        } else {
            const T *const end = this->end();

            // move (by assignment) the elements from e to end
            // onto b to the new end
            while (e != end) {
                *b = std::move(*e);
                ++b;
                ++e;
            }
        }
        this->size -= n;
        std::destroy(b, e);
    }

    void eraseFirst() noexcept
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(this->size);
        this->begin()->~T();
        ++this->ptr;
        --this->size;
    }

    void eraseLast() noexcept
    {
        Q_ASSERT(this->isMutable());
        Q_ASSERT(this->size);
        (this->end() - 1)->~T();
        --this->size;
    }


    void assign(T *b, T *e, parameter_type t)
    {
        Q_ASSERT(b <= e);
        Q_ASSERT(b >= this->begin() && e <= this->end());

        while (b != e)
            *b++ = t;
    }

    bool compare(const T *begin1, const T *begin2, size_t n) const
    {
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
};

template <class T, class = void>
struct QArrayOpsSelector
{
    typedef QGenericArrayOps<T> Type;
};

template <typename Iterator>
using IfIsForwardIterator = typename std::enable_if<
        std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::forward_iterator_tag>::value,
        bool>::type;

template <typename T>
class QTypedArrayData;

template <typename T>
struct QArrayDataPointer;

template <class T>
struct QCommonArrayOps : QArrayOpsSelector<T>::Type
{
    using Base = typename QArrayOpsSelector<T>::Type;
    using Data = QTypedArrayData<T>;
    using DataPointer = QArrayDataPointer<T>;
    using parameter_type = typename Base::parameter_type;

protected:
    using Self = QCommonArrayOps<T>;

public:
    // using Base::truncate;
    // using Base::destroyAll;
    // using Base::assign;
    // using Base::compare;

    template<typename It>
    void appendIteratorRange(It b, It e, IfIsForwardIterator<It> = true)
    {
        assert(this->isMutable() || b == e);
        assert(!this->isShared() || b == e);
        const uint32_t distance = std::distance(b, e);
        assert(distance >= 0 && distance <= this->allocatedCapacity() - this->size);

        T *iter = this->end();
        for (; b != e; ++iter, ++b) {
            new (iter) T(*b);
            ++this->size;
        }
    }
};

#endif //ROCKET_BUNDLE_COMMONARRAYOPS_H
