
#include "fuxi/mm/port/posix_shm.h"

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

namespace next {
namespace fuxi {

PosixShm::PosixShm() {}

PosixShm::~PosixShm() { Shutdown(); }

bool PosixShm::Init(const ShmOption& opt) {
  if (inited_) {
    return true;
  }

  opt_ = opt;

  // shm_path_ = std::string("/dev/shm/") + shm_name_;
  std::string domain_id_str = std::to_string(opt.domain_id);
  shm_name_ = domain_id_str + "_" + opt.name;

  OpenOrCreate();
  inited_ = true;
  return true;
}

void PosixShm::Shutdown() {
  if (inited_ == false) {
    return;
  }
  inited_ = false;
  Reset();
  Remove();
}

bool PosixShm::OpenOrCreate() {
  if (inited_) {
    return true;
  }

  int fd = shm_open(shm_name_.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
  if (fd < 0) {
    if (EEXIST == errno) {
      return OpenOnly();
    } else {
      return false;
    }
  }

  if (ftruncate(fd, opt_.size) < 0) {
    std::cout << "ftruncate failed: " << strerror(errno) << std::endl;
    close(fd);
    return false;
  }
  shm_size_ = opt_.size;

  // attach managed_shm_
  managed_shm_ =
      mmap(opt_.addr, opt_.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if ((managed_shm_ == MAP_FAILED) ||
      (opt_.addr != nullptr && managed_shm_ != opt_.addr)) {
    close(fd);
    shm_unlink(shm_name_.c_str());
    return false;
  }
  close(fd);

  inited_ = true;
  return true;
}

bool PosixShm::OpenOnly() {
  if (inited_) {
    return true;
  }

  // get managed_shm_
  int fd = shm_open(shm_name_.c_str(), O_RDWR, 0644);
  if (fd == -1) {
    std::cout << "get shm failed: " << strerror(errno)
              << ", shm_name: " << shm_name_ << std::endl;
    return false;
  }

  struct stat file_attr;
  if (fstat(fd, &file_attr) < 0) {
    std::cout << "fstat failed: " << strerror(errno) << std::endl;
    close(fd);
    return false;
  }
  shm_size_ = file_attr.st_size;

  // attach managed_shm_
  managed_shm_ =
      mmap(opt_.addr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if ((managed_shm_ == MAP_FAILED) ||
      (opt_.addr != nullptr && managed_shm_ != opt_.addr)) {
    std::cout << "attach shm failed: " << strerror(errno) << std::endl;
    close(fd);
    shm_unlink(shm_name_.c_str());
    return false;
  }
  close(fd);

  inited_ = true;
  return true;
}

bool PosixShm::Remove() {
  if (shm_unlink(shm_name_.c_str()) < 0) {
    std::cout << "shm_unlink failed: " << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

void PosixShm::Reset() {
  if (managed_shm_ != nullptr) {
    munmap(managed_shm_, shm_size_);
    managed_shm_ = nullptr;
    return;
  }
}

}  // namespace fuxi
}  // namespace next
