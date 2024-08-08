
#include "fuxi/shm/port/xsi_segment.h"

#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "fuxi/common/log.h"
#include "fuxi/common/util.h"
#include "fuxi/shm/impl/segment.h"
#include "fuxi/shm/impl/shm_conf.h"

namespace next {
namespace fuxi {

XsiSegment::XsiSegment(const SegmentOption& opt) : Segment(opt) {
  std::string channel_id_str = std::to_string(opt.channel_id);
  std::string domain_id_str = std::to_string(opt.domain_id);
  path_ = "/tmp/" + domain_id_str + "/" + channel_id_str;
}

XsiSegment::~XsiSegment() {
  Destroy();
  Reset();
}

bool XsiSegment::OpenOrCreate() {
  RETURN_VAL_IF2(open_flag_, true);

  if (key_ == -1) {
    ::next::fuxi::Util::EnsureDirectory(path_);
    // key_ = ftok(path_.c_str(), 'R');
    key_ = static_cast<key_t>(std::hash<std::string>()(path_));
  }

  // layout segment
  int retry = 0;
  int shmid = 0;
  uint64_t total_shm_size = TotalSegmentSpaceSize();

  // do retry times
  while (retry++ < 2) {
    shmid = shmget(key_, total_shm_size, 0644 | IPC_CREAT | IPC_EXCL);
    FUXI_DO_IF(shmid != -1, break;);

    // open if already exist
    if (EEXIST == errno) {
      // already exist
      // COMM_INFO << "shm already exist, open only.";
      if (OpenOnly()) {
        return true;
      }

      // open only failed, continue try
      continue;
    }

    // COMM_ERROR << "get shm failed, error: " << strerror(errno);
    if (EINVAL == errno) {
      // COMM_ERROR << "the shm size: " << conf_.managed_shm_size()
      //      << " is larger than system allows, you can write "
      //      << "\"kernel.shmmax = 268435456\" to /etc/sysctl.conf, then sysctl
      //      -p";
      std::cout << "open failed " << strerror(errno) << std::endl;
      Reset();
      Remove();
      break;
    }
  }
  if (shmid == -1) {
    std::cout << "open failed" << std::endl;
    return false;
  }

  std::cout << "attach shm addr " << (void*)segment_base_addr_ << std::endl;
  // attach shm addr
  head_protection_addr_ = shmat(shmid, segment_base_addr_, 0);
  FUXI_DO_IF(head_protection_addr_ == reinterpret_cast<void*>(-1), {
    // COMM_ERROR << "attach shm failed." << strerror(errno) << "
    // segment_base_addr_ = "
    // << segment_base_addr_;
    shmctl(shmid, IPC_RMID, 0);
    return false;
  });

  managed_shm_header_ = static_cast<void*>(
      static_cast<char*>(head_protection_addr_) + page_size_);

  // add head & tail page size barrier
  void* tail_protect_addr = static_cast<void*>(
      static_cast<char*>(head_protection_addr_) + total_shm_size - page_size_);
  FUXI_DO_IF(mprotect(head_protection_addr_, page_size_, PROT_READ) == -1 ||
                 mprotect(tail_protect_addr, page_size_, PROT_READ) == -1,
             {
               // COMM_ERROR << "mprotect failed: " << strerror(errno);
               shmdt(head_protection_addr_);
               head_protection_addr_ = nullptr;
               managed_shm_header_ = nullptr;
               shmctl(shmid, IPC_RMID, 0);
               return false;
             });

  shm_id_ = shmid;
  create_flag_ = true;
  open_flag_ = true;
  return true;
}

bool XsiSegment::OpenOnly() {
  RETURN_VAL_IF2(open_flag_, true);

  if (key_ == -1) {
    ::next::fuxi::Util::EnsureDirectory(path_);
    // key_ = ftok(path_.c_str(), 'R');
    key_ = static_cast<key_t>(std::hash<std::string>()(path_));
  }

  int shmid = shmget(key_, 0, 0644);
  FUXI_DO_IF(shmid == -1, {
    // COMM_ERROR << "get shm failed. error: " << strerror(errno);
    return false;
  });

  // attach managed_shm_
  head_protection_addr_ = shmat(shmid, segment_base_addr_, 0);
  FUXI_DO_IF(head_protection_addr_ == reinterpret_cast<void*>(-1), {
    // COMM_ERROR << "attach shm failed." << strerror(errno);
    return false;
  });

  managed_shm_header_ = static_cast<void*>(
      static_cast<char*>(head_protection_addr_) + page_size_);

  // add head & tail page size barrier
  void* tail_protect_addr =
      static_cast<void*>(static_cast<char*>(head_protection_addr_) +
                         TotalSegmentSpaceSize() - page_size_);
  FUXI_DO_IF(mprotect(head_protection_addr_, page_size_, PROT_READ) == -1 ||
                 mprotect(tail_protect_addr, page_size_, PROT_READ) == -1,
             {
               // COMM_ERROR << "mprotect failed: " << strerror(errno);
               shmdt(head_protection_addr_);
               head_protection_addr_ = nullptr;
               managed_shm_header_ = nullptr;
               return false;
             });

  open_flag_ = true;
  return true;
}

bool XsiSegment::Remove() {
  int shmid = shmget(key_, 0, 0644);
  FUXI_DO_IF(shmid == -1 || shmctl(shmid, IPC_RMID, 0) == -1, {
    // COMM_ERROR << "remove shm failed, error code: " << strerror(errno);
    return false;
  });
  // COMM_INFO << "remove success.";

  return true;
}

void XsiSegment::Reset() {
  // TODO
  // LogicReset();

  FUXI_DO_IF(head_protection_addr_ != nullptr, {
    shmdt(head_protection_addr_);
    head_protection_addr_ = nullptr;
    managed_shm_header_ = nullptr;
    return;
  });
}

}  // namespace fuxi
}  // namespace next
