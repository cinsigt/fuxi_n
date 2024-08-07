#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <vector>

namespace next {
namespace fuxi {

struct FieldBufferMetadata {
  int32_t field_id{0};
  size_t repeated_size{0};
  size_t request_space_size{0};
};

struct FieldBuffer {
  FieldBufferMetadata meta;
  void* ptr{nullptr};
  size_t space_size{0};
};

struct StorageSchema {
  std::vector<FieldBufferMetadata> fields;

  size_t FieldSize() const { return fields.size(); }

  size_t TotalSize() {
    size_t total = 0;
    for (size_t i = 0; i < fields.size(); ++i) {
      total += fields[i].request_space_size;
    }
    return total;
  }
};

struct PlacementStorage {
  std::vector<FieldBuffer> field_buffers;

  size_t FieldSize() const { return field_buffers.size(); }

  const FieldBuffer& GetFieldBuffer(size_t i) const { return field_buffers[i]; }

  FieldBuffer& MutableFieldBuffer(size_t i) { return field_buffers[i]; }

  void resize(size_t n) { field_buffers.resize(n); }

  void clear() { field_buffers.clear(); }
};

struct Buffer {
  size_t size{0};
  void* ptr{nullptr};
};

// PiePointer
template <typename T>
class PlacementPointer {
 public:
  PlacementPointer();
  ~PlacementPointer();

  // inline T& operator[](size_t index) noexcept;
  inline T* operator[](size_t index) noexcept;

 private:
  // get local pointer use local access handle id
  static constexpr int MAX_CONCURRENCY = 32;
  T* local_pointers_[MAX_CONCURRENCY];
};

}  // namespace fuxi
}  // namespace next
