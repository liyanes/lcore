/**
 * @file pipe.hpp
 * @author liyanes (liyanes@outlook.com)
 * @brief Pipe-styled data handling
 * @version 0.1
 * @date 2024-11-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "config.h"
#include "container/utils.hpp"
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

template <typename Handler>
class Pipe {
    List<Handler> m_handlers;
public:
    inline operator()()
};

LCORE_NAMESPACE_END
