function decodeUplink(input) {
  const bytes = input.bytes || [];
  const warnings = [];
  const errors = [];

  const FIELD_NAMES = [
    "soilVWC_5cm",
    "soilVWC_10cm",
    "soilVWC_20cm",
    "soilVWC_50cm",
    "soilVWC_100cm",
    "soilEC_5cm",
    "soilEC_10cm",
    "soilEC_20cm",
    "soilEC_50cm",
    "soilEC_100cm",
    "soilT_5cm",
    "soilT_10cm",
    "soilT_20cm",
    "soilT_50cm",
    "soilT_100cm",
    "precip"
  ];

  if (bytes.length < 2) {
    return {
      data: { measurements: [] },
      warnings: warnings,
      errors: ["Payload too short."]
    };
  }

  const bitmap = (bytes[0] << 8) | bytes[1];
  let offset = 2;
  const measurements = [];

  for (let i = 0; i < FIELD_NAMES.length; i++) {
    if (bitmap & (1 << i)) {
      if (offset + 1 >= bytes.length) {
        errors.push("Payload ended before all bitmap fields were decoded.");
        break;
      }

      let raw = (bytes[offset] << 8) | bytes[offset + 1];
      if (raw & 0x8000) raw -= 0x10000;

      measurements.push({
        name: FIELD_NAMES[i],
        value: raw / 100.0
      });

      offset += 2;
    }
  }

  if (input.fPort !== 2) {
    warnings.push(`Unexpected FPort=${input.fPort}; expected 2.`);
  }

  return {
    data: {
      bitmap: bitmap,
      measurements: measurements
    },
    warnings: warnings,
    errors: errors
  };
}
