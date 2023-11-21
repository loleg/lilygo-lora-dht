#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include "boards.h"

// Switch to MSB mode in the TTN console, then copy the End Device keys here:

// eui-lilygo-glue-00x

// AppEUI or JoinEUI in LSB mode
static const u1_t PROGMEM APPEUI[8] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// DevEUI in ** LSB ** mode
static const u1_t PROGMEM DEVEUI[8] = {
  //0xDD, 0x2C, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 // 000
  0xC1, 0x2C, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 // 001
  //0xDC, 0x2C, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 // 002
};

// AppKey in ** MSB ** mode
static const u1_t PROGMEM APPKEY[16] = {
  //0x3E, 0x82, 0x44, 0x0C, 0x82, 0x45, 0x69, 0x1B, 0xC0, 0x3A, 0xBA, 0x44, 0xA2, 0x92, 0x71, 0xA4 // 000
  0xB4, 0x4E, 0xD1, 0x03, 0xEB, 0xFE, 0xE8, 0x77, 0xEB, 0x10, 0xF2, 0xDD, 0x1C, 0x7E, 0x40, 0x70 // 001
  //0x47, 0x5A, 0x2F, 0xD4, 0x6B, 0xC4, 0x68, 0x4F, 0xA1, 0x99, 0xD2, 0x46, 0xFE, 0x84, 0x55, 0xE1 // 002
};

// Pin mapping
#ifdef STM32L073xx
const lmic_pinmap lmic_pins = {
  .nss = RADIO_CS_PIN,
  .rxtx = RADIO_SWITCH_PIN,
  .rst = RADIO_RST_PIN,
  .dio = { RADIO_DIO0_PIN, RADIO_DIO1_PIN, RADIO_DIO2_PIN },
  .rx_level = HIGH
};
#else
const lmic_pinmap lmic_pins = {
  .nss = RADIO_CS_PIN,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = RADIO_RST_PIN,
  .dio = { RADIO_DIO0_PIN, RADIO_DIO1_PIN, RADIO_BUSY_PIN }
};
#endif

static osjob_t sendjob;
static int spreadFactor = DR_SF7;
static int joinStatus = EV_JOINING;
static const unsigned TX_INTERVAL = 30;
static String lora_msg = "";

// Contains the data that we will be sending
byte lora_payload[5];

void os_getArtEui(u1_t *buf) {
  memcpy_P(buf, APPEUI, 8);
}

void os_getDevEui(u1_t *buf) {
  memcpy_P(buf, DEVEUI, 8);
}

void os_getDevKey(u1_t *buf) {
  memcpy_P(buf, APPKEY, 16);
}


void do_send(osjob_t *j) {
  if (joinStatus == EV_JOINING) {
    Serial.println(F("Not joined yet"));
    // Check if there is not a current TX/RX job running
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);

  //} else if (LMIC.opmode & OP_TXRXPEND) {
  //  Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    Serial.println(F("OP_TXRXPEND, sending ..."));
    // Prepare upstream data transmission at the next possible time.
    //static uint8_t mydata[] = "Hello, world!";
    //LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    LMIC_setTxData2(1, lora_payload, sizeof(lora_payload) - 1, 0);
    // Schedule a callback to check confirmation
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);

#ifdef HAS_DISPLAY
    if (u8g2) {
      char buf[256];
      u8g2->clearBuffer();
      snprintf(buf, sizeof(buf), "[%lu]data sending!", millis() / 1000);
      u8g2->drawStr(0, 12, buf);
      u8g2->sendBuffer();
    }
#endif
  }
}

void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));

      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("Received ack"));
        lora_msg = "Received ACK.";
      }

      lora_msg = "rssi:" + String(LMIC.rssi) + " snr: " + String(LMIC.snr);

      if (LMIC.dataLen) {
        // data received in rx slot after tx
        Serial.print(F("Data Received: "));
        // Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        // Serial.println();
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING: -> Joining..."));
      lora_msg = "OTAA joining....";
      joinStatus = EV_JOINING;
#ifdef HAS_DISPLAY
      if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "OTAA joining....");
        u8g2->sendBuffer();
      }
#endif
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED: -> Joining failed"));
      lora_msg = "OTAA Joining failed";
#ifdef HAS_DISPLAY
      if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "OTAA joining failed");
        u8g2->sendBuffer();
      }
#endif
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      lora_msg = "Joined!";
      joinStatus = EV_JOINED;

#ifdef HAS_DISPLAY
      if (u8g2) {
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Joined TTN!");
        u8g2->sendBuffer();
      }
#endif
      delay(3);
      // Disable link check validation (automatically enabled
      // during join, but not supported by TTN at this time).
      LMIC_setLinkCheckMode(0);

      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void setupLMIC(void) {
  // LMIC init
  os_init();

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);  // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);   // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);    // g2-band
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(spreadFactor, 14);

  // Start job
  LMIC_startJoining();

  // Fire up the join
  do_send(&sendjob);
}

void loopLMIC(float rTemperature, float rHumidity) {

  // prepare byte values for the payload
  uint16_t payloadTemp = round(rTemperature * 100);
  uint16_t payloadHumid = round(rHumidity * 100);
  lora_payload[0] = lowByte(payloadTemp);
  lora_payload[1] = highByte(payloadTemp);
  lora_payload[2] = lowByte(payloadHumid);
  lora_payload[3] = highByte(payloadHumid);

  // just keep spinning..
  os_runloop_once();
}
