#pragma once
#include <string>
#include "json.hpp"

namespace lm{
    struct Light{
        std::string name{};
        std::string id{};
        std::string room{};
        int brightness{};
        bool on{}; 
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Light, name, id, room, brightness, on);
}
