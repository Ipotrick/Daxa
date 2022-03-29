#pragma once

#include <memory>

#include "../DaxaCore.hpp"

namespace daxa {
    template<typename T>
    struct DummyStaticFunctionOverride {
        static void cleanup(std::shared_ptr<T>& value) { }
    };

    template<typename T, typename StaticFunctionOverrideT = DummyStaticFunctionOverride<T>>
    class SharedHandle {
    public:
        using SelfT = SharedHandle<T, StaticFunctionOverrideT>;

        SharedHandle() = default;
        SharedHandle(std::shared_ptr<T> const& other)
            : value{ other }
        { 
            //DAXA_ASSERT_M(value, "can not assign invalid handle");
        }
        SharedHandle(std::shared_ptr<T>&& other)
            : value{ std::move(other) }
        {  
            //DAXA_ASSERT_M(value, "can not assign invalid handle");
        }
        SharedHandle(SelfT&& other) noexcept 
            : value{ std::move(other.value) }
        { 
            //DAXA_ASSERT_M(value, "can not assign invalid handle");
        }
        SharedHandle(SharedHandle<T, StaticFunctionOverrideT> const& other) 
            : value{ other.value }
        { 
            //DAXA_ASSERT_M(value, "can not assign invalid handle");
        }
        SelfT& operator=(SharedHandle<T, StaticFunctionOverrideT>&& other) noexcept {
            //DAXA_ASSERT_M(other, "can not assign invalid handle");
            StaticFunctionOverrideT::cleanup(value);
            value = std::move(other.value);
            return *this;
        }
        SelfT& operator=(SelfT const& other) {
            //DAXA_ASSERT_M(other, "can not assign invalid handle");
            StaticFunctionOverrideT::cleanup(value);
            value = other.value;
            return *this;
        }
        ~SharedHandle() {
            StaticFunctionOverrideT::cleanup(value);
        }

        T& operator*() { return *value; }
        T const& operator*() const { return *value; }
        T* operator->() { return value.get(); }
        T const* operator->() const { return value.get(); }

        operator bool() const { return value.operator bool(); }
        bool operator!() const { return !(value.operator bool()); }

        T* get() { return value.get(); }
        T const* get() const { return value.get(); }

        bool valid() const { return value.operator bool(); }

        std::weak_ptr<T> getWeak() const {
            return { value };
        }

        size_t getRefCount() const { return value.use_count(); }
    protected:
        friend class Device;
        friend class Queue;
        friend class BindingSetAllocator;

        std::shared_ptr<T> value = {};
    };
}