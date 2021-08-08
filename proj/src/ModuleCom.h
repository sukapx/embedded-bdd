#pragma once

#include <stdint.h>
#include <stdlib.h>

struct FuncFrame
{
  uint8_t func;
  uint8_t subFunc;
  union {
    float f;
    int32_t i32;
  };
};

/**
 * @brief Sends data on Serial
 * Will block thread until data is send or timeout
 * 
 * @param data 
 * @param length 
 * @param timeout in Seconds
 * @return true 
 * @return false 
 */
bool dataSend(const uint8_t* const data, const size_t length, const float timeout = 1.F);

bool dataSendFrame(const FuncFrame& frame, const float timeout = 1.F);
