
#include "fuxi/base/bounded_queue.h"
#include "fuxi/base/flat_bond_queue.h"
#include "fuxi/base/flat_object_queue.h"
#include "fuxi/base/flat_signal_queue.h"
#include "fuxi/base/rte_ring.h"
#include "fuxi/shm/arena/shm_deleter.h"

namespace next {
namespace fuxi {
ConcurrentBoundedQueue<int32_t> id_queue;
RteRing<void*> buffer_queue;
FlatObjectQueue<int> obj_queue;
FlatSignalQueue<int> signal_queue;
// FlatBondQueue<int> bond_queue;

ShmObjectDeleter shm_deleter;
}  // namespace fuxi
}  // namespace next
