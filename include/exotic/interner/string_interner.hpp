// Copyright (c) May 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#pragma once

#include "../memory/unsynchronized_chunk_allocator.hpp"

#include <cstring>
#include <mutex>
#include <new>
#include <shared_mutex>
#include <string_view>
#include <unordered_set>
#include <iostream>

namespace exotic::intern {

class alignas(std::hardware_destructive_interference_size) StringInterner {
public:
    explicit StringInterner(exotic::memory::unsynchronized_chunk_allocator<char>&& upstream)
        : ressource_{ std::move(upstream) } {}

    ~StringInterner() = default;

public:
    const char* intern(std::string_view sv) {
        {
            std::shared_lock<std::shared_mutex> rlock(shared_mutex_);

            if (auto it = table_.find(sv); it != table_.end()) {
                return it->data();
            }
        }

        {
            std::unique_lock<std::shared_mutex> wlock(shared_mutex_);

            if (auto it = table_.find(sv); it != table_.end()) [[unlikely]] {
                return it->data();
            }

            char* cptr = static_cast<char*>(ressource_.allocate_bytes(sv.size() + 1, 1));

            std::memcpy(cptr, sv.data(), sv.size());
            cptr[sv.size()] = '\0';

            table_.emplace(std::string_view(cptr));

            return static_cast<const char*>(cptr);
        }
    }

private:
    std::unordered_set<std::string_view> table_;
    exotic::memory::unsynchronized_chunk_allocator<char> ressource_;

    std::shared_mutex shared_mutex_;
};

}
