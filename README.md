# Sharded String Interner

A high-performance sharded string interning system with low-contention concurrency design, custom memory resources, and chunk-based allocation strategies. This project is part of the **EXOTIC::memory** / **EXOTIC::intern** ecosystem.

---

## Overview

This project was originally intended to be integrated into a compiler, but it remains a fully generic system that can be used in other performance-critical environments.

It took approximately three weeks to complete during an end-of-term period, from initial design to full implementation. This work has been extremely beneficial in strengthening my understanding of low-level systems, memory management, and multithreaded programming.

Before this project, my knowledge of multithreading was very limited. It served as a foundational step toward understanding professional systems programming concepts such as contention control, memory ownership, concurrent access patterns, and low-level allocation strategies.

Furthermore, it's in this project that I implemented my personal allocation system inspired by `std::pmr`. In fact, I have a repository that deals with it!

---

### Architecture

At its core, this project implements a sharded string interning system designed to reduce contention as much as possible while keeping lookups fast and lightweight.

Strings are distributed across shards using a hash-based sharding function. Each shard is completely independent and contains:

- An `unordered_set<std::string_view>` used as the intern registry
- Its own `unsynchronized_chunk_allocator`
- A dedicated `shared_mutex`

The idea behind the design is fairly simple: most operations are reads, so the synchronization model is optimized around that.

- `Shared` locking is used during lookups
- `Exclusive` locking only happens when a new string must be inserted

This allows already-interned strings to be retrieved extremely quickly with very little contention.

---

### Lookup Flow

When a string enters a shard, the system first checks if it already exists inside the shard registry under a shared lock.

If the string is already interned:

- The existing `string_view` is retrieved
- Its `.data()` pointer is returned directly
- No allocation is performed
- No exclusive locking is required

In practice, repeated lookups become extremely cheap since they mostly stay inside the shared-lock path.

---

### Allocation Flow

If the string is not found during lookup, the system switches to an exclusive lock.

At this stage:

1. Memory is requested from the shard's `unsynchronized_chunk_allocator`
2. The allocator itself acquires memory from a shared upstream atomic memory resource
3. The string is copied into the allocated memory region
4. A new `std::string_view` pointing to this stable memory is inserted into the registry
5. The resulting pointer is returned

Once interned, future lookups will directly reuse the same pointer without allocating again.

---

### Memory Model

Each shard owns its own chunk allocator, but all allocators ultimately share the same upstream atomic memory resource.

The goal here was to keep shard logic local while still centralizing the actual memory acquisition in a concurrency-safe way.

This design provides:

- Local allocation behavior per shard
- Thread-safe upstream memory acquisition
- Stable string lifetime guarantees
- Reduced allocation overhead through chunk-based allocation

The atomic memory resource essentially acts as a global pool from which all shards acquire their memory ranges.

---

## Example Usage

```cpp
exotic::memory::monotonic_atomic_buffer upstream(1 << 20);
exotic::memory::unsynchronized_chunk_allocator<char> alloc(&upstream);

exotic::intern::ShardedStringInterner<8> interner(&alloc); // 8 shards

const char* a = interner.intern("hello");
const char* b = interner.intern("hello"); // same pointer
```

---

## Notes

This project is intentionally designed as a learning-focused system rather than a drop-in standard library replacement. That’s how I learn, and it seems to work very well.

---

## License

This project is licensed under the Boost Software License. See the [LICENSE](LICENSE) file for details.

<p align="center"><sub>© Félix-Olivier Dumas 2026</sub></p>

---
