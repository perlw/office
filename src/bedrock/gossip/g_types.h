#pragma once

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
