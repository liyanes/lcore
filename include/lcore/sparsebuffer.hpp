#pragma once
#include "base.hpp"
#include <map>
#include <vector>
#include <functional>
#include <span>

LCORE_NAMESPACE_BEGIN

/// @brief A sparse buffer that can hold data at arbitrary offsets, with efficient memory usage
template <typename T = char>
class SparseBuffer {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    /// @brief A chunk of data in the sparse buffer
    struct Chunk {
        size_type offset; // Offset of the chunk
        std::vector<T> data; // Data of the chunk

        inline constexpr Chunk(size_type offset, size_type size) : offset(offset), data(size) {}
        inline constexpr size_type size() const { return data.size(); }
        inline constexpr bool contains(size_type pos) const { return pos >= offset && pos < offset + size(); }
        inline constexpr size_type endat() const { return offset + size(); }
        inline constexpr T& operator[](size_type pos) { return data[pos - offset]; }
        inline constexpr const T& operator[](size_type pos) const { return data[pos - offset]; }
    };

    using ChunkMap = std::map<size_type, Chunk>;
private:
    ChunkMap chunks; // Map of chunks, key is the offset of the chunk
    size_type total_size = 0; // Total size of the sparse buffer

    inline constexpr void merge_chunks() {
        if (chunks.empty()) return;
        auto it = chunks.begin();
        auto next_it = std::next(it);
        while (next_it != chunks.end()) {
            if (it->second.endat() >= next_it->first) {
                // Merge next_it into it
                size_type overlap = it->second.endat() - next_it->first;
                if (overlap < next_it->second.size()) {
                    it->second.data.insert(it->second.data.end(),
                                          next_it->second.data.begin() + overlap,
                                          next_it->second.data.end());
                }
                chunks.erase(next_it);
                next_it = std::next(it);
            } else {
                ++it;
                ++next_it;
            }
        }
    }
public:
    inline constexpr SparseBuffer() = default;
    inline constexpr explicit SparseBuffer(size_type size) : total_size(size) {}
    inline constexpr SparseBuffer(const SparseBuffer&) = default;
    inline constexpr SparseBuffer(SparseBuffer&&) noexcept = default;
    inline constexpr SparseBuffer& operator=(const SparseBuffer&) = default;
    inline constexpr SparseBuffer& operator=(SparseBuffer&&) noexcept = default;
    inline constexpr ~SparseBuffer() = default;

    /// @brief Get the total size of the sparse buffer
    inline constexpr size_type size() const { return total_size; } 
    /// @brief Check if the sparse buffer is empty
    inline constexpr bool empty() const { return total_size == 0; }
    /// @brief Clear the sparse buffer
    inline constexpr void clear() { chunks.clear(); total_size = 0; }
    /// @brief Get the number of chunks in the sparse buffer
    inline constexpr size_type chunk_count() const { return chunks.size(); }
    /// @brief Get the chunks in the sparse buffer
    inline constexpr const ChunkMap& get_chunks() const { return chunks; }
    /// @brief Get the chunk that contains the given position, or nullptr if not found
    inline constexpr const Chunk* get_chunk(size_type pos) const {
        auto it = chunks.upper_bound(pos);
        if (it == chunks.begin()) return nullptr;
        --it;
        if (it->second.contains(pos)) return &it->second;
        return nullptr;
    }
    inline constexpr Chunk* get_chunk(size_type pos) {
        auto it = chunks.upper_bound(pos);
        if (it == chunks.begin()) return nullptr;
        --it;
        if (it->second.contains(pos)) return &it->second;
        return nullptr;
    }
    inline constexpr bool has_data(size_type pos) const {
        return get_chunk(pos) != nullptr;
    }
    /// @brief Read data from the sparse buffer, return a span of the data read
    inline constexpr std::span<const T> read(size_type pos, size_type size) const {
        auto chunk = get_chunk(pos);
        if (!chunk) return {};
        size_type read_size = std::min(size, chunk->endat() - pos);
        return std::span<const T>(&(*chunk)[pos], read_size);
    }
    inline constexpr std::span<T> read(size_type pos, size_type size) {
        auto chunk = get_chunk(pos);
        if (!chunk) return {};
        size_type read_size = std::min(size, chunk->endat() - pos);
        return std::span<T>(&(*chunk)[pos], read_size);
    }
    /// @brief Write data to the sparse buffer, return the number of bytes written
    inline constexpr size_type write(size_type pos, std::span<const T> data) {
        if (data.empty()) return 0;
        size_type written = 0;
        size_type data_size = data.size();
        while (written < data_size) {
            size_type current_pos = pos + written;
            auto it = chunks.upper_bound(current_pos);
            if (it != chunks.begin()) {
                auto prev_it = std::prev(it);
                if (prev_it->second.contains(current_pos)) {
                    // Write to existing chunk
                    size_type chunk_written = std::min(data_size - written, prev_it->second.endat() - current_pos);
                    std::copy_n(data.data() + written, chunk_written, &prev_it->second[current_pos]);
                    written += chunk_written;
                    continue;
                }
            }
            // Create a new chunk
            size_type chunk_size = 1;
            if (it != chunks.end()) {
                chunk_size = std::max(chunk_size, it->first - current_pos);
            }
            chunk_size = std::min(chunk_size, data_size - written);
            Chunk new_chunk(current_pos, chunk_size);
            std::copy_n(data.data() + written, chunk_size, new_chunk.data.data());
            chunks.emplace(current_pos, std::move(new_chunk));
            written += chunk_size;
        }
        total_size = std::max(total_size, pos + written);
        merge_chunks();
        return written;
    }
    /// @brief Write data to the sparse buffer, without overwriting existing data, return the number of bytes written
    inline constexpr size_type write_sparse(size_type pos, std::span<const T> data) {
        if (data.empty()) return 0;
        size_type written = 0;
        size_type data_size = data.size();
        while (written < data_size) {
            size_type current_pos = pos + written;
            auto it = chunks.upper_bound(current_pos);
            if (it != chunks.begin()) {
                auto prev_it = std::prev(it);
                if (prev_it->second.contains(current_pos)) {
                    // Skip existing chunk
                    size_type chunk_skipped = std::min(data_size - written, prev_it->second.endat() - current_pos);
                    written += chunk_skipped;
                    continue;
                }
            }
            // Create a new chunk
            size_type chunk_size = 1;
            if (it != chunks.end()) {
                chunk_size = std::max(chunk_size, it->first - current_pos);
            }
            chunk_size = std::min(chunk_size, data_size - written);
            Chunk new_chunk(current_pos, chunk_size);
            std::copy_n(data.data() + written, chunk_size, new_chunk.data.data());
            chunks.emplace(current_pos, std::move(new_chunk));
            written += chunk_size;
        }
        total_size = std::max(total_size, pos + written);
        merge_chunks();
        return written;
    }
    /// @brief Resize the sparse buffer, if the new size is smaller, truncate the buffer
    inline constexpr void resize(size_type new_size) {
        if (new_size < total_size) {
            // Truncate the buffer
            auto it = chunks.lower_bound(new_size);
            if (it != chunks.begin()) {
                auto prev_it = std::prev(it);
                if (prev_it->second.endat() > new_size) {
                    prev_it->second.data.resize(new_size - prev_it->second.offset);
                }
            }
            chunks.erase(it, chunks.end());
        }
        total_size = new_size;
    }
};

LCORE_NAMESPACE_END
