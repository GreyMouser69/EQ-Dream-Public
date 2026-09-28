#ifndef EQEMU_ZONE_NMS_ANYTHING_H
#define EQEMU_ZONE_NMS_ANYTHING_H

#include "../common/nms_anything_structs.h"

class Client;

namespace NMSAnything {

// Implemented by the Anything service. Keeping packet dispatch separate from
// Client prevents the custom storage protocol from expanding the native RoF2
// inventory slot map.
void HandleAction(Client *client, const ActionRequest &request);

} // namespace NMSAnything

#endif
