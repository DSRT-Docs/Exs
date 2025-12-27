// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_UTILS_H
#define EXS_UTILS_H

#include "Traits.h"
#include <utility>
#include <cstddef>

namespace Exs_ {
namespace Utils {

// Move and forward
template<typename T>
constexpr typename Traits::ExsRemoveReference<T>::type&& ExsMove(T&& t) noexcept {
    return static_cast<typename Traits::ExsRemoveReference<T>::type&&>(t);
}

template<typename T>
constexpr T&& ExsForward(typename Traits::ExsRemoveReference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& ExsForward(typename Traits::ExsRemoveReference<T>::type&& t) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value,
                  "Cannot forward rvalue as lvalue");
    return static_cast<T&&>(t);
}

// Swap
template<typename T>
void ExsSwap(T& a, T& b) noexcept(
    std::is_nothrow_move_constructible<T>::value &&
    std::is_nothrow_move_assignable<T>::value
) {
    T temp = ExsMove(a);
    a = ExsMove(b);
    b = ExsMove(temp);
}

// Exchange
template<typename T, typename U = T>
T ExsExchange(T& obj, U&& new_value) noexcept(
    std::is_nothrow_move_constructible<T>::value &&
    std::is_nothrow_assignable<T&, U&&>::value
) {
    T old_value = ExsMove(obj);
    obj = ExsForward<U>(new_value);
    return old_value;
}

// Min/Max
template<typename T>
constexpr const T& ExsMin(const T& a, const T& b) noexcept(
    noexcept(a < b)
) {
    return (a < b) ? a : b;
}

template<typename T>
constexpr const T& ExsMax(const T& a, const T& b) noexcept(
    noexcept(a < b)
) {
    return (a < b) ? b : a;
}

template<typename T, typename Compare>
constexpr const T& ExsMin(const T& a, const T& b, Compare comp) noexcept(
    noexcept(comp(a, b))
) {
    return comp(a, b) ? a : b;
}

template<typename T, typename Compare>
constexpr const T& ExsMax(const T& a, const T& b, Compare comp) noexcept(
    noexcept(comp(a, b))
) {
    return comp(a, b) ? b : a;
}

// Clamp
template<typename T>
constexpr const T& ExsClamp(const T& value, const T& low, const T& high) noexcept(
    noexcept(value < low && value < high)
) {
    return ExsMax(ExsMin(value, high), low);
}

template<typename T, typename Compare>
constexpr const T& ExsClamp(const T& value, const T& low, const T& high, Compare comp) noexcept(
    noexcept(comp(value, low) && comp(value, high))
) {
    return ExsMax(ExsMin(value, high, comp), low, comp);
}

// Pair
template<typename T1, typename T2>
struct ExsPair {
    T1 first;
    T2 second;
    
    ExsPair() = default;
    
    ExsPair(const T1& x, const T2& y) : first(x), second(y) {}
    
    template<typename U1, typename U2>
    ExsPair(U1&& x, U2&& y) : 
        first(ExsForward<U1>(x)), 
        second(ExsForward<U2>(y)) {}
    
    template<typename U1, typename U2>
    ExsPair(const ExsPair<U1, U2>& p) : first(p.first), second(p.second) {}
    
    template<typename U1, typename U2>
    ExsPair(ExsPair<U1, U2>&& p) : 
        first(ExsMove(p.first)), 
        second(ExsMove(p.second)) {}
    
    ExsPair& operator=(const ExsPair& other) {
        if (this != &other) {
            first = other.first;
            second = other.second;
        }
        return *this;
    }
    
    ExsPair& operator=(ExsPair&& other) noexcept(
        std::is_nothrow_move_assignable<T1>::value &&
        std::is_nothrow_move_assignable<T2>::value
    ) {
        first = ExsMove(other.first);
        second = ExsMove(other.second);
        return *this;
    }
    
    void ExsSwap(ExsPair& other) noexcept(
        noexcept(ExsSwap(first, other.first)) &&
        noexcept(ExsSwap(second, other.second))
    ) {
        ExsSwap(first, other.first);
        ExsSwap(second, other.second);
    }
};

template<typename T1, typename T2>
ExsPair<T1, T2> ExsMakePair(T1&& x, T2&& y) {
    return ExsPair<T1, T2>(ExsForward<T1>(x), ExsForward<T2>(y));
}

// Tuple (simplified)
template<typename... Ts>
class ExsTuple;

template<>
class ExsTuple<> {};

template<typename T, typename... Ts>
class ExsTuple<T, Ts...> : private ExsTuple<Ts...> {
private:
    T value;
    
public:
    ExsTuple() = default;
    
    ExsTuple(const T& first, const Ts&... rest) : 
        ExsTuple<Ts...>(rest...), value(first) {}
    
    template<typename U, typename... Us>
    ExsTuple(U&& first, Us&&... rest) : 
        ExsTuple<Ts...>(ExsForward<Us>(rest)...), 
        value(ExsForward<U>(first)) {}
    
    T& ExsGet() { return value; }
    const T& ExsGet() const { return value; }
    
    template<std::size_t I>
    auto& ExsGet() {
        if constexpr (I == 0) {
            return value;
        } else {
            return ExsTuple<Ts...>::template ExsGet<I - 1>();
        }
    }
    
    template<std::size_t I>
    const auto& ExsGet() const {
        if constexpr (I == 0) {
            return value;
        } else {
            return ExsTuple<Ts...>::template ExsGet<I - 1>();
        }
    }
};

template<typename... Ts>
ExsTuple<Ts...> ExsMakeTuple(Ts&&... args) {
    return ExsTuple<Ts...>(ExsForward<Ts>(args)...);
}

// Integer sequence
template<std::size_t... Ints>
struct ExsIndexSequence {};

template<std::size_t N, std::size_t... Ints>
struct ExsMakeIndexSequence : ExsMakeIndexSequence<N - 1, N - 1, Ints...> {};

template<std::size_t... Ints>
struct ExsMakeIndexSequence<0, Ints...> {
    using type = ExsIndexSequence<Ints...>;
};

template<std::size_t N>
using ExsMakeIndexSequenceT = typename ExsMakeIndexSequence<N>::type;

// Scope guard
template<typename Func>
class ExsScopeGuard {
private:
    Func func;
    bool active;
    
public:
    explicit ExsScopeGuard(Func&& f) : func(ExsMove(f)), active(true) {}
    
    ExsScopeGuard(ExsScopeGuard&& other) noexcept : 
        func(ExsMove(other.func)), active(other.active) {
        other.active = false;
    }
    
    ~ExsScopeGuard() {
        if (active) {
            func();
        }
    }
    
    void ExsDismiss() { active = false; }
    
    ExsScopeGuard(const ExsScopeGuard&) = delete;
    ExsScopeGuard& operator=(const ExsScopeGuard&) = delete;
    ExsScopeGuard& operator=(ExsScopeGuard&&) = delete;
};

template<typename Func>
ExsScopeGuard<Func> ExsMakeScopeGuard(Func&& func) {
    return ExsScopeGuard<Func>(ExsForward<Func>(func));
}

// Defer macro
#define EXS_DEFER(code) \
    auto EXS_CONCAT(_exs_defer_, __LINE__) = \
        Exs_::Utils::ExsMakeScopeGuard([&]() { code; })

#define EXS_CONCAT_IMPL(x, y) x##y
#define EXS_CONCAT(x, y) EXS_CONCAT_IMPL(x, y)

} // namespace Utils
} // namespace Exs_

#endif // EXS_UTILS_H
