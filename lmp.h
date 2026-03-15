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

#define LET_LAZY(__name__, ...) \
    struct __name__ { \
        using type = ::lmp::force<__VA_ARGS__>; \
    };

#define META_FN(__name__, ...) template<__VA_ARGS__> struct __name__

#define META_RETURN(...) using type = ::lmp::force<__VA_ARGS__>

#define HAS_VALUE static constexpr auto value = type::value

// data constructor

template<typename... args>
struct list { using type = list<args...>; };

using nil = list<>;

template<typename head, typename tail>
struct cons_impl;

template<typename head, typename tail>
struct cons_impl {
    using type = typename cons_impl<head, force<tail>>::type;
};

template<typename head, typename... args>
struct cons_impl<head, list<args...>> {
    using type = list<head, args...>;
};

template<typename head, typename tail>
using cons = typename cons_impl<head, tail>::type;

// data accessor

template<typename lst>
struct car_impl;

template<typename head, typename... tail>
struct car_impl<list<head, tail...>> {
    using type = head;
};

template<typename lst>
using car = force<typename car_impl<force<lst>>::type>;

template<typename lst>
struct cdr_impl;

template<typename head, typename... tail>
struct cdr_impl<list<head, tail...>> {
    using type = list<tail...>;
};

template<typename lst>
using cdr = force<typename cdr_impl<force<lst>>::type>;

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

template<typename Head, typename... Tail>
struct pairp_impl<list<Head, Tail...>> : std::true_type { };

template<typename T>
using pairp = pairp_impl<force<T>>;


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

META_FN(if_, class Cond, class tb, class fb) {
    using condition = force<Cond>;
    META_RETURN (if_impl<condition, tb, fb>);
};

template<typename ...arg>
using if_impl_ = typename if_<arg...>::type;

// cond<pred1, expr1, pred2, expr2, ..., default_expr>
META_FN(cond_, class... Args);
    template<class default_expr>
    struct cond_<default_expr> {
        META_RETURN (default_expr);
    };
    template<class pred, class expr, class... rest>
    struct cond_<pred, expr, rest...> {
        LET_LAZY(next, cond_<rest...>);
        META_RETURN (if_impl_<pred, expr, next>);
    };

template<class... args>
using cond = typename cond_<args...>::type;

META_FN(listp, class T) {
    using t = force<T>;
    LET_LAZY(rest, listp<cdr<t>>);
    META_RETURN (
        cond<nilp<t>, std::true_type,
             pairp<t>, rest,
             std::false_type>);
    HAS_VALUE;
};

// logical primitive (with short-circuit)

META_FN(and_, class... Bs);
    template<>
    struct and_<> {
        META_RETURN (std::true_type);
        HAS_VALUE;
    };
    template<class B, class... Bs>
    struct and_<B, Bs...> {
        LET_LAZY(rest, and_<Bs...>);
        META_RETURN (if_<B, rest, std::false_type>);
        HAS_VALUE;
    };

META_FN(or_, class... Bs);
    template<>
    struct or_<> {
        META_RETURN (std::false_type);
        HAS_VALUE;
    };
    template<class B, class... Bs>
    struct or_<B, Bs...> {
        LET_LAZY(rest, or_<Bs...>);
        META_RETURN (if_<B, std::true_type, rest>);
        HAS_VALUE;
    };


// basic arithmetics

META_FN(equal, class L, class R) {
    using l = force<L>;
    using r = force<R>;
    LET_LAZY(pair_eq, and_<equal<car<l>, car<r>>, equal<cdr<l>, cdr<r>>>);
    META_RETURN (
        cond<and_<pairp<l>, pairp<r>>,
              pair_eq,
              eq<l, r>>);
    HAS_VALUE;
};

template<typename L, typename R>
using gt = bool_constant<(force<L>::value > force<R>::value)>;

template<typename L, typename R>
using ge = bool_constant<force<L>::value >= force<R>::value>;

template<typename L, typename R>
using lt = bool_constant<(force<L>::value < force<R>::value)>;

template<typename L, typename R>
using le = bool_constant<(force<L>::value <= force<R>::value)>;

META_FN(add2, class lhs, class rhs) {
    META_RETURN (Int<(force<lhs>::value + force<rhs>::value)>);
};

template<typename T>
using mandates = typename std::enable_if<T::value>::type;

META_FN(foldl, template<class, class> class fn, class init, class... args);
    template<template<class, class> class fn, class init>
    struct foldl<fn, init> {
        META_RETURN (init);
    };
    template<template<class, class> class fn, class init, class arg, class... args>
    struct foldl<fn, init, arg, args...> {
        META_RETURN (foldl<fn, fn<init, arg>, args...>);
    };

META_FN(add, class... args) {
    META_RETURN (foldl<add2, Int<0>, args...>);
    HAS_VALUE;
};

META_FN(neg, class rhs) {
    META_RETURN (Int<(- force<rhs>::value)>);
    HAS_VALUE;
};

META_FN(sub, class lhs, class rhs) {
    META_RETURN (Int<(force<lhs>::value - force<rhs>::value)>);
    HAS_VALUE;
};

META_FN(mul2, class lhs, class rhs) {
    META_RETURN (Int<(force<lhs>::value * force<rhs>::value)>);
};

META_FN(mul, class... args) {
    META_RETURN (foldl<mul2, Int<1>, args...>);
    HAS_VALUE;
};

