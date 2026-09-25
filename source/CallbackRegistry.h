#pragma once
#include <algorithm>
#include <type_traits>
#include <vector>

// Third-party callbacks may register additional callbacks or reenter the API.
// Snapshot dispatch makes new registrations visible on the next outer call.
template<class Function> class SkyCallbacks {
    std::vector<Function> entries;
    bool dispatching = false;
public:
    void Add(Function callback) {
        if (callback && std::find(entries.begin(), entries.end(), callback) == entries.end()) entries.push_back(callback);
    }
    template<class... Args> auto Invoke(Args... args) -> std::invoke_result_t<Function, Args...> {
        using Result = std::invoke_result_t<Function, Args...>;
        if (dispatching) {
            if constexpr (std::is_void_v<Result>) return;
            else return Result{};
        }
        const auto snapshot = entries;
        struct Guard { bool& flag; Guard(bool& f):flag(f){flag=true;} ~Guard(){flag=false;} } guard(dispatching);
        for (auto callback : snapshot) {
            if constexpr (std::is_void_v<Result>) callback(args...);
            else if (auto result = callback(args...)) return result;
        }
        if constexpr (!std::is_void_v<Result>) return Result{};
    }
};
