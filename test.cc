#include <type_traits>
#include <cstdio>
struct nil {};

template<typename A, typename D>
struct cons { using car = A; using cdr = D; };

template<typename L>
using car  = typename L::car;

template<typename L>
using cdr  = typename L::cdr;

template<int N>
using cint = std::integral_constant<int, N>;

template<typename L, typename R>
using eq = std::is_same<L, R>;

template<typename T>
struct delay { using type = T; };

template<bool B, typename tb, typename fb>
struct cond;

template<typename tb, typename fb>
struct cond<true, tb, fb> {
  using type = typename tb::type;
};

template<typename tb, typename fb>
struct cond<false, tb, fb> {
  using type = typename fb::type;
};

template<typename L>
struct length;

template<typename lhs, typename rhs>
struct add {
  using type = cint<lhs::type::value + rhs::type::value>;
};

template<typename L>
struct length {
    struct cdr_length_thunk {
        using type = typename length<cdr<L>>::type;
    };
    using type = typename cond<
        eq<L, nil>::value,
        delay<cint<0>>,
        add<delay<cint<1>>, cdr_length_thunk>
    >::type;
    static constexpr int value = type::value;
};
// test
using my_list = cons<cint<1>, cons<cint<2>, nil>>;
int main() {
  std::printf("first: %d\n", car<my_list>::value);
  std::printf("second: %d\n", car<cdr<my_list>>::value);
  std::printf("length: %d\n", length<my_list>::value);
}