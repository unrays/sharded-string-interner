// Copyright (c) May 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#pragma once

#include <cstddef>
#include <stdexcept>

namespace exotic::memory {

class memory_resource {
public:
    virtual ~memory_resource() = default;

public:
    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t alignment) noexcept {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) [[unlikely]]
            throw std::invalid_argument("alignment must be a power of two");
        return do_allocate(bytes, alignment);
    }

    void deallocate(void* p, std::size_t bytes, std::size_t alignment) noexcept {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) [[unlikely]]
            throw std::invalid_argument("alignment must be a power of two");
        do_deallocate(p, bytes, alignment);
    }

    bool is_equal(const memory_resource& other) const noexcept {
        return do_is_equal(other);
    }

protected:
    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;
    virtual void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) = 0;

    virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
};

}
