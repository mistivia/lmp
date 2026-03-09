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
struct ReflexObject {
    ReflexObjectImpl<args...> value;
    template<typename N>
    using value_type = typename lmp::next_of<N, lmp::list<args...>>::type;
    template<typename N>
    value_type<N> get() {
        return getImpl<N, value_type<N>, args...>(value);
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
    printf("%s: %d\n", lmp::list2string<age>::type::value, x.get<age>());
    printf("%s: %lf\n", lmp::list2string<height>::type::value, x.get<height>());
    printf("%s: %s\n", lmp::list2string<name>::type::value, x.get<name>().c_str());
    return 0;
}