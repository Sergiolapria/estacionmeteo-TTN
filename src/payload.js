function decodeUplink(input) {
  var bytes = input.bytes;

  if (bytes.length < 4) {
    return {
      data: {},
      errors: ["Se esperan al menos 4 bytes, recibidos: " + bytes.length]
    };
  }
  // Temperatura: bytes 0-1 (signed, big-endian) / 100
  var rawTemp = (bytes[3] << 24) + (bytes[2]<<16)+(bytes[1]<<8)+(bytes[0]);
   // convertir a signed
  var temperature = rawTemp / 100;
  // Humedad: bytes 2-3 (unsigned, big-endian) / 100
  var rawHum = (input.bytes[7] << 24) + (input.bytes[6]<<16)+(input.bytes[5]<<8)+(input.bytes[4]);
  var humidity = rawHum / 100;
  var rawPress=(input.bytes[11]<<24)+(input.bytes[10]<<16)+(input.bytes[9]<<8)+(input.bytes[8]);
  var pressure=rawPress/100;
  return {
    data:{
      field1: temperature,
      field2: humidity,
      field3: pressure
    }
};
}