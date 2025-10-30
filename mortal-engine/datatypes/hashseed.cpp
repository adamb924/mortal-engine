#include "hashseed.h"
#include "qhashfunctions.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
uint ME::HASH_SEED = QHashSeed::globalSeed();
#else
uint ME::HASH_SEED = static_cast<uint>(qGlobalQHashSeed());
#endif

