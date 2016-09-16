#ifndef ELFSPY_LAMBDA_H
#define ELFSPY_LAMBDA_H

namespace spy
{

/**
 * @namespace spy
 * @class Lambda
 * This replaces an existing function with a lambda, so that all calls to
 * the program are made to the lambda.
 * The constructor installs the lambda in place of the function, the destructor
 * uninstalls it
 */

template <typename H, typename L, typename ReturnType, typename... ArgTypes>
class Lambda
{
public:
  Lambda(H& hook, L& lambda);
  ~Lambda();

private:
  static ReturnType function(ArgTypes... argtypes);
  H& hook_;
  L lambda_;
  static Lambda* instance_;
  ReturnType (*func_)(ArgTypes...);
};

template <typename H, typename L, typename ReturnType, typename... ArgTypes>
Lambda<H, L, ReturnType, ArgTypes...>*
Lambda<H, L, ReturnType, ArgTypes...>::instance_ = nullptr;

template <typename H, typename L, typename ReturnType, typename... ArgTypes>
inline Lambda<H, L, ReturnType, ArgTypes...>::Lambda(H& hook, L& lambda)
  :hook_(hook)
  ,lambda_(lambda)
{
  instance_ = this;
  func_ = hook.patch(&Lambda<H, L, ReturnType, ArgTypes...>::function);
}

template <typename H, typename L, typename ReturnType, typename... ArgTypes>
inline Lambda<H, L, ReturnType, ArgTypes...>::~Lambda()
{
  hook_.patch(func_);
  instance_ = nullptr;
}

template <typename H, typename L, typename ReturnType, typename... ArgTypes>
ReturnType Lambda<H, L, ReturnType, ArgTypes...>::function(ArgTypes... argtypes)
{
  return instance_->lambda_(std::forward<ArgTypes>(argtypes)...);
}

template <typename H, typename L>
inline auto fake(H& hook, L& lambda)
  -> typename H::template Export<Lambda, H, L, typename H::Result>::Type
{
  return { hook, lambda };
}

template <typename H, typename L>
inline auto new_fake(H& hook, L& lambda)
  -> typename H::template Export<Lambda, H, L, typename H::Result>::Type*
{
  using Install =
      typename H::template Export<Lambda, H, L, typename H::Result>::Type;
  return new Install(hook, lambda);
}

} // namespace elfspy

#endif
