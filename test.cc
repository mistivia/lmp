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

struct primes {
    meta_return (prime_sieve<infinite_integers<2>>);
};

// tests

static_assert(eq<apply<list, Intlist<1, 2, 3>>, Intlist<1, 2, 3>>::value);

using reversed_list = reverse<Intlist<1, 2, 3>>;
static_assert(length<reversed_list>::type::value == 3);
static_assert(nth<reversed_list, 0>::type::value == 3);
static_assert(nth<reversed_list, 1>::type::value == 2);
static_assert(nth<reversed_list, 2>::type::value == 1);

static_assert(not_<std::false_type>::value);
static_assert(!not_<std::true_type>::value);

using my_list = Intlist<1,2,3>;
static_assert(car<my_list>::value == 1);
static_assert(cadr<my_list>::value == 2);
static_assert(length<my_list>::type::value == 3);

static_assert(nth<primes, 0>::type::value == 2);
static_assert(nth<primes, 1>::type::value == 3);
static_assert(nth<primes, 2>::type::value == 5);
static_assert(nth<primes, 3>::type::value == 7);
static_assert(nth<primes, 4>::type::value == 11);
static_assert(nth<primes, 5>::type::value == 13);

static_assert(add<Int<1>, Int<2>, Int<3>>::type::value == 6);
static_assert(apply<add, Intlist<1, 2, 3>>::type::value == 6);
static_assert(apply<add, Intlist<1, 2, 3, 4, 5, 6, 7, 8>>::type::value == 36);

int main() {
    return 0;
}  
