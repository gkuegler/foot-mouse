#ifndef FOOTMOUSE_SERIAL_MSG_PARSING
#define FOOTMOUSE_SERIAL_MSG_PARSING

#include "constants.h"
#include "crc32.h"

struct __attribute__((packed)) SerialMsgHeader
{
  uint32_t sof = 0;
  uint32_t length = 0;
  uint32_t crc32 = 0xDEADBEEF;
  // TODO: cmd is unprotected by crc with this topology.
  uint32_t cmd = 999999;
};

struct __attribute__((packed)) CmdPayloadSetButtonMode
{
  uint8_t pedal_index;
  uint8_t mode;
  uint8_t inversion;
};

// Note that Teensy architecture is little-endian.
struct __attribute__((packed)) CmdPayloadSetKeycombo
{
  uint8_t pedal_index;
  uint8_t inverted;
  uint8_t nKeycodes;
  uint16_t keycodes[64];
};

static_assert(sizeof(CmdPayloadSetButtonMode) < MAX_PAYLOAD_SIZE, "");
static_assert(sizeof(CmdPayloadSetKeycombo) < MAX_PAYLOAD_SIZE, "");

/*
 * Get the next byte with a timeout built in.
 * Returns -1 when no more bytes available.
 */
int
read_next_byte()
{
  constexpr int READ_TIMEOUT_MS = 20;

  for (int i = 0; i < READ_TIMEOUT_MS; i++) {
    auto rb = Serial.read();
    if (rb == -1) {
      delay(1);
      continue;
    }
    return rb;
  }

  Serial.print("Serial read timed out of retires.\n");
  return -1;
}

/* Validates message and returns true if success.
 * buf is filled with the message payload.
 */
bool
validate_frame_and_get_payload(SerialMsgHeader* header,
                               unsigned char* buf,
                               size_t bufsize)
{
  constexpr uint32_t MSOF = 0xFFFFFFFFUL;
  byte index = 0;

  // Clear the input buffer.
  memset(buf, 0, bufsize);

  // First I need to decode the header, which is always a consistent # of bytes.
  // Once I figure byte length, then I can run a while loop to attemp to receive
  // 'x' num of bytes. At the end I crc check.

  // Fill Header Struct
  for (size_t i = 0; i < sizeof(SerialMsgHeader); i++) {
    auto rb = read_next_byte();
    if (rb == -1) {
      Serial.printf("%d: Serial read was -1.\n", i);
      return NULL;
    }
    reinterpret_cast<unsigned char*>(header)[i] = (unsigned char)rb;
  }

  Serial.printf("sof: %X\n", header->sof);
  Serial.printf("length: %d\n", header->length);
  Serial.printf("crc32: %X\n", header->crc32);
  Serial.printf("cmd: %d\n", header->cmd);

  // Check SOF
  if (MSOF != header->sof) {
    Serial.print("No start-of-frame found.\n");
    return NULL;
  }

  // Load payload into buf.
  while (index < header->length) {
    if (index >= bufsize) {
      Serial.print("Message payload size exceded buffer.\n");
      return NULL;
    }

    auto rb = read_next_byte();
    if (rb == -1) {
      Serial.printf("%d: Serial read was -1.\n", index);
      return NULL;
    }

    buf[index++] = (unsigned char)rb;
  }

  // Check payload length.
  if (index != header->length) {
    Serial.printf("Payload length not correct. Recieved '%d' bytes but "
                  "expected '%d' bytes.",
                  index,
                  header->length);
    return NULL;
  }

  // CRC check.
  if (crc::crc32(buf, header->length) != header->crc32) {
    Serial.print("CRC-32 check failed.\n");
    // TODO: need to return crc and other stuff for debug purposes.
    // return NULL;
  }

  return buf;
}

#endif // FOOTMOUSE_SERIAL_MSG_PARSING