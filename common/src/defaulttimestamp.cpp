#include "defaulttimestamp.h"

#include <chrono>


DefaultTimestamp::DefaultTimestamp() {}

DefaultTimestamp::~DefaultTimestamp() {}

uint64_t DefaultTimestamp::getCurrentTimeStamp() const {
    const auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}


