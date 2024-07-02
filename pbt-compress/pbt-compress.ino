// ================================================================================
// PBT-Compress
//
// Special compression method to reduce data send via I2C for PBT
// Developed by Fandi Adinata (https://github.com/SuryaAssistant)
//
// For use: install "Base64" library and copy this file to your project folder
// ================================================================================

#include <Base64.h>

// Helper function to parse a string based on delimiter
String parsing(String &inputString, char delimiter, uint8_t index){
  uint8_t inputLength = inputString.length();
  uint8_t indexNow = 0;
  String outputString;
  for(uint8_t i=0; i<inputLength; i++){
    if(indexNow == index){
      if(inputString[i] != delimiter){
        outputString += inputString[i];
      }
    }
    if(inputString[i] == delimiter){indexNow ++;}
  }
  return outputString;
}

// Helper function to pad hex strings
void padding(String &input, String &output) {
  while (input.length() < 4) {
    input = "0" + input;
  }
  output += input;
}

void compressCommand(String &input, String &output) {
  String hexString;

  // Parsing each value
  uint8_t id = parsing(input, '|', 0).toInt();
  uint8_t mode = parsing(input, '|', 1).toInt();
  uint8_t on_off = parsing(input, '|', 7).toInt();
  int vSet = int(parsing(input, '|', 2).toFloat() * 1000);
  int iSet = int(parsing(input, '|', 3).toFloat() * 1000);
  int vStop = int(parsing(input, '|', 4).toFloat() * 1000);
  int iStop = int(parsing(input, '|', 5).toFloat() * 1000);
  int sampling = parsing(input, '|', 6).toInt();

  // Calculate first group
  String firstGroup = String((id * 100 + mode * 10 + on_off), HEX);
  padding(firstGroup, hexString);

  // Calculate second group
  String temp = String(vSet, HEX);
  padding(temp, hexString);
  temp = String(iSet, HEX);
  padding(temp, hexString);
  temp = String(vStop, HEX);
  padding(temp, hexString);
  temp = String(iStop, HEX);
  padding(temp, hexString);
  temp = String(sampling, HEX);
  padding(temp, hexString);

  // Convert hex string to byte array
  int byteArrayLength = hexString.length() / 2;
  uint8_t byteArray[byteArrayLength];
  for (int i = 0; i < byteArrayLength; i++) {
    sscanf(&hexString[i * 2], "%2hhx", &byteArray[i]);
  }

  // Encode the byte array to base64
  int encodedLength = Base64.encodedLength(byteArrayLength);
  char encodedString[encodedLength + 1];
  Base64.encode(encodedString, (char*)byteArray, byteArrayLength);
  encodedString[encodedLength] = '\0';

  output = encodedString;
}

void decompressCommand(String &input, String &output) {
  String hexString;
  uint8_t on_off;

  // Decode the base64 string to a byte array
  int byteArrayLength = Base64.decodedLength(input.c_str(), input.length());
  uint8_t byteArray[byteArrayLength];
  Base64.decode((char*)byteArray, input.c_str(), input.length());

  // Convert byte array to hex string
  for (int i = 0; i < byteArrayLength; i++) {
    if (byteArray[i] < 0x10) {
      hexString += "0";
    }
    hexString += String(byteArray[i], HEX);
  }

  // Decompose back to original string
  long temp = strtol(hexString.substring(0, 4).c_str(), NULL, 16);
  output = String(temp / 100) + "|" + String((temp % 100) / 10) + "|";
  on_off = temp % 10;

  temp = strtol(hexString.substring(4, 8).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  temp = strtol(hexString.substring(8, 12).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  temp = strtol(hexString.substring(12, 16).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  temp = strtol(hexString.substring(16, 20).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  temp = strtol(hexString.substring(20, 24).c_str(), NULL, 16);
  output += String(temp) + '|';

  output += String(on_off);
}

void compressLog(String &input, String &output) {
  String hexString;

  // Parsing each value
  uint8_t id = parsing(input, '|', 0).toInt();
  uint8_t mode = parsing(input, '|', 1).toInt();
  uint8_t status = parsing(input, '|', 2).toInt();
  int vout = int(parsing(input, '|', 3).toFloat() * 1000);
  int iout = int(parsing(input, '|', 4).toFloat() * 1000);
  int temperature = int(parsing(input, '|', 5).toFloat() * 100);
  int cap = parsing(input, '|', 6).toInt();
  int energy = parsing(input, '|', 7).toInt();

  // Limit value
  if (temperature > 9999) { temperature = 9999; }
  if (temperature < 0) { temperature = 0; }
  if (cap > 65535) { cap = 65535; }
  if (energy > 65535) { energy = 65535; }

  // Calculate first group
  String firstGroup = String((id * 100 + mode * 10 + status), HEX);
  padding(firstGroup, hexString);

  // Calculate second group
  String temp = String(vout, HEX);
  padding(temp, hexString);
  temp = String(iout, HEX);
  padding(temp, hexString);
  temp = String(temperature, HEX);
  padding(temp, hexString);
  temp = String(cap, HEX);
  padding(temp, hexString);
  temp = String(energy, HEX);
  padding(temp, hexString);

  // Convert hex string to byte array
  int byteArrayLength = hexString.length() / 2;
  uint8_t byteArray[byteArrayLength];
  for (int i = 0; i < byteArrayLength; i++) {
    sscanf(&hexString[i * 2], "%2hhx", &byteArray[i]);
  }

  // Encode the byte array to base64
  int encodedLength = Base64.encodedLength(byteArrayLength);
  char encodedString[encodedLength + 1];
  Base64.encode(encodedString, (char*)byteArray, byteArrayLength);
  encodedString[encodedLength] = '\0';

  output = encodedString;
}

void decompressLog(String &input, String &output) {
  output = "";
  String hexString = "";

  // Decode the base64 string to a byte array
  int byteArrayLength = Base64.decodedLength(input.c_str(), input.length());
  uint8_t byteArray[byteArrayLength];
  Base64.decode((char*)byteArray, input.c_str(), input.length());

  for (int i = 0; i < byteArrayLength; i++) {
    if (byteArray[i] < 0x10) {
      hexString += "0";
    }
    hexString += String(byteArray[i], HEX);
  }

  // Decompose back to original string
  long temp;
  int tempInt;
  float tempFloat;

  // Get id
  temp = strtol(hexString.substring(0, 4).c_str(), NULL, 16);
  output += String(temp / 100) + "|" + String((temp % 100) / 10) + "|" + String(temp % 10) + "|";

  // Get voltage
  temp = strtol(hexString.substring(4, 8).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  // Get current
  temp = strtol(hexString.substring(8, 12).c_str(), NULL, 16);
  output += String((float)temp / 1000, 3) + "|";

  // Get temperature
  temp = strtol(hexString.substring(12, 16).c_str(), NULL, 16);
  output += String((float)temp / 100, 2) + "|";

  // Get capacity
  temp = strtol(hexString.substring(16, 20).c_str(), NULL, 16);
  output += String(temp) + "|";

  // Get energy
  temp = strtol(hexString.substring(20, 24).c_str(), NULL, 16);
  output += String(temp);
}
