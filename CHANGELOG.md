**Added support for multi-speed pumps**

**Breaking change**
* This changes the pumps to type fan, but does not delete the old pump switches. You will need to manually delete them with MQTT explorer or disable them in HA.
* This will break any pump automations you have.

**Added mDNS support (default name espa.local)

**Added support for SV3-VH (aka Craig's Spa)