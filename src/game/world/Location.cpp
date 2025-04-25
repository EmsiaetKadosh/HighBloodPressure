//
// Created by EmsiaetKadosh on 25-3-23.
//

#include "Location.h"

inline WorldTransportReason::Manager WorldTransportReason::manager = Manager();
inline const WorldTransportReason& WorldTransportReason::Shutdown = *manager.create(L"game shutdown", true, true);
inline const WorldTransportReason& WorldTransportReason::InitialGeneration = *manager.create(L"initial generation", true, true);
inline const WorldTransportReason& WorldTransportReason::WorldCollapse = *manager.create(L"world collapse", true, true);
inline const WorldTransportReason& WorldTransportReason::BlockBreak = *manager.create(L"block break", true, false);
inline const WorldTransportReason& WorldTransportReason::BlockReplace = *manager.create(L"block replace", true, false);
inline const WorldTransportReason& WorldTransportReason::EntityTeleport = *manager.create(L"entity teleport", false, true);
inline const WorldTransportReason& WorldTransportReason::Debug = *manager.create(L"debug", true, true);
