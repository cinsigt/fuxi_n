
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "fuxi/shm/impl/segment.h"
#include "fuxi/shm/shm_domain.h"
#include "fuxi/shm/topo/domain_layout.h"

namespace next {
namespace fuxi {

class ShmManager {
 public:
  ShmManager();
  ~ShmManager();

  bool Initialize();

  ShmDomainPtr GetDomain(int32_t domain_id);

 private:
  std::unordered_map<int32_t, ShmDomainPtr> domain_map_;
};

}  // namespace fuxi
}  // namespace next
