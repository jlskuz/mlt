#ifndef ATOMIC_COMPAT_H
#define ATOMIC_COMPAT_H

#include <stdatomic.h>


//TODO remove when /experimental:c11atomics enabled for MSVC
// #ifdef _MSC_VER
// #include <windows.h>
// #define atomic_int _Atomic(int)
// //#define atomic_int_fetch_add(object, operand) InterlockedExchangeAdd(object, operand)
// //#define atomic_int_load(object) InterlockedCompareExchange(object, 0, 0)
// //#define atomic_int_store(object, desired) InterlockedExchange(object, desired)
// #else
// #include <stdatomic.h>
// #endif

#endif // ATOMIC_COMPAT_H
