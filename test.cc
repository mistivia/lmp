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

#include <cstdio>
#include "lmp.h"

using namespace lmp;

// tests

static_assert(eq<apply<list, IntList<1, 2, 3>>, IntList<1, 2, 3>>::value);

using reversed_list = reverse<IntList<1, 2, 3>>;
static_assert(length<reversed_list>::type::value == 3);
static_assert(nth<reversed_list, 0>::type::value == 3);
static_assert(nth<reversed_list, 1>::type::value == 2);
static_assert(nth<reversed_list, 2>::type::value == 1);

static_assert(eq<Int<7>, add<Int<3>, Int<4>>>::value);
static_assert(!eq<Int<7>, add<Int<3>, Int<5>>>::value);

// intp checks
static_assert(intp<Int<0>>::value);
static_assert(intp<add<Int<1>, Int<2>>>::value);
static_assert(!intp<nil>::value);
static_assert(!intp<cons<Int<1>, nil>>::value);

// pairp checks
static_assert(pairp<cons<Int<1>, nil>>::value);
static_assert(pairp<IntList<1, 2>>::value);
static_assert(!pairp<nil>::value);
static_assert(!pairp<Int<1>>::value);

// listp checks
static_assert(listp<nil>::value);
static_assert(listp<IntList<1, 2, 3>>::value);
static_assert(!listp<Int<1>>::value);
static_assert(!listp<add<Int<1>, Int<2>>>::value);

// equal checks
static_assert(equal<Int<7>, add<Int<3>, Int<4>>>::value);
static_assert(!equal<Int<8>, add<Int<3>, Int<4>>>::value);
static_assert(equal<IntList<1, 2, 3>, IntList<1, 2, 3>>::value);
static_assert(!equal<IntList<1, 2, 3>, IntList<1, 2, 4>>::value);
static_assert(!equal<IntList<1, 2>, IntList<1, 3>>::value);

// complex equal cases
static_assert(equal<
    cons<IntList<1, 2>, cons<IntList<3, 4>, nil>>,
    cons<IntList<1, 2>, cons<IntList<3, 4>, nil>>
>::value);
static_assert(equal<
    cons<Int<2>, cons<Int<3>, nil>>,
    cons<add<Int<1>, Int<1>>, cons<Int<3>, nil>>
>::value);

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

// cond checks
static_assert(
    cond<std::true_type, Int<1>,
          Int<2>>
    ::type::value == 1);
static_assert(
    cond<std::false_type, Int<1>,
          std::true_type, Int<2>,
          Int<3>>
    ::type::value == 2);
static_assert(
    cond<std::false_type, Int<1>,
          std::false_type, Int<2>,
          Int<3>>
    ::type::value == 3);

// cond short-circuit: later predicate must not be instantiated if already matched
static_assert(
    cond<std::true_type, std::true_type,
          should_not_be_forced, std::false_type,
          std::false_type>\
    ::type::value);

constexpr char hello_str[] = "hello";

static_assert(char_at<hello_str, 0>::type::value == 'h');
static_assert(char_at<hello_str, 1>::type::value == 'e');
static_assert(char_at<hello_str, 4>::type::value == 'o');
static_assert(char_at<hello_str, 5>::type::value == '\0');

using hello_str_lst = string2list<hello_str>;

static_assert(nth<hello_str_lst, 0>::type::value == (int)'h');
static_assert(nth<hello_str_lst, 1>::type::value == (int)'e');
static_assert(nth<hello_str_lst, 4>::type::value == (int)'o');

using hello_str2 = list2string<IntList<'H', 'e', 'l', 'l', 'o'>>::type;
static_assert(hello_str2::value[0] == 'H');
static_assert(hello_str2::value[1] == 'e');
static_assert(hello_str2::value[4] == 'o');
static_assert(hello_str2::value[5] == '\0');

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

// using longlist = range<5, 1000>;
// static_assert(length<longlist>::value == 995);

static_assert(memberp<Int<1>, my_list>::value);
static_assert(memberp<Int<2>, my_list>::value);
static_assert(!memberp<Int<4>, my_list>::value);

static_assert(equal<next_of<Int<1>, my_list>, Int<2>>::value);
static_assert(equal<next_of<Int<2>, my_list>, Int<3>>::value);
static_assert(equal<next_of<Int<3>, my_list>, nil>::value);
static_assert(equal<next_of<Int<4>, my_list>, nil>::value);

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

template<typename T>
using is_even = equal<mod<T, Int<2>>, Int<0>>;

using filtered_empty_list = filter<is_even, nil>;
static_assert(nilp<filtered_empty_list>::value);

using filtered_even_list = filter<is_even, IntList<1, 2, 3, 4, 5, 6>>;
static_assert(equal<filtered_even_list, IntList<2, 4, 6>>::value);

using filtered_all_even = filter<is_even, IntList<2, 4, 6>>;
static_assert(equal<filtered_all_even, IntList<2, 4, 6>>::value);

using filtered_none_even = filter<is_even, IntList<1, 3, 5>>;
static_assert(nilp<filtered_none_even>::value);

static_assert(add<Int<1>, Int<2>, Int<3>>::value == 6);
static_assert(apply<add, IntList<1, 2, 3>>::type::value == 6);

using sum_list = IntList<1, 2, 3, 4, 5, 6, 7, 8>;
static_assert(apply<add,sum_list>::type::value == 36);

using concat_list = concat<IntList<1,2,3>, IntList<4,5,6>>;
static_assert(equal<concat_list, IntList<1, 2, 3, 4, 5, 6>>::value);

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

meta_fn(build_query, typename query) {
    meta_return (list2string<concat<
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
