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


### 2. A (toy) query DSL

```cpp
// exmaple of a query EDSL

struct field1 {
    static constexpr char name[] = "age";
    using type = int;
};

struct field2 {
    static constexpr char name[] = "height";
    using type = double;
};

struct field3 {
    static constexpr char name[] = "name";
    using type = char*;
};

struct mytable {
    static constexpr char name[] = "mytable";
    using fields = list<field1, field2>;
};

template<typename query>
using is_valid_query = memberp<typename query::field, typename query::from::fields>;

constexpr char from_lit[] = "FROM";
constexpr char select_lit[] = "SELECT";
constexpr char space_lit[] = " ";
constexpr char semicolon_lit[] = ";";

META_FN(build_query, typename query) {
    META_RETURN (list2string<concat<
        string2list<select_lit>,
        string2list<space_lit>,
        string2list<query::field::name>,
        string2list<space_lit>,
        string2list<from_lit>,
        string2list<space_lit>,
        string2list<query::from::name>,
        string2list<semicolon_lit>>>);
};

// usage:

// checking validity of a query
struct myquery {
    using from = mytable;
    using field = field2;
};

static_assert(is_valid_query<myquery>::value);

struct invalid_query {
    using from = mytable;
    using field = field3;
};

// error:
// static_assert(is_valid_query<invalid_query>::value);

int main() {
    printf("%s\n", build_query<myquery>::type::value);
    return 0;
}

// result:
// SELECT height FROM mytable;
```

### 3. (C++17) Compile-time reflex

```cpp
#include <string>

#include "lmp.h"

template<typename... args>
struct ReflexObjectImpl;

template<typename N, typename T, typename... args>
struct ReflexObjectImpl<N, T, args...> : ReflexObjectImpl<args...> {
    T value;
    using name = N;
};

template<>
struct ReflexObjectImpl<> {};

#define DEFINE_LITERAL(_name_)  \
    namespace { \
        inline static constexpr char _name_##_lit[] = #_name_; \
        using _name_ = typename lmp::string2list<_name_##_lit>::type; \
    } \

template<typename N, typename T, typename ...args>
T& getImpl(ReflexObjectImpl<args...> &obj) {
    using lst = lmp::list<args...>;
    static_assert(lmp::length<lst>::value >= 2);
    if constexpr (lmp::eq<lmp::car<lst>, N>::value) {
        return obj.value;
    } else {
        using BaseType = typename lmp::apply<ReflexObjectImpl, lmp::cddr<lst>>::type;
        return getImpl<N, T>(static_cast<BaseType&>(obj));
    }
}

template<typename N, typename T, typename ...args>
void setImpl(ReflexObjectImpl<args...> &obj, T&& new_value) {
    using lst = lmp::list<args...>;
    static_assert(lmp::length<lst>::value >= 2);
    if constexpr (lmp::eq<lmp::car<lst>, N>::value) {
        obj.value = std::forward<T&&>(new_value);
    } else {
        using BaseType = typename lmp::apply<ReflexObjectImpl, lmp::cddr<lst>>::type;
        return setImpl<N, T>(static_cast<BaseType&>(obj), std::forward<T&&>(new_value));
    }
}

template<typename... args>
void printImpl(ReflexObjectImpl<args...> &obj) {
    using lst = typename lmp::list<args...>::type;
    if constexpr (lmp::length<lst>::value >= 2) {
        using N = typename lmp::nth<lst, 0>::type;
        using T = typename lmp::nth<lst, 1>::type;
        if constexpr (lmp::eq<T, int>::value) {
            printf("%s: %d\n", lmp::list2string<N>::type::value, obj.value);
        } else if constexpr (lmp::eq<T, double>::value) {
            printf("%s: %lf\n", lmp::list2string<N>::type::value, obj.value);
        } else if constexpr (lmp::eq<T, std::string>::value) {
            printf("%s: %s\n", lmp::list2string<N>::type::value, obj.value.c_str());
        }
        using BaseType = typename lmp::apply<ReflexObjectImpl, lmp::cddr<lst>>::type;
        printImpl(static_cast<BaseType&>(obj));
    } else {
        return;
    }
}

template<typename... args>
struct ReflexObject {
    ReflexObjectImpl<args...> value;
    template<typename N>
    using value_type = typename lmp::next_of<N, lmp::list<args...>>::type;
    template<typename N>
    value_type<N> get() {
        return getImpl<N, value_type<N>, args...>(value);
    }
    void print() {
        printImpl(value);
    }
    template<typename N>
    void set(value_type<N> &&new_value) {
        setImpl<N, value_type<N>, args...>(value, std::forward<value_type<N>&&>(new_value));
    }
};

DEFINE_LITERAL(age)
DEFINE_LITERAL(height)
DEFINE_LITERAL(name)

using mytype = ReflexObject<
    age, int,
    height, double,
    name, std::string
>;

int main (){
    mytype x;
    x.set<age>(1);
    x.set<height>(3.1415926);
    x.set<name>("hello");
    x.print();
    return 0;
}
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
META_FN(myadd, Integer A, Integer B)
{
    using a = force<A>;
    using b = force<B>;
    META_RETURN (add<a, b>);
    HAS_VALUE;
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
#define LET_LAZY(__name__, ...) \
    struct __name__ { \
        using type = ::lmp::force<__VA_ARGS__>; \
    };
```

Usage:

```cpp
LET_LAZY(do_a_thunk, do_a<x>);
LET_LAZY(do_b_thunk, do_b<x>);
```

Now `do_a_thunk` / `do_b_thunk` are lazy expressions.

### `force`: evaluate a thunk (or any meta-object with `::type`)

To evaluate something in LMP, you use `force`.

- If `T` is a thunk, `force<T>` evaluates the thunk, which using the
  `::type` convention:


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


### Branching with `if_`: only evaluate the selected branch

With thunks, we can implement a real conditional branching that only evaluates one branch.

Conceptually, `if_<Cond, TB, FB>` does:

1. `force<Cond>` to get a boolean type (`std::true_type` / `std::false_type`)
2. based on that, it forces one of `TB` or `FB`

So we define both branch as a thunk:

```cpp
LET_LAZY(do_a_thunk, do_a<x>);
LET_LAZY(do_b_thunk, do_b<x>);

using result = lmp::force< lmp::if_<ok, do_a_thunk, do_b_thunk> >;
```

This is still a bit more verbose than a real functional language, but
it's predictable, composable, and avoids SFINAE-heavy branching
patterns.

All other syntax involving hort-circuit logic (`and_`, `or_`, `cond`)
is built the same way. Once you have a conditional that only evaluates
one branch, you can implement short-circuiting constructs naturally:

- `or_` evaluates left-to-right and stops at the first true condition
- `and_` evaluates left-to-right and stops at the first false condition
- `cond` chains `(predicate, expression)` pairs and picks the first matching one

All of them rely on the same mechanism: recursive structure + thunks +
`cond_` so that the "rest of the computation" is delayed and only
forced when needed.

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
#define META_FN(__name__, ...) template<__VA_ARGS__> struct __name__
#define META_RETURN(...) using type = ::lmp::force<__VA_ARGS__>
```

So the same function becomes:

```cpp
META_FN(my_meta_func, class Arg) {
    using arg = lmp::force<Arg>;
    META_RETURN(do_something<arg>);
};
```

You need to follow the same convention when defining new
meta-functions with `using`. For example, to create a function which
add one to an integer:

```cpp
template<typename T>
using add1 = force<add<Int<1>, force<T>>>;
```

