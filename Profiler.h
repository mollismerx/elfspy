#ifndef ELFSPY_PROFILER_H
#define ELFSPY_PROFILER_H

#include <time.h>
#include "elfspy/Capture.h"

namespace spy
{

/**
 * @namespace spy
 * @class Profiler
 */

template <typename H, typename ReturnType, typename... ArgTypes>
class Profiler : public Capture<unsigned long long>
{
public:
  Profiler(H& hook);
  Profiler(const Profiler&) = delete;
  Profiler& operator=(const Profiler&) = delete;
  Profiler(Profiler&& move) = default;
  Profiler& operator=(Profiler& move) = default;
  ~Profiler();

private:
  static ReturnType profile(ArgTypes...);
  static ReturnType (*func_)(ArgTypes...);
  static Profiler* instance_;
  struct Recorder;
  friend class Recorder;
  inline void add(unsigned long long nanoseconds)
  {
    this->captures_.push_back(nanoseconds);
  }
  struct Recorder
  {
    inline Recorder()
    {
      clock_gettime(CLOCK_REALTIME, &start_);
    }
    inline ~Recorder()
    {
      struct timespec finish;
      clock_gettime(CLOCK_REALTIME, &finish);
      unsigned long long nanoseconds = 1000000000ULL
          * (finish.tv_sec - start_.tv_sec)
          + finish.tv_nsec - start_.tv_nsec;
      Profiler::instance_->add(nanoseconds);
    }
    struct timespec start_;
  };
  H& hook_;
};

template <typename H, typename ReturnType, typename... ArgTypes>
Profiler<H, ReturnType, ArgTypes...>*
Profiler<H, ReturnType, ArgTypes...>::instance_ = nullptr;

template <typename H, typename ReturnType, typename... ArgTypes>
ReturnType (*Profiler<H, ReturnType, ArgTypes...>::func_)(ArgTypes...)
  = nullptr;

template <typename H, typename ReturnType, typename... ArgTypes>
inline Profiler<H, ReturnType, ArgTypes...>::
Profiler(H& hook)
  :hook_(hook)
{
  func_ = hook_.patch(&Profiler::profile);
  instance_ = this;
}

template <typename H, typename ReturnType, typename... ArgTypes>
inline Profiler<H, ReturnType, ArgTypes...>::~Profiler()
{
  hook_.patch(func_);
  instance_ = nullptr;
}

template <typename H, typename ReturnType, typename... ArgTypes>
ReturnType Profiler<H, ReturnType, ArgTypes...>::profile(ArgTypes... args)
{
  Recorder r;
  return (*func_)(std::forward<ArgTypes>(args)...);
}

template <typename H>
inline auto profiler(H& hook)
  -> typename H::template Export<Profiler, H, typename H::Result>::Type
{
  return hook;
}

template <typename H>
inline auto new_profiler(H& hook)
  -> typename H::template Export<Profiler, H, typename H::Result>::Type*
{
  using Install =
    typename H::template Export<Profiler, H, typename H::Result>::Type;
  return new Install(hook);
}

} // namespace elfspy

#endif
