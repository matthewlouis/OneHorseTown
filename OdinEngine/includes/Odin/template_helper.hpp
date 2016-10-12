#pragma once

#include <type_traits>

namespace meta
{

    ////////////////////////////////////////////////////////////
    #pragma region // Value type of

    template< typename T >
    struct value_type_of
    {
        using type = std::conditional_t<
                         std::is_fundamental< T >::value,
                         T, typename T::value_type >;
    };

    template< typename T >
    using value_type_of_t = typename value_type_of< T >::type;

    #pragma endregion

    /*/ type_changer and its related code is supposedly valid but
    /// most compilers seem to reject it. 

    ////////////////////////////////////////////////////////////
    #pragma region // Type changer

    template< typename T, template< typename > class Finder >
    struct type_changer
    {
        template< typename To >
        using change_type = To;
    };

    template< template< typename... > class T, template< typename > class Finder, typename... Types >
    struct type_changer< T< Types... >, Finder >
    {
        template< typename... Former >
        struct front
        {
            template< typename X, typename... Latter >
            struct back
            {
                template< typename To >
                using next = typename front< Former..., X >::type< To, Latter... >;

                template< typename To >
                using type = std::conditional_t<
                                 std::is_same<
                                     Finder< T< Former..., To, Latter... > >,
                                     To >::value,
                                 T< Former..., To, Latter... >,
                                 next< To > >;
            };

            template< typename X >
            struct back< X >
            {
                template< typename To >
                using type = std::enable_if_t<
                                 std::is_same<
                                     Finder< T< Former..., To > >,
                                     To >::value,
                                 T< Former..., To > >;
            };

            template< typename To, typename... Types >
            using type = typename back< Types... >::type< To >;
        };

    public:
        template< typename To >
        struct change_type
        {
            using type = std::conditional_t<
                             std::is_same<
                                 Finder< T< Types... > >,
                                 To >::value,
                             T< Types... >,
                             typename front<>::type< To, Types... > >;
        };
    };

    template< typename T, template< typename > class Finder, typename To >
    using type_changer_t = typename type_changer< T, Finder >::change_type< To >::type;

    #pragma endregion

    ////////////////////////////////////////////////////////////
    #pragma region // Similar float

    template< typename T >
    struct _similar_float
    {
        using type = float;
    };

    template< typename T >
    struct similar_float
        : _similar_float< std::decay_t< T > >
    {
    };

    template< typename T >
    using similar_float_t = typename similar_float< T >::type;

    template< template< typename... > class T, typename... Types >
    struct _similar_float< T< Types... > >
    {
        using complete_type = T< Types... >;
        using type = type_changer_t<
            complete_type,
            value_type_of_t,
            similar_float_t< value_type_of_t< complete_type > > >;
    };

    #pragma region similar_float specializations

    template<>
    struct _similar_float< void >
    {
        using type = void;
    };

    // Matches double

    template<>
    struct _similar_float< double >
    {
        using type = double;
    };

    template<>
    struct _similar_float< long >
    {
        using type = double;
    };

    template<>
    struct _similar_float< unsigned long >
    {
        using type = double;
    };

    // Matches long double

    template<>
    struct _similar_float< long double >
    {
        using type = long double;
    };

    template<>
    struct _similar_float< long long >
    {
        using type = long double;
    };

    template<>
    struct _similar_float< unsigned long long >
    {
        using type = long double;
    };

    #pragma endregion

    #pragma endregion

    ////////////////////////////////////////////////////////////
    #pragma region // Larger float

    template< typename T >
    struct _larger_float
    {
        using type = double;
    };

    template< typename T >
    struct larger_float
        : _larger_float< similar_float_t< T > >
    {
    };

    template< typename T >
    using larger_float_t = typename larger_float< T >::type;

    template< template< typename... > class T, typename... Types >
    struct _larger_float< T< Types... > >
    {
        using complete_type = T< Types... >;
        using type = type_changer_t<
            complete_type,
            value_type_of_t,
            larger_float_t< value_type_of_t< complete_type > > >;
    };

    template<>
    struct _larger_float< void >
    {
        using type = void;
    };

    template<>
    struct _larger_float< double >
    {
        using type = long double;
    };

    template<>
    struct _larger_float< long double >
    {
        using type = long double;
    };

    #pragma endregion

    //*/

    template <
        template <typename> class Trait,
        typename                  Head,
        typename...               Tail
    >
    struct check_all
    {
        enum {
            value = Trait< Head >::value && check_all< Trait, Tail... >::value
        };
    };

    template <
        template <typename> class Trait,
        typename                  Head
    >
    struct check_all< Trait, Head >
    {
        enum {
            value = Trait< Head >::value
        };
    };

    template < typename T >
    struct converts_to
    {
        template < typename U >
        using check = ::std::is_convertible< T, U >;
    };

    template < bool B >
    using require_condition = typename ::std::enable_if< B >::type;

    template < size_t S, typename... Args >
    using require_pack_size = require_condition< sizeof...(Args) == S >;

    template < typename T, typename... Args >
    using require_homogeneous_args
        = typename ::std::enable_if<
            meta::check_all< meta::converts_to< T >::check, Args... >::value
        >::type;

    #define META_ENSURE_PACK_SIZE_AND_TYPE( _pack_, _size_, _type_ )     \
    typename = meta::require_condition< sizeof...(_pack_) == (_size_) >, \
    typename = meta::require_homogeneous_args< (_type_), _pack_... >

    #define META_WITH_PACK_OF_SIZE_AND_TYPE( _name_, _size_, _type_ ) \
    typename... _name_,                                               \
    META_ENSURE_PACK_SIZE_AND_TYPE( _name_, _size_, _type_ )
}