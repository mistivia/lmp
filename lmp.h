/**
* Copyright (c) 2026 Mistivia <i@mistivia.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <type_traits>

namespace lmp {

template<typename T, typename = void>
struct force_impl {
    using type = T;
};

template<typename T>
struct force_impl<T, std::void_t<typename T::type>> {
    using type = typename T::type;
};

template<typename T>
using force = typename force_impl<T>::type;

// macros

#define let_lazy(__name__, ...) \
    struct __name__ { \
        using type = typename __VA_ARGS__::type; \
    };

#define meta_fn(__name__, ...) template<__VA_ARGS__> struct __name__

#define meta_return(...) using type = ::lmp::force<__VA_ARGS__>

#define has_value static constexpr int value = type::value

// data constructor

struct nil {};

template<typename head, typename tail>
struct cons {
    using car = head;
    using cdr = tail;
};

// data accessor

template<typename lst>
using car = typename force<lst>::car;

template<typename lst>
using cdr = typename force<lst>::cdr;

template<typename lst>
using cadr = car<cdr<lst>>;
template<typename lst>
using caar = car<car<lst>>;
template<typename lst>
using cdar = cdr<car<lst>>;
template<typename lst>
using cddr = cdr<cdr<lst>>;


// wrapper for integer

template<int N>
using Int = std::integral_constant<int, N>;

// type function for comparing

template<typename L, typename R>
using eq = std::is_same<force<L>, force<R>>;

template<typename T>
using nilp = eq<T, nil>;

template<typename B>
using not_ = std::bool_constant<!force<B>::value>;

// cond

template<typename condition, typename tb, typename fb>
struct cond_impl;

template<typename tb, typename fb>
struct cond_impl<std::true_type, tb, fb> {
  using type = force<tb>;
};

template<typename tb, typename fb>
struct cond_impl<std::false_type, tb, fb> {
  using type = force<fb>;
};

meta_fn(cond, class Cond, class tb, class fb) {
    using condition = force<Cond>;
    meta_return (cond_impl<condition, tb, fb>);
};

// logical primitive (with short-circuit)

meta_fn(and_, class... Bs);
    template<>
    struct and_<> {
        meta_return (std::true_type);
        has_value;
    };
    template<class B, class... Bs>
    struct and_<B, Bs...> {
        let_lazy(rest, and_<Bs...>);
        meta_return (cond<B, rest, std::false_type>);
        has_value;
    };

meta_fn(or_, class... Bs);
    template<>
    struct or_<> {
        meta_return (std::false_type);
        has_value;
    };
    template<class B, class... Bs>
    struct or_<B, Bs...> {
        let_lazy(rest, or_<Bs...>);
        meta_return (cond<B, std::true_type, rest>);
        has_value;
    };


// basic arithmetics

template<typename L, typename R>
using equal = std::bool_constant<force<L>::value == force<R>::value>;

template<typename L, typename R>
using nequal = std::bool_constant<force<L>::value != force<R>::value>;

template<typename L, typename R>
using gt = std::bool_constant<(force<L>::value > force<R>::value)>;

template<typename L, typename R>
using ge = std::bool_constant<force<L>::value >= force<R>::value>;

template<typename L, typename R>
using lt = std::bool_constant<(force<L>::value < force<R>::value)>;

template<typename L, typename R>
using le = std::bool_constant<(force<L>::value <= force<R>::value)>;

meta_fn(add2, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value + force<rhs>::value)>);
};

template<typename T>
using mandates = typename std::enable_if<T::value>::type;

meta_fn(add , class... args);
    template<class arg, class... args>
    struct add<arg, args...> {
        meta_return (add2<arg, add<args...>>);
        has_value;
    };
    template<>
    struct add<> {
        meta_return (Int<0>);
        has_value;
    };

meta_fn(neg, class rhs) {
    meta_return (Int<(- force<rhs>::value)>);
    has_value;
};

meta_fn(sub, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value - force<rhs>::value)>);
    has_value;
};

meta_fn(mul2, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value * force<rhs>::value)>);
};

meta_fn(mul , class... args);
    template<class arg, class... args>
    struct mul<arg, args...> {
        meta_return (mul2<arg, add<args...>>);
        has_value;
    };
    template<>
    struct mul<> {
        meta_return (Int<1>);
        has_value;
    };

meta_fn(div, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value / force<rhs>::value)>);
    has_value;
};

meta_fn(mod, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value % force<rhs>::value)>);
    has_value;
};

// list primitives

meta_fn(list , class... args);

template<class arg, class... args>
struct list<arg, args...> {
    meta_return (cons<arg, list<args...>>);
};

template<>
struct list<> {
    meta_return (nil);
};

meta_fn(int_list, int... n) {
    meta_return (list<Int<n>...>);
};

meta_fn(length, class Lst) {
    using lst = force<Lst>;
    let_lazy(cdr_length, length<cdr<lst>>);
    meta_return (
        cond<nilp<lst>,
            Int<0>,
            add<Int<1>, cdr_length>>);
    has_value;
};

meta_fn(nth, class Lst, int N) {
    using lst = force<Lst>;
    let_lazy(next, nth<cdr<lst>, N-1>);
    meta_return (
        cond<equal<Int<N>, Int<0>>,
            car<lst>,
            next>);
};

meta_fn(reverse_impl, class Lst, class Acc) {
    using lst = force<Lst>;
    using acc = force<Acc>;
    let_lazy(next, reverse_impl<cdr<lst>, cons<car<lst>, acc>>);
    meta_return (
        cond<nilp<lst>,
            acc,
            next>);
};

meta_fn(reverse, class lst) {
    meta_return (reverse_impl<lst, nil>);
};

meta_fn(apply_impl, template<class... args> class fn, class lst, typename = void, class... applied);
    template<
        template<class... args> class fn,
        class lst,
        class... applied>
    struct apply_impl<fn, lst, mandates<nilp<lst>>, applied...> {
        meta_return (fn<applied...>);
    };

    template<template<class... args> class fn, class lst, class... applied>
    struct apply_impl<fn, lst, mandates<not_<nilp<lst>>>, applied...> {
        meta_return (apply_impl<fn, cdr<lst>, void, applied..., car<lst>>);
    };

meta_fn(apply, template<class... args> class fn, class lst) {
    meta_return (apply_impl<fn, lst>);
};

} // namespace lmp
