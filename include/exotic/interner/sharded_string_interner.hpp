// Copyright (c) May 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#pragma once

#include "../memory/unsynchronized_chunk_allocator.hpp"
#include "string_interner.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>

namespace exotic::interner {

template<std::size_t N>
struct ShardedStringInterner {
public:
    using Shard = StringInterner;

public:
    explicit sharded_string_interner(exotic::memory::memory_resource* upstream) {
        for (std::size_t i = 0; i < N; ++i) {
            shards_[i] = ::new Shard(exotic::memory::unsynchronized_chunk_allocator<char>(upstream));
        }
    }

    ~sharded_string_interner() noexcept{
        for (auto* shard : shards_) {
            if (shard == nullptr) [[unlikely]] continue;
            delete shard;
        }
    }

public:
    [[nodiscard]] const char* const intern(std::string_view sv) {
        std::hash<std::string_view> h; std::size_t index = h(sv) % N;
        return shards_[index]->intern(sv);
    }

private:
    std::array<Shard*, N> shards_;
};

}
