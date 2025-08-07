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

#include "base.hpp"
#include "container/utils.hpp"
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

template <typename Handler>
requires OneOf<DeclReturnType<Handler>, void, bool>
class Pipe {
    List<Handler> m_handlers;
public:
    using HandlerReturnType = DeclReturnType<Handler>;
    using PipeReturnType = ParameterTuple<Handler>;

    Pipe() {}
    Pipe(const List<Handler>& handlers): m_handlers(handlers) {}
    Pipe(List<Handler>&& handlers): m_handlers(std::move(handlers)) {}

    inline void AddHandler(Handler&& handler){
        m_handlers.push_back(std::move(handler));
    }
    
    template <typename... Args>
    requires Same<HandlerReturnType, void>
    inline PipeReturnType operator()(Args... params){
        for (auto& handler: m_handlers){
            handler(params...);
        }
        return std::make_tuple(params...);
    }

    template <typename... Args>
    requires Same<HandlerReturnType, bool>
    inline std::pair<bool, PipeReturnType> operator()(Args... params){
        for (auto& handler: m_handlers){
            if (handler(params...)){
                // If handler returns true, stop the pipe progration
                return std::make_pair(true, std::make_tuple(params...));
            }
        }
        return std::make_pair(false, std::make_tuple(params...));
    }
};

LCORE_NAMESPACE_END
