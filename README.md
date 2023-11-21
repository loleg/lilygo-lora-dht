DHT OTTA ESP32
---

Scripts for the [LilyGo + LoRa workshop](https://md.coredump.ch/s/9WkGfU7mj) organized with [Glue Software](https://www.glue.ch/de/startseite/) and [TTN Bern](https://www.thethingsnetwork.org/community/Bern/).

## Set up

Note: configure devices in TTN console using:

- Europe SF9 for RX2
- LoRaWAN Spec 1.0.4
- RP002 Regional Parameters 1.0.4 B
- JoinEUI = 00000000

## Payload decoding

See [formatter.js](formatter.js) for Application Uplink custom JS formatter that unpacks the binary data.

Useful links:

- https://www.thethingsindustries.com/docs/integrations/payload-formatters/#working-with-bytes
- https://learn.adafruit.com/the-things-network-for-feather/payload-decoding
- https://github.com/lnlp/LMIC-node/tree/main

