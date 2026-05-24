# Sharded String Interner

A high-performance sharded string interning system with low-contention concurrency design, custom memory resources, and chunk-based allocation strategies. This project is part of the **EXOTIC::memory** / **EXOTIC::intern** ecosystem.

---

## Overview

This project was originally intended to be integrated into a compiler, but it remains a fully generic system that can be used in other performance-critical environments.

It took approximately three weeks to complete during an end-of-term period, from initial design to full implementation. This work has been extremely beneficial in strengthening my understanding of low-level systems, memory management, and multithreaded programming.

Before this project, my knowledge of multithreading was very limited. It served as a foundational step toward understanding professional systems programming concepts such as contention control, memory ownership, concurrent access patterns, and low-level allocation strategies.

---

### Architecture

At its core, the project implements a **sharded string interning architecture** designed to minimize contention while maintaining fast lookup performance.

Incoming strings are distributed across shards using a hash-based sharding function.

Each shard independently maintains:

- An `unordered_set<std::string_view>` acting as the intern registry
- Its own `unsynchronized_chunk_allocator` responsible for local allocation logic
- A dedicated `shared_mutex` protecting concurrent access

The synchronization model is intentionally optimized for read-heavy workloads:

- `Shared` (read) locking is used for lookup operations
- `Exclusive` (write) locking is only required when inserting a new string

---

### Lookup Flow

When a string arrives inside a shard, the system first performs a lookup under a shared lock.

If the string already exists inside the shard registry:

- The existing `string_view` is retrieved
- Its `.data()` pointer is returned immediately
- No allocation occurs
- No write contention is introduced

This makes repeated lookups extremely lightweight.

---

### Allocation Flow

If the string is not found during the lookup phase, the system upgrades to an exclusive lock.

At this point:

1. Memory is requested through the shard's `unsynchronized_chunk_allocator`
2. The chunk allocator itself allocates memory from a shared global atomic memory resource
3. The string data is copied into the allocated memory region
4. A new `std::string_view` referencing this stable memory is inserted into the shard registry
5. The final pointer is returned to the caller

---

### Memory Model

Each shard owns its own chunk allocator, but all allocators ultimately share the same upstream atomic memory resource.

This design provides:

- Localized shard allocation logic
- Centralized thread-safe memory acquisition
- Stable string lifetime guarantees
- Reduced allocation overhead through chunk-based allocation

The atomic memory resource acts as a global allocation pool from which shards acquire large contiguous memory ranges.

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
