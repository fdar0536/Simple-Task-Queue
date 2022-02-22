#pragma once

#include <vector>

#include <cinttypes>

namespace TOTP
{

uint8_t decodeBase32(const uint8_t *, size_t, std::vector<uint32_t> *);

uint8_t getTOTP(uint8_t *);

} // end namespace TOTP
