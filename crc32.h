/*
Taken from https://www.w3.org/TR/png/#D-CRCAppendix

W3C CRC-32 algorithm used in gzip and png specs.
*/

#pragma once

#include <Arduino.h>

namespace crc
{

/* Generate CRC table.*/
void
init();

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t
crc32(unsigned char* buf, int len);

} // namespace crc
