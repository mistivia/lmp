#include <string>

#include "lmp.h"

template<typename... args>
struct ReflObjmpl;

template<typename N, typename T, typename... args>
struct ReflObjmpl<N, T, args...> : ReflObjmpl<args...> {
    T value;
    using name = N;
};

template<>
struct ReflObjmpl<> {};

#define DEFINE_LITERAL(_name_)  \
    namespace { \
        inline static constexpr char _name_##_lit[] = #_name_; \
        using _name_ = typename lmp::string2list<_name_##_lit>::type; \
    } \

template<typename N, typename T, typename ...args>
T& getImpl(ReflObjmpl<args...> &obj) {
    using lst = lmp::list<args...>;
    static_assert(lmp::length<lst>::value >= 2);
    if constexpr (lmp::eq<lmp::car<lst>, N>::value) {
        return obj.value;
    } else {
        using BaseType = typename lmp::apply<ReflObjmpl, lmp::cddr<lst>>::type;
        return getImpl<N, T>(static_cast<BaseType&>(obj));
    }
}

template<typename N, typename T, typename ...args>
void setImpl(ReflObjmpl<args...> &obj, T&& new_value) {
    using lst = lmp::list<args...>;
    static_assert(lmp::length<lst>::value >= 2);
    if constexpr (lmp::eq<lmp::car<lst>, N>::value) {
        obj.value = std::forward<T&&>(new_value);
    } else {
        using BaseType = typename lmp::apply<ReflObjmpl, lmp::cddr<lst>>::type;
        return setImpl<N, T>(static_cast<BaseType&>(obj), std::forward<T&&>(new_value));
    }
}

template<typename... args>
void printImpl(ReflObjmpl<args...> &obj) {
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
        using BaseType = typename lmp::apply<ReflObjmpl, lmp::cddr<lst>>::type;
        printImpl(static_cast<BaseType&>(obj));
    } else {
        return;
    }
}

template<typename... args>
struct ReflexObject {
    ReflObjmpl<args...> value;
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
