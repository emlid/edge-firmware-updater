#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <memory>
#include <QtCore>

#include <memory>
#include <QtCore>
#include <type_traits>

namespace util {
    template<typename T> class Optional;

    template<typename M>  Optional<typename std::decay<M>::type> make_optional(M&& value) {
        return {std::forward<M>(value)};
    }
}


template<typename T>
class util::Optional
{
    template<typename M>
    friend Optional<M> util::make_optional(M&&);

public:
    Optional(T&& value)
        : _ptr(new T(std::forward<T>(value)))
    { }

    Optional(T const& value)
        : _ptr(new T(value))
    { }

    Optional(void)
        : _ptr(nullptr)
    { }

    Optional(int errcode, QString msg)
        : _ptr(nullptr)
    { }

    Optional(Optional&& value)
        : _ptr(std::move(value._ptr))
    { }

    Optional operator =(Optional&& value) {
        return Optional(std::move(value));
    }

    bool present(void) const {
        return _ptr != nullptr;
    }

    T release(void) {
        Q_ASSERT(_ptr != nullptr);
        auto obj = T(std::move(*(_ptr.release())));
        return obj;
    }

    T& get(void) {
        Q_ASSERT(_ptr != nullptr);
        return *_ptr;
    }

private:
    std::unique_ptr<T> _ptr;
};

#endif // OPTIONAL_H
