function readUint32BE(bytes, offset) {
  return (
    (((bytes[offset] & 0xff) << 24) >>> 0) +
    ((bytes[offset + 1] & 0xff) << 16) +
    ((bytes[offset + 2] & 0xff) << 8) +
    (bytes[offset + 3] & 0xff)
  ) >>> 0;
}

function decodeUplink(input) {
  var bytes = input.bytes;

  if (bytes.length !== 7) {
    return {
      errors: ["Invalid Bolus payload length: " + bytes.length + ", expected 7 bytes"]
    };
  }

  var timestamp2000 = readUint32BE(bytes, 0);
  var condition = bytes[4];
  var rssi = bytes[5];
  var battery = bytes[6];

  return {
    data: {
      protocol_version: 1,
      device_type: "bolus",
      ts2000: timestamp2000,
      condition: condition,
      normal: condition === 0,
      unavailable: (condition & 1) !== 0,
      atony: (condition & 2) !== 0,
      hypomotility: (condition & 4) !== 0,
      hyperactivity: (condition & 8) !== 0,
      high_temperature: (condition & 16) !== 0,

      rssi: rssi,
      battery: battery
    }
  };
}