/**
 * @file std.h
 * @brief Classes that are a part of the standard library (std)
 */

#pragma once

#include "types.h"

namespace std
{
    struct nothrow_t;
    struct __va_list;

    namespace __l
    {
        template<typename T, typename T2>
        class __tree_node;
        
        template<typename T, typename T2>
        class list
        {
        public:
            list(T const &);
        };

        template<typename T, typename T2>
        class pair
        {
        public:
            ~pair();
        };  

        template<typename T, typename T2>
        class vector
        {
        public:
            void reserve(u64);

            void __push_back_slow_path(T const &);
        };

        template<typename T, typename T2, typename T3>
        class __tree
        {
        public:
            void destroy(std::__l::__tree_node<T, void *> *);
        };

        template<typename T, typename T2>
        void __sort(T2, T2, T2, T);

        template<typename T, typename T2>
        void __sort3(T2, T2, T);

        template<typename T, typename T2>
        void __sort5(T2, T2, T2, T2, T2, T);

        template<typename T, typename T2>
        void __insertion_sort_incomplete(T2, T2, T);
    }; 
};