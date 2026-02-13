#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_PN532.h>
#include <Adafruit_VS1053.h>

// Full wiring diagram:
/*
// ###### I2C, PN532 NFC ######
5V -- NFC (VCC)
GND -- NFC (GND)
D5 --- NFC (IRQ)
D6 --- NFC (RESET)
SDA --- NFC (SDA)
SCL --- NFC (SCL)

// Because the UNO R4 Minima doesn't include I2C pullup resistors:
SDA --- 4.7k Ω --- 5 V
SCL --- 4.7k Ω --- 5 V

// By using SDA and SCL on this Arduino, A4 and A5 are also in use
A4 --- SDA
A5 --- SCL

// ###### SPI, VS1053 ######

5V -- VS1053 (VCC)
GND -- VS1053 (GND)

D3 --- VS1053 (DREQ)
D4 --- VS1053 (SDCS)
D8 --- VS1053 (XDCS)

// As a note, in the UNO R4 Minima CS == SSLB, MOSI == COPI, MISO == CIPO, CLK == RSPCK
D9 --- VS1053 (RST)
D10 --- VS1053 (CS)
D11 --- VS1053 (MOSI)
D12 --- VS1053 (MISO)
D13 --- VS1053 (CLK)

*/

// TODO -- solder and wire these, as IRQ (interrupts) will be valuable
#define PN532_IRQ   5
#define PN532_RESET 6  // Not connected by default on the NFC Shield
#define PN532_CHIP_SELECT 10  // The 'SS' pin

#define DREQ 3      // VS1053 Data request, ideally an Interrupt pin
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define BREAKOUT_DCS    7      // VS1053 Data/command select pin (output)
#define BREAKOUT_RESET  8      // VS1053 reset pin (output)
#define BREAKOUT_CS    9     // VS1053 chip select pin (output)
// THESE CHIP SELECT PINS NEE

Adafruit_PN532 nfc(PN532_CHIP_SELECT);

Adafruit_VS1053_FilePlayer musicPlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT); // Shared with SPI pin, weird. Doens't work well then though.


Serial.begin(115200);
 while (!Serial) delay(10); // for Leonardo/Micro/Zero

  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);


  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
 
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20,20);

   musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); 
     // Play another file in the background, REQUIRES interrupts!
  Serial.println(F("Playing track"));
  musicPlayer.startPlayingFile("/test1.mp3");
  
}

uint8_t readAsASCII(uint8_t* data, int idx) {
  return (uint8_t)data[idx] - 48; // 49 == 1 in ASCII
}

int8_t readTagNumber() {
  // Based on how the tags were formatted, page 10 has the tag number
  // Arguably I could use the tag UID, but because I've already programmed tags from 1 to 100 I'll use that.
  const uint8_t PAGE_NUMBER = 10;

  uint8_t data[4];
  uint8_t success = nfc.ntag2xx_ReadPage(PAGE_NUMBER, data);

  // The tag number is one to 3 bytes long. data[0] is 2F (/), data[1,2,3] is an ascii number or 254 (end)
  uint8_t currentIdx = 1; // skip the '/'
  int8_t result = readAsASCII(data, currentIdx);
  while (data[currentIdx + 1] != 254) {
    result = result * 10;
    result = result + readAsASCII(data, currentIdx + 1);

    currentIdx = currentIdx + 1;
    if (currentIdx >= 4) {
      break;
    }
  }

  return result;
}

void loop(void) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
  delay(500);                      // wait for a half second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  delay(500);  

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500); // 0.5s timeout

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);

    Serial.print("Tag Number: ");
    Serial.println(readTagNumber(), DEC);
  }
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

