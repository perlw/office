#ifndef OCCULUS_LOADED
#define OCCULUS_LOADED
#include "occulus/occulus.h"
#endif

#ifdef USE_ARCHIVIST
#ifndef ARCHIVIST_LOADED
#define ARCHIVIST_LOADED
#include "archivist/archivist.h"
#endif
#endif

#ifdef USE_BOOMBOX
#ifndef BOOMBOX_LOADED
#define BOOMBOX_LOADED
#include "boombox/boombox.h"
#endif
#endif

#ifdef USE_KRONOS
#ifndef KRONOS_LOADED
#define KRONOS_LOADED
#include "kronos/kronos.h"
#endif
#endif

#ifdef USE_PICASSO
#ifndef PICASSO_LOADED
#define PICASSO_LOADED
#include "picasso/picasso.h"
#endif
#endif

#ifdef USE_RECTIFY
#ifndef RECTIFY_LOADED
#define RECTIFY_LOADED
#include "rectify/rectify.h"
#endif
#endif

#ifdef USE_TOME
#ifndef TOME_LOADED
#define TOME_LOADED
#include "tome/tome.h"
#endif
#endif

#ifndef BEDROCK
#define BEDROCK
double bedrock_time(void);
#endif
