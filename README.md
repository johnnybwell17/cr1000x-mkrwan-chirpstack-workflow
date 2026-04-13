# cr1000x-mkrwan-chirpstack-workflow
CR1000X, MKR WAN 1310, and ChirpStack workflow

# CR1000X → MKR WAN 1310 → ChirpStack Workflow

## Overview
This repo contains the code and documentation for sending tagged sensor data
from a Campbell Scientific CR1000X to an Arduino MKR WAN 1310, then uplinking
to ChirpStack using a compact bitmap-based binary payload.

## Repository Structure
- `cr1000x/` datalogger code
- `arduino/` MKR WAN 1310 sketch
- `chirpstack/` ChirpStack codec
- `docs/` workflow docs

## Data Flow
1. CR1000X sends serial data in tagged format:
   `S,VWC5=0.23,EC5=1.45,T5=21.8,PRECIP=0.00,F`
2. MKR WAN parses tags, builds bitmap payload, and transmits via LoRaWAN
3. ChirpStack decodes the bitmap payload into named measurements

## Sensor Names
- VWC5
- VWC10
- VWC20
- VWC50
- VWC100
- EC5
- EC10
- EC20
- EC50
- EC100
- T5
- T10
- T20
- T50
- T100
- PRECIP

## Notes
- Do not commit real LoRaWAN keys
- Rotate keys if they were ever exposed
