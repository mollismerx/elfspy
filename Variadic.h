#ifndef ELFSPY_VARIADIC_H
#define ELFSPY_VARIADIC_H

namespace spy
{

/**
 * @namespace spy
 * @class Variadic
 * get the n'th argument from a variadic parameter list
 */

template <size_t N, typename T, typename... ArgTypes> struct Variadic;

template <typename T, typename... ArgTypes>
struct Variadic<0, T, ArgTypes...>
{
  typedef T Type;
  static T&& get(T&& first, ArgTypes&&...)
  {
    return std::forward<Type>(first);
  }
};

template <size_t N, typename T, typename... ArgTypes>
struct Variadic
{
  using Next = Variadic<N - 1, ArgTypes...>;
  using Type = typename Next::Type;
  static Type&& get(T&& first, ArgTypes&&... args)
  {
    return std::forward<Type>(Next::get(std::forward<ArgTypes>(args)...));
  }
};

} // namespace elfspy

#endif
