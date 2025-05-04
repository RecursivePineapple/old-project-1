
#pragma once

#include <spdlog/spdlog.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include "Utils/jsonstruct.hpp"

#include "Common/Types.hpp"

namespace utils
{
    std::optional<jsontypes::span_t> DoGet(CR<std::string> url);
}
