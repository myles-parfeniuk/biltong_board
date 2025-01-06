#pragma once

namespace opee
{
    class ForwardUtil
    {
        public:
            // detect lvalue reference
            template <typename T>
            struct is_lvalue_reference
            {
                    static const bool value = false;
            };

            template <typename T>
            struct is_lvalue_reference<T&>
            {
                    static const bool value = true;
            };

            // remove references from types
            template <typename T>
            struct remove_reference
            {
                    using type = T;
            };

            template <typename T>
            struct remove_reference<T&>
            {
                    using type = T;
            };

            // decay traits
            template <typename T>
            struct decay
            {
                    using type = typename remove_reference<T>::type;
            };

            template <typename T>
            struct decay<T&>
            {
                    using type = typename decay<T>::type;
            };

            template <typename T>
            struct decay<T&&>
            {
                    using type = typename decay<T>::type;
            };

            template <typename T>
            using decay_t = typename decay<T>::type;

            template <typename T>
            static T&& forward(typename remove_reference<T>::type& t)
            {
                if constexpr (is_lvalue_reference<T>::value)
                {
                    // iff lvalue return an lvalue reference
                    return t;
                }
                else
                {
                    // else return an rvalue reference
                    return static_cast<T&&>(t);
                }
            }

            ForwardUtil() = delete;
            ForwardUtil(const ForwardUtil&) = delete;
            ForwardUtil& operator=(const ForwardUtil&) = delete;
    };
}; // namespace opee