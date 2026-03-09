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

template <typename...>
using void_t = void;

template<bool B>
using bool_constant = std::integral_constant<bool, B>;

template<typename T, typename = void>
struct force_impl {
    using type = T;
};

template<typename T>
struct force_impl<T, void_t<typename T::type>> {
    using type = typename T::type;
};


template<typename T>
using force = typename force_impl<T>::type;

// macros

#define let_lazy(__name__, ...) \
    struct __name__ { \
        using type = ::lmp::force<__VA_ARGS__>; \
    };

#define meta_fn(__name__, ...) template<__VA_ARGS__> struct __name__

#define meta_return(...) using type = ::lmp::force<__VA_ARGS__>

#define has_value static constexpr auto value = type::value

// data constructor

struct nil { };

template<typename head, typename tail>
struct cons {
    using car = head;
    using cdr = tail;
};

// data accessor

template<typename lst>
using car = force<typename force<lst>::car>;

template<typename lst>
using cdr = force<typename force<lst>::cdr>;

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
using nilp = eq<force<T>, nil>;

template<typename T>
struct intp_impl : std::false_type { };

template<int N>
struct intp_impl<Int<N>> : std::true_type { };

template<typename T>
using intp = intp_impl<force<T>>;

template<typename T>
struct pairp_impl : std::false_type { };

template<typename Head, typename Tail>
struct pairp_impl<cons<Head, Tail>> : std::true_type { };

template<typename T>
using pairp = pairp_impl<force<T>>;

// forward declaration (used by listp)
meta_fn(cond, class... Args);

meta_fn(listp, class T) {
    using t = force<T>;
    let_lazy(rest, listp<cdr<t>>);
    meta_return (
        cond<nilp<t>, std::true_type,
             pairp<t>, rest,
             std::false_type>);
    has_value;
};

template<typename B>
using not_ = bool_constant<!force<B>::value>;

// cond

template<typename condition, typename tb, typename fb>
struct if_impl;

template<typename tb, typename fb>
struct if_impl<std::true_type, tb, fb> {
  using type = force<tb>;
};

template<typename tb, typename fb>
struct if_impl<std::false_type, tb, fb> {
  using type = force<fb>;
};

meta_fn(if_, class Cond, class tb, class fb) {
    using condition = force<Cond>;
    meta_return (if_impl<condition, tb, fb>);
};

// cond<pred1, expr1, pred2, expr2, ..., default_expr>
meta_fn(cond, class... Args);
    template<class default_expr>
    struct cond<default_expr> {
        meta_return (default_expr);
    };
    template<class pred, class expr, class... rest>
    struct cond<pred, expr, rest...> {
        let_lazy(next, cond<rest...>);
        meta_return (if_<pred, expr, next>);
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
        meta_return (if_<B, rest, std::false_type>);
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
        meta_return (if_<B, std::true_type, rest>);
        has_value;
    };


// basic arithmetics

meta_fn(equal, class L, class R) {
    using l = force<L>;
    using r = force<R>;
    let_lazy(pair_eq, and_<equal<car<l>, car<r>>, equal<cdr<l>, cdr<r>>>);
    meta_return (
        cond<and_<pairp<l>, pairp<r>>,
              pair_eq,
              eq<l, r>>);
    has_value;
};

template<typename L, typename R>
using gt = bool_constant<(force<L>::value > force<R>::value)>;

template<typename L, typename R>
using ge = bool_constant<force<L>::value >= force<R>::value>;

template<typename L, typename R>
using lt = bool_constant<(force<L>::value < force<R>::value)>;

template<typename L, typename R>
using le = bool_constant<(force<L>::value <= force<R>::value)>;

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

meta_fn(IntList, int... n) {
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

meta_fn(append, class Lst, class Elem) {
    using lst = force<Lst>;
    using elem = force<Elem>;
    meta_return (
        reverse<cons<elem, reverse<lst>>>);
};

meta_fn(range, int start, int end) {
    meta_fn(range_impl, int cur, class lst) {
        meta_return (
            cond<lt<Int<cur>, Int<start>>,
                lst,
                range_impl<(cur-1), cons<Int<cur>, lst>>>);
    };
    meta_return (
        cond<le<Int<(end-start)>, Int<0>>,
            nil,
            range_impl<(end-1), nil>>);
};

meta_fn(memberp, class Item, class Lst) {
    using lst = force<Lst>;
    using item = force<Item>;
    let_lazy(try_rest, memberp<item, cdr<lst>>);
    let_lazy(check_list,
        cond<eq<item, car<lst>>,
            std::true_type,
            try_rest>);
    meta_return (
        cond<nilp<lst>,
            std::false_type,
            check_list>);
    has_value;
};

meta_fn(map, template<class> class fn, class Lst) {
    using lst = force<Lst>;
    let_lazy(new_lst, cons<fn<car<lst>>, map<fn, cdr<lst>>>);
    meta_return (
        cond<nilp<lst>,
            nil,
            new_lst>);
};

meta_fn(filter, template<class> class pred, class Lst) {
    using lst = force<Lst>;
    let_lazy(filtered_tail, filter<pred, cdr<lst>>);
    let_lazy(new_lst,
        cond<pred<car<lst>>,
            cons<car<lst>, filtered_tail>,
            filtered_tail>);
    meta_return (
        cond<nilp<lst>,
            nil,
            new_lst>);
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

// string utils

meta_fn(char_at, const char* str, int n) {
    meta_return (Int<(int)str[n]>);
};

constexpr int string_len(const char* s) {
    return *s ? 1 + string_len(s + 1) : 0;
}

meta_fn(string_to_list, const char *str) {
    using idx_lst= range<0, string_len(str)>;
    template<class Elem>
    using at = char_at<str, force<Elem>::value>;
    meta_return (map<at, idx_lst>);
};

} // namespace lmp
