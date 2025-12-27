// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_TRAITS_H
#define EXS_TRAITS_H

#include <type_traits>
#include <cstddef>

namespace Exs_ {
namespace Traits {

// Type categories
template<typename T>
struct ExsIsIntegral : public std::is_integral<T> {};

template<typename T>
struct ExsIsFloatingPoint : public std::is_floating_point<T> {};

template<typename T>
struct ExsIsArithmetic : public std::is_arithmetic<T> {};

template<typename T>
struct ExsIsPointer : public std::is_pointer<T> {};

template<typename T>
struct ExsIsReference : public std::is_reference<T> {};

template<typename T>
struct ExsIsConst : public std::is_const<T> {};

template<typename T>
struct ExsIsVolatile : public std::is_volatile<T> {};

template<typename T>
struct ExsIsTrivial : public std::is_trivial<T> {};

template<typename T>
struct ExsIsStandardLayout : public std::is_standard_layout<T> {};

template<typename T>
struct ExsIsPOD : public std::is_pod<T> {};

template<typename T>
struct ExsIsEmpty : public std::is_empty<T> {};

template<typename T>
struct ExsIsPolymorphic : public std::is_polymorphic<T> {};

template<typename T>
struct ExsIsAbstract : public std::is_abstract<T> {};

template<typename T>
struct ExsIsFinal : public std::is_final<T> {};

// Type transformations
template<typename T>
using ExsRemoveConst = typename std::remove_const<T>::type;

template<typename T>
using ExsRemoveVolatile = typename std::remove_volatile<T>::type;

template<typename T>
using ExsRemoveCV = typename std::remove_cv<T>::type;

template<typename T>
using ExsAddConst = typename std::add_const<T>::type;

template<typename T>
using ExsAddVolatile = typename std::add_volatile<T>::type;

template<typename T>
using ExsAddCV = typename std::add_cv<T>::type;

template<typename T>
using ExsRemoveReference = typename std::remove_reference<T>::type;

template<typename T>
using ExsAddLValueReference = typename std::add_lvalue_reference<T>::type;

template<typename T>
using ExsAddRValueReference = typename std::add_rvalue_reference<T>::type;

template<typename T>
using ExsRemovePointer = typename std::remove_pointer<T>::type;

template<typename T>
using ExsAddPointer = typename std::add_pointer<T>::type;

template<typename T>
using ExsDecay = typename std::decay<T>::type;

// Type queries
template<typename T>
struct ExsAlignmentOf : public std::alignment_of<T> {};

template<typename T>
struct ExsSizeOf : public std::integral_constant<std::size_t, sizeof(T)> {};

template<typename T>
struct ExsRank : public std::rank<T> {};

template<typename T, unsigned N = 0>
struct ExsExtent : public std::extent<T, N> {};

// Type comparisons
template<typename T, typename U>
struct ExsIsSame : public std::is_same<T, U> {};

template<typename Base, typename Derived>
struct ExsIsBaseOf : public std::is_base_of<Base, Derived> {};

template<typename From, typename To>
struct ExsIsConvertible : public std::is_convertible<From, To> {};

// Custom type traits
template<typename T>
struct ExsIsTriviallyCopyable : public std::is_trivially_copyable<T> {};

template<typename T>
struct ExsIsTriviallyDestructible : public std::is_trivially_destructible<T> {};

template<typename T>
struct ExsIsTriviallyDefaultConstructible : 
    public std::is_trivially_default_constructible<T> {};

template<typename T>
struct ExsIsTriviallyMoveConstructible : 
    public std::is_trivially_move_constructible<T> {};

template<typename T>
struct ExsIsTriviallyCopyConstructible : 
    public std::is_trivially_copy_constructible<T> {};

template<typename T>
struct ExsIsTriviallyMoveAssignable : 
    public std::is_trivially_move_assignable<T> {};

template<typename T>
struct ExsIsTriviallyCopyAssignable : 
    public std::is_trivially_copy_assignable<T> {};

// Enable if
template<bool B, typename T = void>
using ExsEnableIf = typename std::enable_if<B, T>::type;

template<bool B, typename T, typename F>
using ExsConditional = typename std::conditional<B, T, F>::type;

// Remove const/volatile/reference
template<typename T>
using ExsBareType = ExsRemoveCV<ExsRemoveReference<T>>;

// Type identity
template<typename T>
struct ExsTypeIdentity {
    using type = T;
};

// Void type
template<typename...>
using ExsVoid = void;

// Detection idiom
namespace detail {
    template<typename...>
    struct ExsVoider {
        using type = void;
    };
    
    template<typename... Ts>
    using ExsVoidT = typename ExsVoider<Ts...>::type;
    
    template<typename, template<typename...> class, typename = ExsVoidT<>>
    struct ExsDetector : std::false_type {};
    
    template<typename T, template<typename...> class Op>
    struct ExsDetector<T, Op, ExsVoidT<Op<T>>> : std::true_type {};
}

template<template<typename...> class Op, typename... Ts>
using ExsIsDetected = typename detail::ExsDetector<Ts..., Op>::type;

template<template<typename...> class Op, typename... Ts>
using ExsDetectedOr = std::experimental::detected_or<Ts..., Op>;

template<typename Default, template<typename...> class Op, typename... Ts>
using ExsDetectedOrT = typename ExsDetectedOr<Op, Default, Ts...>::type;

// Constexpr if helper
template<typename T>
constexpr bool ExsAlwaysFalse = false;

// Type list
template<typename... Ts>
struct ExsTypeList {};

// Type at index
template<std::size_t I, typename... Ts>
struct ExsTypeAt;

template<std::size_t I, typename T, typename... Ts>
struct ExsTypeAt<I, T, Ts...> : ExsTypeAt<I - 1, Ts...> {};

template<typename T, typename... Ts>
struct ExsTypeAt<0, T, Ts...> {
    using type = T;
};

template<std::size_t I, typename... Ts>
using ExsTypeAtT = typename ExsTypeAt<I, Ts...>::type;

// Index of type
template<typename T, typename... Ts>
struct ExsIndexOf;

template<typename T, typename... Ts>
struct ExsIndexOf<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template<typename T, typename U, typename... Ts>
struct ExsIndexOf<T, U, Ts...> : 
    std::integral_constant<std::size_t, 1 + ExsIndexOf<T, Ts...>::value> {};

template<typename T>
struct ExsIndexOf<T> : std::integral_constant<std::size_t, 0> {
    static_assert(ExsAlwaysFalse<T>, "Type not found in type list");
};

} // namespace Traits
} // namespace Exs_

#endif // EXS_TRAITS_H
