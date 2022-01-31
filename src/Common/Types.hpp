
#pragma once

#include <memory>

template<typename T>
using SP = std::shared_ptr<T>;

template<typename T>
using CR = const T&;

template<typename T>
using SPCR = std::shared_ptr<T> const&;

template<typename T>
struct PTR
{
    PTR(std::nullptr_t) : value(nullptr) { }
    PTR(T *ptr) : value(ptr) { }

    template<typename memory_ptr = std::shared_ptr<T>>
    PTR(memory_ptr const& ptr) : value(ptr.get()) { }

    constexpr operator T*() { return value; }
    constexpr operator bool() { return value != nullptr; }

    constexpr T* operator->() { return value; }

    T *value;
};

#define PROP_GETTER(type, name) virtual type Get##name () = 0;
#define PROP_SETTER(type, name) virtual void Set##name(type const& value) = 0;

#define PROPERTY(type, name) \
    PROP_GETTER(type, name) \
    PROP_SETTER(type, name)

#define IMPLEMENT_PROP_GETTER(type, name, varname) \
    virtual type Get##name () override \
    { return varname; }

#define IMPLEMENT_PROP_SETTER(type, name, varname) \
    virtual void Set##name(type const& value) override \
    { varname = value; }

#define IMPLEMENT_PROPERTY(type, name, varname) \
    private: \
    type varname; \
    public: \
    IMPLEMENT_PROP_GETTER(type, name, varname) \
    IMPLEMENT_PROP_SETTER(type, name, varname)
