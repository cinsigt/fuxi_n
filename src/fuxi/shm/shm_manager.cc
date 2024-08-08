
#include "fuxi/shm/shm_manager.h"

namespace next {
namespace fuxi {

ShmManager::ShmManager() {}

ShmManager::~ShmManager() {}

bool ShmManager::Initialize() { return true; }

ShmDomainPtr ShmManager::GetDomain(int32_t domain_id) { return nullptr; }

}  // namespace fuxi
}  // namespace next
