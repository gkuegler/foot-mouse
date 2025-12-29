// TinyUSB-based compatibility shim that provides the minimal
// Keyboard / Mouse API used by this sketch.
// - Implements: begin(), write(), press(), release(), releaseAll()
// - Implements: begin(), press(), release(), click()


// Prevent compiling if not using an architecture that uses tinyusb.
#if defined(ARDUINO_ARCH_NRF52)

#include "tinyusbhidshim.h"

#include <Arduino.h>

// Seeed Studio has provided compatible versions of adafruit's tinyusb library.
#include <Adafruit_TinyUSB.h>
#include "FreeRTOS.h"

#include "tinyusbkeycodes.h"

static_assert(CFG_TUD_HID);
static_assert(CFG_TUD_CDC);

#define FALSE 0

namespace HIDCompat {

// Report ID
enum
{
  RID_KEYBOARD = 1,
  RID_MOUSE,
  RID_CONSUMER_CONTROL, // Media, volume etc ..
};

static Adafruit_USBD_HID usb_hid;
static bool initialized = false;

// Using a composite device also requires report_id's in report function calls.
// composite HID report: keyboard (ID 1), mouse (ID 2), consumer (ID 3)
static uint8_t const desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(RID_MOUSE)),
    // TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))
};


static void init_usb() {
  // This is for the modern version 3.X
  // Seedstudio includes TinyUSB 1.X in its board package.
  if (!TinyUSBDevice.isInitialized()) {
    Serial.println("Initializing usb.");

    TinyUSBDevice.begin(0);  
  }

  if (!initialized){

    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

    // The primary purpose of the boot protocol is to provide a simplified,
    // standardized communication method for essential input devices like
    // keyboards and mice, so they can be used in environments that do not have a
    // full-featured USB driver stack, such as the system's BIOS/UEFI setup
    // utility. usb_hid.setBootProtocol(HID_ITF_PROTOCOL_MOUSE);

    usb_hid.setPollInterval(2);  // ms
    
    // TinyUSBDevice.setManufacturerDescriptor("Me");
    // TinyUSBDevice.setProductDescriptor("Composite HID");
    
    // Up to 11 string descriptors?
    // This is a bug! Don't set this.
    // usb_hid.setStringDescriptor("nRF52xTUSB");

    if(!usb_hid.begin()) {
      Serial.println("Failed to begin usb_hid.");
    }

    // If already enumerated, additional class driverr begin() e.g msc, hid,
    // midi won't take effect until re-enumeration.
    if (TinyUSBDevice.mounted()) {
      Serial.println("tusb already mounted.");
      TinyUSBDevice.detach();
      delay(10);
      TinyUSBDevice.attach();
    }

    initialized = true;
  }

}

static bool make_usb_ready(Adafruit_USBD_HID& usb){
  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host.
    Serial.println("Performing remote wakeup");
    TinyUSBDevice.remoteWakeup();
  }

  for (size_t i = 0; i < 1000; i++)
  {
    if (usb.ready()) {
      Serial.printf("USB Ready Count: %d\n", i);
      return true;
    } else {
      vTaskDelay(1);
      continue;
    }
  }

  return false;
}

static void flush() {
  // Send Report will wait for usb to become available again, so it's okay to
  // have no timeout before returning.
  tud_task_ext(0, FALSE);
}

void
KeyboardTinyUsbShim::begin()
{
  init_usb();
}

bool
KeyboardTinyUsbShim::send_report()
{
  // // report id = 0, modifiers, 6 keycodes
  // // tud_hid_keyboard_report(0, _mod, _keys);

  if (!make_usb_ready(usb_hid)) {
    Serial.println("usb_hid not ready");
    return false;
  }


  if (usb_hid.ready()) {
    // return usb_hid.keyboardReport(RID_KEYBOARD, _mod, _keys);
     bool result = usb_hid.keyboardReport(RID_KEYBOARD, _mod, _keys);
     flush();
     return result;

  }  
  
  return false;
}

/*
 * Add the key to the next available slot. Does nothing if a key is already
 * present in the keycode structure.
 */
void
KeyboardTinyUsbShim::add_key(uint8_t scode)
{
  if (scode == 0)
    return;
  for (int i = 0; i < 6; ++i)
    if (_keys[i] == scode)
      return;
  for (int i = 0; i < 6; ++i) {
    if (_keys[i] == 0) {
      _keys[i] = scode;
      return;
    }
  }
}

