// Application formatter for The Things Network console

function bin2String(array) {
  var result = "";
  for (var i = 0; i < array.length; i++) {
    result += String.fromCharCode(array[i]);
  }
  return result;
}

function decodeUplink(input) {
  
  // temperature 
  let rawTempC = input.bytes[0] + input.bytes[1] * 256;
  // humidity 
  let rawHumid = input.bytes[2] + input.bytes[3] * 256;

  return {
    data: {
      tempC: rawTempC / 100,
      humid: rawHumid / 100,
      //bytes: input.bytes,
      //str: bin2String(input.bytes),
    },
    warnings: [],
    errors: []
  };
}
