#include "hashseed.h"
#include "qhashfunctions.h"

uint ME::HASH_SEED = static_cast<uint>(qGlobalQHashSeed());
