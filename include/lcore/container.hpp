#pragma once
#include "base.hpp"

#include "container/vector.hpp"
#include "container/list.hpp"
#include "container/view.hpp"

LCORE_NAMESPACE_BEGIN

template <typename T>
using VectorView = ConstContainerView<std::vector, T>;

template <typename T>
using ListView = ConstContainerView<std::list, T>;

LCORE_NAMESPACE_END

