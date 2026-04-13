Workflow notes
1. Datalogger side
The CR1000X sends a tagged message so the Arduino does not rely on field position alone. This makes the workflow more resilient if a sensor stops reporting.
Example frame:
```text
S,VWC5=0.23,VWC10=0.24,EC5=1.11,T5=21.80,PRECIP=0.00,F
```
2. Arduino side
The MKR WAN 1310:
reads the serial frame
verifies the `S, ... ,F` wrapper
parses each `name=value` field
maps names to fixed indices
builds a bitmap of present sensors
packs values as signed int16 scaled by 100
sends the binary payload on LoRaWAN port 2
3. ChirpStack side
The decoder:
reads the 16-bit bitmap
reads only the values whose bits are set
assigns human-readable field names
Troubleshooting
Joins on USB but not battery
Common causes:
battery supply instability during radio TX
incorrect use of VIN vs battery JST connector
code blocking on serial startup in other sketches
Joins fail repeatedly
Common causes:
incorrect AppEUI/AppKey
region mismatch
US915 channel/sub-band mismatch
Data names do not appear correctly in ChirpStack
Check:
the CR1000X names match the Arduino parser
the Arduino field order matches the ChirpStack `FIELD_NAMES`
ChirpStack is using the correct codec
