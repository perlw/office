#pragma once

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(const char *message, void *const subscriberdata, void *const userdata);