void
KeyboardTinyUsbShim::remove_key(uint8_t scode)
{
  for (int i = 0; i < 6; ++i) {
    if (_keys[i] == scode) {
      _keys[i] = 0;
      break;
    }
  }
  // compact keys (optional) to keep deterministic reports
  uint8_t tmp[6] = { 0 };
  int idx = 0;
  for (int i = 0; i < 6; ++i)
    if (_keys[i])
      tmp[idx++] = _keys[i];
  memcpy(_keys, tmp, sizeof(_keys));
}

uint8_t
KeyboardTinyUsbShim::ascii_to_hid_usage_id(char c, uint8_t& out_mod)
{
  out_mod = 0;
  if (c >= 'a' && c <= 'z')
    return 0x04 + (c - 'a'); // a..z
  if (c >= 'A' && c <= 'Z') {
    out_mod = (uint8_t)(MODIFIERKEY_SHIFT & 0xFF);
    return 0x04 + (c - 'A');
  }
  if (c >= '1' && c <= '9')
    return 0x1E + (c - '1'); // 1..9
  if (c == '0')
    return 0x27;
  if (c == ' ')
    return 0x2C;
  if (c == '\n' || c == '\r')
    return 0x28; // Enter
  if (c == '\t')
    return 0x2B; // Tab
  if (c == '-')
    return 0x2D;
  if (c == '=')
    return 0x2E;
  if (c == '[')
    return 0x2F;
  if (c == ']')
    return 0x30;
  if (c == '\\')
    return 0x31;
  if (c == ';')
    return 0x33;
  if (c == '\'')
    return 0x34;
  if (c == '`')
    return 0x35;
  if (c == ',')
    return 0x36;
  if (c == '.')
    return 0x37;
  if (c == '/')
    return 0x38;
  // Add more mappings as required.
  return 0;
}

void
KeyboardTinyUsbShim::write(char c)
{
  uint8_t mod = 0;
  uint8_t uid = ascii_to_hid_usage_id(c, mod);
  uint8_t prev_mod = _mod;
  uint8_t prev_keys[6];

  if (uid == 0)
    return;

  memcpy(prev_keys, _keys, sizeof(_keys));

  _mod |= mod;
  memset(_keys, 0, sizeof(_keys));
  _keys[0] = uid;
  send_report();

  // Release & Restore
  _mod = prev_mod;
  memcpy(_keys, prev_keys, sizeof(_keys));
  send_report();
}

/*
Currently only accepts Macros.
TODO: Should I remove ASCII interface compliance?
*/
bool
KeyboardTinyUsbShim::press(uint16_t k)
{
  // Using Paul's Teensy key mapping.
  if ((k & 0xFF00) == 0xE000) {
    _mod |= (uint8_t)(k & 0xFF);
    Serial.println("treat common modifiers");    
  } else if ((k & 0xFF00) == 0) {
    uint8_t mod = 0;
    uint8_t uid = ascii_to_hid_usage_id((uint8_t)k, mod);
    _mod |= mod;
    add_key((uint8_t)uid);
  } else {
    add_key((uint8_t)k);
  }
  return send_report();
}


bool
KeyboardTinyUsbShim::release(uint16_t k)
{
if (k & 0xE000) {
    _mod &= ~(uint8_t)(k & 0xFF);
    Serial.println("treat common modifiers");    
  } else {
    remove_key((uint8_t)k);
  }
  return send_report();
}

bool 
KeyboardTinyUsbShim::releaseAll()
{
  Serial.println("releasing all keys");

  _mod = 0;
  memset(_keys, 0, sizeof(_keys));
  return send_report();
}

/* MouseCompat */
void
MouseTinyUsbShim::begin()
{
  init_usb();
}

bool
MouseTinyUsbShim::press(uint8_t buttons)
{
  _buttons |= buttons;

  if (!make_usb_ready(usb_hid)) {
    Serial.println("usb_hid not ready");
    return false;
  }

  return usb_hid.mouseButtonPress(RID_MOUSE, _buttons);
}

bool
MouseTinyUsbShim::release(uint8_t buttons)
{
  _buttons &= ~buttons;

  if (!make_usb_ready(usb_hid)) {
    Serial.println("usb_hid not ready");
    return false;
  }

  return usb_hid.mouseReport(RID_MOUSE, _buttons, 0, 0, 0, 0);
}

void
MouseTinyUsbShim::click(uint8_t buttons)
{
  press(buttons);
  delay(2);
  release(buttons);
}

} // namespace HIDCompat

#endif 