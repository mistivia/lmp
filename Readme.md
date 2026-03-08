# Lispy Meta Programming

C++ template meta programming in a Lisp style.

C++17 is needed.

This is currently a demo just for proof of concept.

See `test.cc` for examples.


## Example: Sieve of Eratosthenes

```cpp
#include <lmp.h>

using namespace lmp;
    
meta_fn(infinite_integers, int n) {
    // `let_lazy(name, expr)` is similar to `(define name (delay expr))` in scheme
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

static_assert(nth<primes, 0>::type::value == 2);
static_assert(nth<primes, 1>::type::value == 3);
static_assert(nth<primes, 2>::type::value == 5);
static_assert(nth<primes, 3>::type::value == 7);
static_assert(nth<primes, 4>::type::value == 11);
static_assert(nth<primes, 5>::type::value == 13);
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

## How This Works

This chapter explains the core idea behind LMP: bring lazy evaluation (“thunks”) into C++ template metaprogramming, so we can write branching (`if/cond`) and short-circuit logic (`and/or/case`) without resorting to heavy SFINAE boilerplate.

### Templates feel like function calls, it's eager.

In template metaprogramming, you can mentally treat

```cpp
F<A, B, C>
```

like a function call.

The problem is that template metaprogramming is effectively eager: if you write something like

```cpp
myif<Cond, TrueBranch, FalseBranch>
```

you might expect it to only instantiate/evaluate the chosen branch. But in practice, both `TrueBranch` and `FalseBranch` get instantiated while forming the types—so you can’t reliably use “function call style” to implement branching.

The classic workaround is SFINAE / `enable_if` / partial specialization dispatch. It works, but it tends to add lots of boilerplate, obscure the actual logic, and make code harder to read.

### A thunk: delay evaluation by wrapping an expression

If we can wrap an expression in a type that does not evaluate it immediately, we can pass branches around safely and only evaluate the one we select.

A minimal “thunk” looks like this:

```cpp
struct thunk_name {
    using type = typename EXPR::type;
};
```


`thunk_name` itself is just a wrapper type. Only when you access `thunk_name::type` do you force evaluation of `EXPR::type`. LMP provides a macro to generate such wrappers quickly:

```cpp
#define let_lazy(__name__, ...) \
    struct __name__ { \
        using type = typename __VA_ARGS__::type; \
    };
```

Usage:

```cpp
let_lazy(do_a_thunk, do_a<x>);
let_lazy(do_b_thunk, do_b<x>);
```

Now `do_a_thunk` / `do_b_thunk` are lazy expressions.

### `force`: evaluate a thunk (or any meta-object with `::type`)

To evaluate something in LMP, you use `force`:

```cpp
template<typename T>
using force = typename T::type;
```

- If `T` is a thunk, `force<T>` evaluates the thunk.
- If `T` is any meta-object that follows the convention "result is in `::type`", `force<T>` retrieves that result.

This is a convention in LMP:

1. Meta-functions return their result in `::type`
2. You call `force<>` when you want the evaluated result


### Branching with `cond`: only evaluate the selected branch

With thunks, we can implement a real conditional branching that only evaluates one branch.

Conceptually, `cond<Cond, TB, FB>` does:

1. `force<Cond>` to get a boolean type (`std::true_type` / `std::false_type`)
2. based on that, it forces only `TB` or `FB`

So we define both branch as a thunk:

```cpp
let_lazy(do_a_thunk, do_a<x>);
let_lazy(do_b_thunk, do_b<x>);

using result = lmp::force< lmp::cond<ok, do_a_thunk, do_b_thunk> >;
```

This is still a bit more verbose than a real functional language, but it’s predictable, composable, and avoids SFINAE-heavy branching patterns.

Short-circuit logic (`and_`, `or_`, `case_`) is built the same way. Once you have a conditional that only evaluates one branch, you can implement short-circuiting constructs naturally:

- `or_` evaluates left-to-right and stops at the first true condition
- `and_` evaluates left-to-right and stops at the first false condition
- `case_` chains `(predicate, expression)` pairs and picks the first matching one

All of them rely on the same mechanism: recursive structure + thunks + `cond` so that the “rest of the computation” is delayed and only forced when needed.

### The meta-function pattern: force inputs, and force the returned expression

In LMP, function arguments might themselves be thunks. A common pattern is:

- `force` arguments at the start (because arguments could be thunks),
- evaluate the expression,
- `force` the result (because the expression may return a thunk).

For example:

```cpp
template<typename Arg>
struct my_meta_func {
    using arg = lmp::force<Arg>;
    using type = lmp::force< do_something<arg> >;
};
```

To make this style uniform and lightweight, LMP uses two macros:

```cpp
#define meta_fn(__name__, ...) template<__VA_ARGS__> struct __name__
#define meta_return(...) using type = ::lmp::force<__VA_ARGS__>
```

So the same function becomes:

```cpp
meta_fn(my_meta_func, class Arg) {
    using arg = lmp::force<Arg>;
    meta_return(do_something<arg>);
};
```

### Data types also follow the `::type` convention

LMP has “meta-functions”, but it also has "meta-data structures" (e.g., Lisp-like lists). For data, a crucial rule is:

> A data object's `::type` should be itself, so `force<Data>` yields the same structure.

For example:

```cpp
struct nil { using type = nil; };

template<typename head, typename tail>
struct cons {
    using car = head;
    using cdr = tail;
    using type = cons<head, tail>;
};
```

This lets you write accessors that work whether the input is a thunk or already-evaluated data:

```cpp
template<typename lst>
using car = typename lmp::force<lst>::car;

template<typename lst>
using cdr = typename lmp::force<lst>::cdr;
```
