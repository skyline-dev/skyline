/**
 * @file std.h
 * @brief Classes that are a part of the standard library (std)
 */

#pragma once

#include <string>

#include "operator.h"
#include "types.h"

namespace std {
struct nothrow_t;
struct __va_list;

namespace __l {
    template <typename T, typename T2>
    class __tree_node;

    template <typename T, typename T2>
    class list {
       public:
        list(T const&);
    };

    template <typename T, typename T2>
    class pair {
       public:
        ~pair();
    };

    template <typename T, typename T2>
    class vector {
       public:
        void reserve(u64);

        void __push_back_slow_path(T const&);
    };

    template <typename T, typename T2, typename T3>
    class __tree {
       public:
        void destroy(std::__l::__tree_node<T, void*>*);
    };

    template <typename T, typename T2>
    void __sort(T2, T2, T2, T);

    template <typename T, typename T2>
    void __sort3(T2, T2, T);

    template <typename T, typename T2>
    void __sort5(T2, T2, T2, T2, T2, T);

    template <typename T, typename T2>
    void __insertion_sort_incomplete(T2, T2, T);
};  // namespace __l

namespace __1 {
    class locale {
        class facet {
            ~facet();
            void __on_zero_shared();
        };

        class id {
            void __init();
            void __get();
        };

       public:
        static locale global(locale const&);
        static locale __global();
        static const locale& classic();

        locale();
        locale(locale const&);
        locale(locale const&, locale const&, int);
        locale(locale const&, std::string const&, int);
        locale(std::string const&);

        std::string name() const;

        bool has_facet(locale::id const&);

        int operator=(locale const&);
        void operator==(locale const&);

        ~locale();
    };
};  // namespace __1
};  // namespace std