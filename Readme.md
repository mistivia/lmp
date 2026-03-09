# Lispy Meta Programming

C++ template meta programming in a Lisp style.

C++11 is needed.

This is not something you would use in production. It's more like treating C++ 
template as an esolang, and create a "Lisp" on it just for fun.

See `test.cc` for examples.


## Examples

### 1. Basic List Operations

```cpp
#include <lmp.h>

using namespace lmp;

using reversed_list = reverse<IntList<1, 2, 3>>;

static_assert(length<reversed_list>::type::value == 3);

static_assert(nth<reversed_list, 0>::type::value == 3);
static_assert(nth<reversed_list, 1>::type::value == 2);
static_assert(nth<reversed_list, 2>::type::value == 1);

template<typename T>
using add1 = force<add<Int<1>, force<T>>>;

using mapped_list = map<add1, IntList<1, 2, 3>>;

static_assert(length<mapped_list>::value == 3);

static_assert(nth<mapped_list, 0>::type::value == 2);
static_assert(nth<mapped_list, 1>::type::value == 3);
static_assert(nth<mapped_list, 2>::type::value == 4);

template<typename T>
using is_even = equal<mod<T, Int<2>>, Int<0>>;

using even_numbers = filter<is_even, IntList<1, 2, 3, 4, 5, 6>>;
static_assert(equal<even_numbers, IntList<2, 4, 6>>::value);

using sum_list = IntList<1, 2, 3, 4, 5, 6, 7, 8>;
static_assert(apply<add,sum_list>::type::value == 36);
```


### 2. Infinite List of Primes (Sieve of Eratosthenes)

```cpp
#include <lmp.h>

using namespace lmp;
    
meta_fn(infinite_integers, int n) {
    // `let_lazy(name, expr)` is similar to `(define name (delay expr))` in scheme
    let_lazy(next, infinite_integers<n + 1>);
    meta_return (cons<Int<n>, next>);
};

meta_fn(prime_sieve, class lst) {
    static constexpr int n = car<lst>::value;

    template<class T>
    using not_divisible = not_<equal<mod<T, Int<n>>, Int<0>>>;
    
    let_lazy(tail, prime_sieve<filter<not_divisible, cdr<lst>>>);
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

This is similar to an infinite list of primes in Scheme:

```scheme
(define (infinite-integers n)
   (define next (delay (infinite-integers (+ n 1))))
   (cons n next))

(define (prime-sieve lst)
  (define n (car lst))
  (define (not-divisible x) (not (= (modulo x n) 0)))
  (define tail
    (delay
      (prime-sieve
        (filter not-divisible(force (cdr lst))))))
  (cons n tail))

(define primes
  (prime-sieve (infinite-integers 2)))
```

or in Haskell:

```
primes = filterPrime [2..] where
  filterPrime (p:xs) =
    p : filterPrime [x | x <- xs, x `mod` p /= 0]
```

## C++20 Concepts

if you are using C++20, in this setting, `concept` becomes "type checking".

For exmaple, we have a meta function `intp` checking if a meta value value is an integer. So we define a concept `Integer`:

```cpp
template<typename T>
concept Integer = intp<T>::value;
```

And then we can type checking our meta functions:

```cpp
meta_fn(myadd, Integer A, Integer B)
{
    using a = force<A>;
    using b = force<B>;
    meta_return (add<a, b>);
    has_value;
};

// success
using x = myadd<Int<1>, Int<2>>;

// error: template constraint failure for ‘template<class A, class B> 
// requires (Integer<A>) && (Integer<B>) struct myfn’
using y = myadd<Int<1>, nil>;
```

## How This Works

This chapter explains the core idea behind LMP: bring lazy evaluation ("thunks") into C++ template metaprogramming, so we can write branching (`if/cond`) and short-circuit logic (`and/or/case`) without resorting to heavy SFINAE boilerplate.

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

you might expect it to only instantiate/evaluate the chosen branch. But in practice, both `TrueBranch` and `FalseBranch` get instantiated while forming the types—so you can't reliably use "function call style" to implement branching.

The classic workaround is SFINAE / `enable_if` / partial specialization dispatch. It works, but it tends to add lots of boilerplate, obscure the actual logic, and make code harder to read.

### A thunk: delay evaluation by wrapping an expression

If we can wrap an expression in a type that does not evaluate it immediately, we can pass branches around safely and only evaluate the one we select.

A minimal "thunk" looks like this:

```cpp
struct thunk_name {
    using type = typename EXPR::type;
};
```


`thunk_name` itself is just a wrapper type. Only when you access `thunk_name::type` do you force evaluation of `EXPR::type`. LMP provides a macro to generate such wrappers quickly:

```cpp
#define let_lazy(__name__, ...) \
    struct __name__ { \
        using type = ::lmp::force<__VA_ARGS__>; \
    };
```

Usage:

```cpp
let_lazy(do_a_thunk, do_a<x>);
let_lazy(do_b_thunk, do_b<x>);
```

Now `do_a_thunk` / `do_b_thunk` are lazy expressions.

### `force`: evaluate a thunk (or any meta-object with `::type`)

To evaluate something in LMP, you use `force`.

- If `T` is a thunk, `force<T>` evaluates the thunk, which using the `::type` concention:


```cpp
template<typename T>
struct force_impl<T, void_t<typename T::type>> {
    using type = typename T::type;
};
```

If `T` is not a thunk, `force<T>` returns itself:

```cpp
template<typename T, typename = void>
struct force_impl {
    using type = T;
};
```

This is a convention in LMP:

1. Meta-functions return their result in `::type`
2. You call `force<>` when you want the evaluated result


### Branching with `cond_`: only evaluate the selected branch

With thunks, we can implement a real conditional branching that only evaluates one branch.

Conceptually, `cond_<Cond, TB, FB>` does:

1. `force<Cond>` to get a boolean type (`std::true_type` / `std::false_type`)
2. based on that, it forces only `TB` or `FB`

So we define both branch as a thunk:

```cpp
let_lazy(do_a_thunk, do_a<x>);
let_lazy(do_b_thunk, do_b<x>);

using result = lmp::force< lmp::cond_<ok, do_a_thunk, do_b_thunk> >;
```

This is still a bit more verbose than a real functional language, but it's predictable, composable, and avoids SFINAE-heavy branching patterns.

Short-circuit logic (`and_`, `or_`, `cond`) is built the same way. Once you have a conditional that only evaluates one branch, you can implement short-circuiting constructs naturally:

- `or_` evaluates left-to-right and stops at the first true condition
- `and_` evaluates left-to-right and stops at the first false condition
- `cond` chains `(predicate, expression)` pairs and picks the first matching one

All of them rely on the same mechanism: recursive structure + thunks + `cond_` so that the "rest of the computation" is delayed and only forced when needed.

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

You need to follow the same convention when defining new meta-functions with `using`. For example, to create a function which add one to an integer:

```cpp
template<typename T>
using add1 = force<add<Int<1>, force<T>>>;
```

