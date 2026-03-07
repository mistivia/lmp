# Lispy Meta Programming

C++ template meta programming in a Lisp style.

C++17 is needed.

See `test.cc` for examples.


## Example: Sieve of Eratosthenes

```cpp
#include <lmp.h>

using namespace lmp;

meta_fn(infinite_integers, int n) {
    // `let_lazy(name, expr)` is similar to `(define name (delay expr))` in scheme
    let_lazy(next, infinite_integers<n + 1>);
    meta_return (Cons<Int<n>, next>);
};

meta_fn(filter_mod, class lst, int n) {
    let_lazy(tail, filter_mod<cdr<lst>, n>);
    meta_return (
        cond<equal<mod<car<lst>, Int<n>>, Int<0>>,
            tail,
            Cons<car<lst>, tail>>);
};

meta_fn(prime_sieve, class lst) {
    static constexpr int n = car<lst>::value;
    let_lazy(tail, prime_sieve<filter_mod<cdr<lst>, n>>);
    meta_return (Cons<Int<n>, tail>);
};

using primes = prime_sieve<infinite_integers<2>>;

static_assert(nth<primes, 0>::value == 2);
static_assert(nth<primes, 1>::value == 3);
static_assert(nth<primes, 2>::value == 5);
static_assert(nth<primes, 3>::value == 7);
static_assert(nth<primes, 4>::value == 11);
static_assert(nth<primes, 5>::value == 13);
```

Similar form in Scheme:

```scheme
(define (infinite-integers n)
   (define next (delay (infinite-integers (+ n 1))))
   (cons n next))

(define (filter-mod lst n)
    (define tail (delay (filter-mod (force (cdr lst)) n)))
    (if (= (modulo (car lst) n) 0)
        (force tail)
        (cons (car lst) tail)))

(define (prime-sieve lst)
  (define n (car lst))
  (define tail (delay (prime-sieve (filter-mod (force (cdr lst)) n))))
  (cons n tail))

(define primes
  (prime-sieve (infinite-integers 2)))
```
