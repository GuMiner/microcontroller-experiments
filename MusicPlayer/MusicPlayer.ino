#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_PN532.h>
#include <Adafruit_VS1053.h>

// Full wiring diagram:
/*
// As a note, in the UNO R4 Minima/Nano V3 CS == SSLB, MOSI == COPI, MISO == CIPO, CLK == RSPCK

// ###### SPI, PN532 NFC ######
5V -- NFC (VCC)
GND -- NFC (GND)

D5 --- NFC (IRQ)
D6 --- NFC (RESET)

D10 --- VS1053 (CS)
D11 --- VS1053 (MOSI)
D12 --- VS1053 (MISO)
D13 --- VS1053 (CLK)

// ###### SPI, VS1053 ######
5V -- VS1053 (VCC)
GND -- VS1053 (GND)

D3 --- VS1053 (DREQ)
D4 --- VS1053 (SDCS)
D7 --- VS1053 (XDCS)
D8 --- VS1053 (RST)

D9 --- VS1053 (CS)
D11 --- VS1053 (MOSI)
D12 --- VS1053 (MISO)
D13 --- VS1053 (CLK)

*/

#define PN532_IRQ   5 // For interrupts, but used as a manual digital input
#define PN532_RESET 6 
#define PN532_CHIP_SELECT 10

#define VS1053_DREQ 3      // VS1053 Data request, on an interrupt pin
#define VS1053_CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define VS1053_BREAKOUT_DCS    7      // VS1053 Data/command select pin (output)
#define VS1053_BREAKOUT_RESET  8      // VS1053 reset pin (output)
#define VS1053_BREAKOUT_CS    9     // VS1053 chip select pin (output)

// Both use hardware SPI
Adafruit_PN532 nfc(PN532_CHIP_SELECT);
Adafruit_VS1053_FilePlayer musicPlayer(VS1053_BREAKOUT_RESET, VS1053_BREAKOUT_CS, VS1053_BREAKOUT_DCS, VS1053_DREQ, VS1053_CARDCS);

void setup(void) {
  Serial.begin(115200);

  // For debugging, halt until a serial connection is established
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1);
  }

  nfc.SAMConfig();
  pinMode(PN532_IRQ, INPUT_PULLUP);

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.println((versiondata>>16) & 0xFF, DEC);


  if (!musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Didn't find VS1053"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
 
   if (!SD.begin(VS1053_CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);
  }

  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(80,80);

  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); 
  Serial.println(F("Playing track"));
  musicPlayer.startPlayingFile("/test1.mp3");

  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
  attachInterrupt(digitalPinToInterrupt(PN532_IRQ), cardDetectedInterrupt, FALLING); // TODO need to rewire IRQ to pin 2, because that's the only digital interrupt on the nano. 
  // The '~' symbol means PWM, not interrupt.
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

void readCard() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readDetectedPassiveTargetID(uid, &uidLength);

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

volatile bool hasCard = false;
int nfcInterruptState = HIGH;

void loop() {
  delay(20);
  if (hasCard) {
    readCard();
    hasCard = false;
  }
}

void cardDetectedInterrupt() {
  hasCard = true;
  Serial.println("Detected card");
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

