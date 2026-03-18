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

  if (bytes.length !== 15) {
    return {
      errors: ["Invalid Collar payload length: " + bytes.length + ", expected 15 bytes"]
    };
  }

  var timestamp2000 = readUint32BE(bytes, 0);
  var longitudeRaw = readUint32BE(bytes, 4);
  var latitudeRaw = readUint32BE(bytes, 8);
  var eventCode = bytes[12];
  var rssi = bytes[13];
  var battery = bytes[14];

  var longitude = (longitudeRaw / 4294967295.0) * 360.0 - 180.0;
  var latitude = (latitudeRaw / 4294967295.0) * 180.0 - 90.0;

  var eventName = "unknown";
  switch (eventCode) {
    case 0:
      eventName = "none";
      break;
    case 10:
      eventName = "light";
      break;
    case 11:
      eventName = "sound";
      break;
    case 12:
      eventName = "shock";
      break;
  }

  return {
    data: {
      protocol_version: 1,
      device_type: "collar",

      ts2000: timestamp2000,
      longitude: longitude,
      latitude: latitude,

      event: eventCode,
      event_name: eventName,

      rssi: rssi,
      battery: battery
    }
  };
}