META_FN(div, class lhs, class rhs) {
    META_RETURN (Int<(force<lhs>::value / force<rhs>::value)>);
    HAS_VALUE;
};

META_FN(mod, class lhs, class rhs) {
    META_RETURN (Int<(force<lhs>::value % force<rhs>::value)>);
    HAS_VALUE;
};

// list primitives

META_FN(IntList, int... n) {
    META_RETURN (list<Int<n>...>);
};

META_FN(length, class Lst) {
    using lst = force<Lst>;
    LET_LAZY(cdr_length, length<cdr<lst>>);
    META_RETURN (
        cond<nilp<lst>,
            Int<0>,
            add<Int<1>, cdr_length>>);
    HAS_VALUE;
};

META_FN(nth, class Lst, int N) {
    using lst = force<Lst>;
    LET_LAZY(next, nth<cdr<lst>, N-1>);
    META_RETURN (
        cond<equal<Int<N>, Int<0>>,
            car<lst>,
            next>);
};

META_FN(reverse_impl, class Lst, class Acc) {
    using lst = force<Lst>;
    using acc = force<Acc>;
    LET_LAZY(next, reverse_impl<cdr<lst>, cons<car<lst>, acc>>);
    META_RETURN (
        cond<nilp<lst>,
            acc,
            next>);
};

META_FN(reverse, class lst) {
    META_RETURN (reverse_impl<lst, nil>);
};

META_FN(concat_, class L1, class L2) {
    using rev1 = reverse<force<L1>>;
    using rev2 = reverse_impl<force<L2>, rev1>;
    META_RETURN (reverse<rev2>);
};

META_FN(concat, class... args) {
    META_RETURN (foldl<concat_, nil, args...>);
};

META_FN(append, class Lst, class Elem) {
    using lst = force<Lst>;
    using elem = force<Elem>;
    META_RETURN (
        reverse<cons<elem, reverse<lst>>>);
};

META_FN(range, int start, int end) {
    META_FN(range_impl, int cur, class lst) {
        META_RETURN (
            cond<lt<Int<cur>, Int<start>>,
                lst,
                range_impl<(cur-1), cons<Int<cur>, lst>>>);
    };
    META_RETURN (
        cond<le<Int<(end-start)>, Int<0>>,
            nil,
            range_impl<(end-1), nil>>);
};

META_FN(memberp, class Item, class Lst) {
    using lst = force<Lst>;
    using item = force<Item>;
    LET_LAZY(try_rest, memberp<item, cdr<lst>>);
    LET_LAZY(check_list,
        cond<eq<item, car<lst>>,
            std::true_type,
            try_rest>);
    META_RETURN (
        cond<nilp<lst>,
            std::false_type,
            check_list>);
    HAS_VALUE;
};

META_FN(next_of, class T, class Lst) {
    using t = force<T>;
    using lst = force<Lst>;
    LET_LAZY(try_rest, next_of<t, cdr<lst>>);
    LET_LAZY(next_item, car<cdr<lst>>);
    LET_LAZY(check_list,
        cond<and_<equal<t, car<lst>>, not_<nilp<cdr<lst>>>>,
            next_item,
            try_rest>);
    META_RETURN (
        cond<nilp<lst>,
            nil,
            check_list>);
};

META_FN(map, template<class> class fn, class Lst) {
    using lst = force<Lst>;
    LET_LAZY(new_lst, cons<fn<car<lst>>, map<fn, cdr<lst>>>);
    META_RETURN (
        cond<nilp<lst>,
            nil,
            new_lst>);
};

META_FN(filter, template<class> class pred, class Lst) {
    using lst = force<Lst>;
    LET_LAZY(filtered_tail, filter<pred, cdr<lst>>);
    LET_LAZY(new_lst,
        cond<pred<car<lst>>,
            cons<car<lst>, filtered_tail>,
            filtered_tail>);
    META_RETURN (
        cond<nilp<lst>,
            nil,
            new_lst>);
};

META_FN(apply_impl, template<class... args> class fn, class lst, typename = void, class... applied);
    template<
        template<class... args> class fn,
        class lst,
        class... applied>
    struct apply_impl<fn, lst, mandates<nilp<lst>>, applied...> {
        META_RETURN (fn<applied...>);
    };

    template<template<class... args> class fn, class lst, class... applied>
    struct apply_impl<fn, lst, mandates<not_<nilp<lst>>>, applied...> {
        META_RETURN (apply_impl<fn, cdr<lst>, void, applied..., car<lst>>);
    };

META_FN(apply, template<class... args> class fn, class lst) {
    META_RETURN (apply_impl<fn, lst>);
};

// string utils

META_FN(char_at, const char* str, int n) {
    META_RETURN (Int<(int)str[n]>);
};

constexpr int string_len(const char* s) {
    return *s ? 1 + string_len(s + 1) : 0;
}

META_FN(string2list, const char *str) {
    using idx_lst= range<0, string_len(str)>;
    template<class Elem>
    using at = char_at<str, force<Elem>::value>;
    META_RETURN (map<at, idx_lst>);
};


template<char... Cs>
struct string_constant {
    static constexpr char value[sizeof...(Cs) + 1] = { Cs..., '\0' };
};

META_FN(list2string, class Lst, char... Cs) {
    using lst = force<Lst>;
    LET_LAZY(next, list2string<cdr<lst>, Cs..., (char)car<lst>::value>);
    META_RETURN (
        cond<nilp<lst>,
            string_constant<Cs...>,
            next>);
};

} // namespace lmp
