#include <type_traits>
#include <cstdio>

namespace lmp { // "lmp' is short for "Lispy Meta Programming"

template<typename T>
using force = typename T::type;

// macros

#define let_lazy(__name__, ...) \
    struct __name__ { \
        using type = typename __VA_ARGS__::type; \
    };

#define meta_fn(__name__, ...) template<__VA_ARGS__> struct __name__

#define meta_return(...) using type = ::lmp::force<__VA_ARGS__>

#define has_value static constexpr int value = type::value

// data Constructor

struct Nil { 
    using type = Nil;
};

template<typename head, typename tail>
struct Cons {
    using car = head;
    using cdr = tail;
    using type = Cons<head, tail>;
};

// data accessor

template<typename lst>
using car = typename force<lst>::car;

template<typename lst>
using cdr = typename force<lst>::cdr;

template<typename lst>
using cadr = car<cdr<lst>>;

// wrapper for integer

template<int N>
using Int = std::integral_constant<int, N>;

// type function for comparing

template<typename L, typename R>
using eq = std::is_same<L, R>;

template<typename T>
using nilp = eq<T, Nil>;

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
using requires = typename std::enable_if<T::value>::type;

meta_fn(add , class... args);
    template<class arg, class... args>
    struct add<arg, args...> {
        meta_return (add2<arg, add<args...>>);
    };
    template<>
    struct add<> {
        meta_return (Int<0>);
    };

meta_fn(neg, class rhs) {
    meta_return (Int<(- force<rhs>::value)>);
};

meta_fn(sub, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value - force<rhs>::value)>);
};

meta_fn(mul2, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value * force<rhs>::value)>);
};

meta_fn(mul , class... args);
    template<class arg, class... args>
    struct mul<arg, args...> {
        meta_return (mul2<arg, add<args...>>);
    };
    template<>
    struct mul<> {
        meta_return (Int<1>);
    };

meta_fn(div, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value / force<rhs>::value)>);
};

meta_fn(mod, class lhs, class rhs) {
    meta_return (Int<(force<lhs>::value % force<rhs>::value)>);
};

// list primitives

meta_fn(List , class... args);

template<class arg, class... args>
struct List<arg, args...> {
    meta_return (Cons<arg, List<args...>>);
};

template<>
struct List<> {
    meta_return (Nil);
};

meta_fn(IntList, int... n) {
    meta_return (List<Int<n>...>);
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
    has_value;
};

meta_fn(apply, template<class... args> class fn, class lst, typename = void);
    template<
        template<class... args> class fn,
        class lst>
    struct apply<fn, lst, requires<equal<Int<1>, length<lst>>>> {
        meta_return (fn<nth<lst, 0>>);
    };
    template<
        template<class... args> class fn,
        class lst>
    struct apply<fn, lst, requires<equal<Int<2>, length<lst>>>> {
        meta_return (fn<nth<lst, 0>, nth<lst, 1>>);
    };
    template<
        template<class... args> class fn,
        class lst>
    struct apply<fn, lst, requires<equal<Int<3>, length<lst>>>> {
        meta_return (fn<nth<lst, 0>, nth<lst, 1>, nth<lst, 2>>);
    };
    template<
        template<class... args> class fn,
        class lst>
    struct apply<fn, lst, requires<equal<Int<4>, length<lst>>>> {
        meta_return (fn<nth<lst, 0>, nth<lst, 1>, nth<lst, 2>, nth<lst, 3>>);
    };
    template<
        template<class... args> class fn,
        class lst>
    struct apply<fn, lst, requires<equal<Int<5>, length<lst>>>> {
        meta_return (fn<nth<lst, 0>, nth<lst, 1>, nth<lst, 2>,
                        nth<lst, 3>, nth<lst, 4>>);
    };

} // namespace lmp

using namespace lmp;

// infinite list of primes

meta_fn(infinite_integers, int n) {
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

struct primes {
    meta_return (prime_sieve<infinite_integers<2>>);
};

// tests

using my_list = IntList<1,2,3>;

int main() {
    std::printf("first: %d\n", car<my_list>::value);
    std::printf("second: %d\n", cadr<my_list>::value);
    std::printf("length: %d\n", length<my_list>::value);

    std::printf("%d\n", nth<primes, 0>::value);
    std::printf("%d\n", nth<primes, 1>::value);
    std::printf("%d\n", nth<primes, 2>::value);
    std::printf("%d\n", nth<primes, 3>::value);
    std::printf("%d\n", nth<primes, 4>::value);
    std::printf("%d\n", nth<primes, 5>::value);

    std::printf("%d\n", add<Int<1>, Int<2>, Int<3>>::type::value);
    std::printf("%d\n", apply<add, IntList<1, 2, 3>>::type::value);
}  
