#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <memory>
#include <QtCore>

#include <memory>
#include <QtCore>
#include <type_traits>

namespace util {
    template<typename T> class Optional;
}


template<typename T>
class util::Optional
{
public:
    Optional(T&& other)
        : _objptr(new T(std::forward<T>(other)))
    { }

    Optional(T const& other)
        : _objptr(new T(other))
    { }

    Optional(void)
        : _objptr(nullptr)
    { }

    Optional(Optional&& value)
        : _objptr(std::move(value._objptr))
    { }

    Optional(Optional const& value)
        : _objptr(value._objptr)
    { }

    Optional& operator =(Optional&& value) {
        _objptr = std::move(value._objptr);
        return *this;
    }

    Optional& operator =(Optional const& value) {
        _objptr = value._objptr;
        return *this;
    }

    void reset(T const& value) {
        _objptr.reset(new T(value));
    }

    void reset(T&& value) {
        _objptr.reset(new T(std::move(value)));
    }

    bool present(void) const {
        return _objptr != nullptr;
    }

    T release(void) {
        Q_CHECK_PTR(_objptr.get());
        _handleCopy();

        auto obj = T(std::move(*_objptr));
        _objptr.reset();
        return obj;
    }

    T& get(void) {
        Q_ASSERT(_objptr);
        _handleCopy();

        return *_objptr;
    }

    T const& get(void) const {
        Q_ASSERT(_objptr);
        return *_objptr;
    }

private:
    void _handleCopy(void) {
        if (_objptr.unique()) {
            _objptr.reset(new T(*_objptr));
        }
    }

    std::shared_ptr<T> _objptr;
};

#endif // OPTIONAL_H
