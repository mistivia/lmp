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

#include "lmp.h"

using namespace lmp;

// infinite list of primes

meta_fn(infinite_integers, int n) {
    let_lazy(next, infinite_integers<n + 1>);
    meta_return (cons<Int<n>, next>);
};

meta_fn(filter_mod, class lst, int n) {
    let_lazy(tail, filter_mod<cdr<lst>, n>);
    meta_return (
        cond<equal<mod<car<lst>, Int<n>>, Int<0>>,
            tail,
            cons<car<lst>, tail>>);
};

meta_fn(prime_sieve, class lst) {
    static constexpr int n = car<lst>::value;
    let_lazy(tail, prime_sieve<filter_mod<cdr<lst>, n>>);
    meta_return (cons<Int<n>, tail>);
};

using primes = prime_sieve<infinite_integers<2>>;

// tests

static_assert(eq<apply<list, IntList<1, 2, 3>>, IntList<1, 2, 3>>::value);

using reversed_list = reverse<IntList<1, 2, 3>>;
static_assert(length<reversed_list>::type::value == 3);
static_assert(nth<reversed_list, 0>::type::value == 3);
static_assert(nth<reversed_list, 1>::type::value == 2);
static_assert(nth<reversed_list, 2>::type::value == 1);

static_assert(eq<Int<7>, add<Int<3>, Int<4>>>::value);
static_assert(!eq<Int<7>, add<Int<3>, Int<5>>>::value);
static_assert(equal<Int<7>, add<Int<3>, Int<4>>>::value);
static_assert(!equal<Int<8>, add<Int<3>, Int<4>>>::value);

static_assert(not_<std::false_type>::value);
static_assert(!not_<std::true_type>::value);

static_assert(and_<>::value);
static_assert(and_<std::true_type, std::true_type>::value);
static_assert(!and_<std::true_type, std::false_type>::value);

static_assert(!or_<>::value);
static_assert(or_<std::false_type, std::true_type>::value);
static_assert(!or_<std::false_type, std::false_type>::value);

// short-circuit checks: second argument must not be instantiated
struct should_not_be_forced;
static_assert(!and_<std::false_type, should_not_be_forced>::value);
static_assert(or_<std::true_type, should_not_be_forced>::value);

// case_ checks
static_assert(
    case_<std::true_type, Int<1>,
          Int<2>>
    ::type::value == 1);
static_assert(
    case_<std::false_type, Int<1>,
          std::true_type, Int<2>,
          Int<3>>
    ::type::value == 2);
static_assert(
    case_<std::false_type, Int<1>,
          std::false_type, Int<2>,
          Int<3>>
    ::type::value == 3);

// case_ short-circuit: later predicate must not be instantiated if already matched
static_assert(
    case_<std::true_type, std::true_type,
          should_not_be_forced, std::false_type,
          std::false_type>\
    ::type::value);

inline constexpr char hello_str[] = "hello";
static_assert(char_at<hello_str, 0>::type::value == 'h');
static_assert(char_at<hello_str, 1>::type::value == 'e');
static_assert(char_at<hello_str, 4>::type::value == 'o');
static_assert(char_at<hello_str, 5>::type::value == '\0');

using hello_str_lst = string_to_list<hello_str>;

static_assert(nth<hello_str_lst, 0>::type::value == (int)'h');
static_assert(nth<hello_str_lst, 1>::type::value == (int)'e');
static_assert(nth<hello_str_lst, 4>::type::value == (int)'o');

using my_list = IntList<1,2,3>;
static_assert(car<my_list>::value == 1);
static_assert(cadr<my_list>::value == 2);
static_assert(length<my_list>::value == 3);

using appended_list = append<my_list, Int<4>>;
static_assert(length<appended_list>::value == 4);
static_assert(nth<appended_list, 0>::type::value == 1);
static_assert(nth<appended_list, 1>::type::value == 2);
static_assert(nth<appended_list, 2>::type::value == 3);
static_assert(nth<appended_list, 3>::type::value == 4);

using range_list = range<5, 10>;
static_assert(length<range_list>::value == 5);
static_assert(nth<range_list, 0>::type::value == 5);
static_assert(nth<range_list, 1>::type::value == 6);
static_assert(nth<range_list, 2>::type::value == 7);
static_assert(nth<range_list, 3>::type::value == 8);
static_assert(nth<range_list, 4>::type::value == 9);

static_assert(memberp<Int<1>, my_list>::value);
static_assert(memberp<Int<2>, my_list>::value);
static_assert(!memberp<Int<4>, my_list>::value);

using mapped_neg_list = map<neg, my_list>;
static_assert(length<mapped_neg_list>::value == 3);
static_assert(nth<mapped_neg_list, 0>::type::value == -1);
static_assert(nth<mapped_neg_list, 1>::type::value == -2);
static_assert(nth<mapped_neg_list, 2>::type::value == -3);

template<typename T>
using add1 = force<add<Int<1>, force<T>>>;

using mapped_add1_list = map<add1, my_list>;
static_assert(length<mapped_add1_list>::value == 3);
static_assert(nth<mapped_add1_list, 0>::type::value == 2);
static_assert(nth<mapped_add1_list, 1>::type::value == 3);
static_assert(nth<mapped_add1_list, 2>::type::value == 4);

static_assert(nth<primes, 0>::type::value == 2);
static_assert(nth<primes, 1>::type::value == 3);
static_assert(nth<primes, 2>::type::value == 5);
static_assert(nth<primes, 3>::type::value == 7);
static_assert(nth<primes, 4>::type::value == 11);
static_assert(nth<primes, 5>::type::value == 13);

static_assert(add<Int<1>, Int<2>, Int<3>>::value == 6);
static_assert(apply<add, IntList<1, 2, 3>>::type::value == 6);
static_assert(apply<add, IntList<1, 2, 3, 4, 5, 6, 7, 8>>::type::value == 36);

int main() {
    return 0;
}  
