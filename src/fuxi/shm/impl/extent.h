
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "fuxi/shm/impl/block.h"
#include "fuxi/shm/impl/extent_metadata.h"
#include "fuxi/shm/impl/shm_conf.h"

namespace next {
namespace fuxi {

class Extent;
using ExtentPtr = std::shared_ptr<Extent>;

struct WritableBlock {
  uint16_t extent_index{0};
  uint32_t block_index{0};
  Block* block{nullptr};
  uint8_t* buf{nullptr};
};
using ReadableBlock = WritableBlock;

// extent layout:
// field ExtentState
// field free Block id queue
// field Block table
// field Block buffer table
class Extent {
 public:
  Extent();
  ~Extent();

  bool Initialize(void* extent_addr);

  bool AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block);
  void ReleaseWrittenBlock(const WritableBlock& writable_block);

  bool AcquireBlockToRead(ReadableBlock* readable_block);
  void ReleaseReadBlock(const ReadableBlock& readable_block);

  uint32_t GetBlockNum();

  void SetWUser(bool has_w_user, uint32_t block_index);

  void AddRUser(uint32_t block_index);
  void ReduceRUser(uint32_t block_index);

 protected:
  bool init_{false};
  uint64_t channel_id_{0};
  // ShmConf conf_;  // extent conf

  // system domain data
  ExtentMetadata metadata_;

  // record block buffers
  std::mutex block_buf_lock_;
  std::vector<uint8_t*> block_buf_addrs_;

  // extend strategy
  uint32_t expand_increment_{0};
  size_t page_size_{4096};

 private:
  uint32_t GetNextWritableBlockIndex();
};

}  // namespace fuxi
}  // namespace next
