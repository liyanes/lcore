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
requires (IsSame<ResultCallable<Handler, FirstParameterType<Handler>>, void> ||
        IsSame<ResultCallable<Handler, FirstParameterType<Handler>>, bool>)
class Pipe {
    List<Handler> m_handlers;
public:
    using PipeDataType = FirstParameterType<Handler>;
    
    inline EnableIf<IsSame<ResultCallable<Handler, FirstParameterType<Handler>>, void>, PipeDataType>& operator()(PipeDataType& param){
        for (auto& handler: m_handlers){
            handler(param);
        }
        return param;
    }

    inline EnableIf<IsSame<ResultCallable<Handler, FirstParameterType<Handler>>, bool>, PipeDataType>& operator()(PipeDataType& param){
        for (auto& handler: m_handlers){
            if (handler(param)){
                // If handler returns true, stop the pipe progration
                break;
            }
        }
        return prarm;
    }
};

LCORE_NAMESPACE_END
