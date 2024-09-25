#pragma once
#include "config.h"

#ifdef LCORE_ENABLE_ASSERT

#include <iostream> // std::cerr needs this

#ifdef LCORE_DEBUG
#define LCORE_ABORT() do {              \
    __asm__ __volatile__("int $3");      \
} while (0)
#else
#define LCORE_ABORT() do {std::abort();} while (0)
#endif

#define LCORE_ASSERT(condition, msg)                                                            \
    do {                                                                                        \
        if (!(condition)){                                                                      \
            std::cerr << "Assertion failed: " << #condition << ", " << msg << std::endl;        \
            LCORE_ABORT();                                                                      \
        }                                                                                       \
    } while (0)                                                                                 
#else
#define LCORE_ASSERT(condition, msg) do {} while (0)
#endif

#include <iostream>
#define LCORE_FATAL(msg) do {std::cerr << msg << std::endl; exit(-1);} while (0)
