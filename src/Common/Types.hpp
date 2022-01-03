
#pragma once

#include <memory>

template<typename T>
using SP = std::shared_ptr<T>;

template<typename T>
using CR = const T&;

template<typename T>
using SPCR = std::shared_ptr<T> const&;
