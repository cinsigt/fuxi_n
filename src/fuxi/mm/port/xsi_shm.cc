
#include "fuxi/mm/port/xsi_shm.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#include <thread>

#include "fuxi/common/util.h"

namespace next {
namespace fuxi {

XsiShm::XsiShm() {}

XsiShm::~XsiShm() { Shutdown(); }

bool XsiShm::Init(const ShmOption& opt) {
  if (inited_) {
    return true;
  }

  opt_ = opt;

  std::string name_id_str = std::to_string(opt.name_id);
  std::string domain_id_str = std::to_string(opt.domain_id);
  path_ = "/tmp/xsi/" + domain_id_str + "/" + name_id_str;

  // get key
  ::next::fuxi::Util::EnsureDirectory(path_);
  key_ = ftok(path_.c_str(), 128);

  shm_size_ = opt.size;

  OpenOrCreate();
  inited_ = true;
  return true;
}

void XsiShm::Shutdown() {
  if (inited_ == false) {
    return;
  }

  inited_ = false;
  Reset();
  Remove();
}

bool XsiShm::OpenOrCreate() {
  // create managed_shm_
  int retry = 0;
  int shmid = 0;
  while (retry < 2) {
    shmid = shmget(key_, shm_size_, 0644 | IPC_CREAT | IPC_EXCL);
    if (-1 != shmid) {
      break;
    }

    if (EEXIST == errno) {
      return OpenOnly();
    }

    if (EINVAL == errno) {
      ++retry;
      continue;
    }
    break;
  }

  if (-1 == shmid) {
    return false;
  }

  // attach managed_shm_
  managed_shm_ = shmat(shmid, opt_.addr, 0);
  if (managed_shm_ == reinterpret_cast<void*>(-1)) {
    shmctl(shmid, IPC_RMID, 0);
    return false;
  }

  is_created_ = true;
  return true;
}

bool XsiShm::OpenOnly() {
  // get managed_shm_
  int shmid = shmget(key_, 0, 0644);
  if (shmid == -1) {
    return false;
  }

  // attach managed_shm_
  managed_shm_ = shmat(shmid, opt_.addr, 0);
  if (managed_shm_ == reinterpret_cast<void*>(-1)) {
    return false;
  }

  // open success
  return true;
}

bool XsiShm::Remove() {
  int shmid = shmget(key_, 0, 0644);
  if ((shmid == -1) || (shmctl(shmid, IPC_RMID, 0) == -1)) {
    return false;
  }

  return true;
}

void XsiShm::Reset() {
  if (managed_shm_ != nullptr) {
    shmdt(managed_shm_);
    managed_shm_ = nullptr;
  }
}

}  // namespace fuxi
}  // namespace next
