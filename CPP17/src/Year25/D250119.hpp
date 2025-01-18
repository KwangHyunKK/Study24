#include <iostream>
#include <iterator>
#include <type_traits>

namespace Y25M1
{
    // Python's enumerate() in C++
    // Inspired by https://gist.github.com/Bktero/78e46c9713e44fefacd960ff3121eabd

    template<typename Iterable>
    auto enumerate(Iterable&& iterable)
    {
        // Decltype and declval
        // declval is used to create a dummy instance of Iterable without actually constructing it,
        // allowing us to deduce the type of its iterator (Iterator) and the type of elements it produces.
        using Iterator = decltype(std::begin(std::declval<Iterable>()));
        using T = decltype(*std::declval<Iterator>());

        struct Enumerated
        {
            std::size_t index;
            T element;
        };

        struct Enumerator
        {
            Iterator iterator;
            std::size_t index;

            auto operator!=(const Enumerator& other) const{
                return iterator != other.iterator;
            }

            auto& operator++()
            {
                ++iterator;
                ++index;
                return *this;
            }

            auto operator*() const{
                return Enumerated{index, *iterator};
            }
        };

        struct Wrapper{
            Iterable& iterable;

            // [[nodiscard]] is used to indicate that the result of begin() and end() should not be ignored,
            // ensuring better code correctness by warning developers if these functions' return values are not used.
            [[nodiscard]] auto begin() const{
                return Enumerator{std::begin(iterable), 0U};
            }
            [[nodiscard]] auto end() const {
                return Enumerator{std::end(iterable), 0U};
            }
        };

        return Wrapper{std::forward<Iterable>(iterable)};
    }
}