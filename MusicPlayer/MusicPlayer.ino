#include <Wire.h>
#include <Adafruit_PN532.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

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
#define PN532_IRQ   2
#define PN532_RESET 6  // Not connected by default on the NFC Shield


#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)


Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

Adafruit_VS1053_FilePlayer musicPlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);


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
  Serial.println(F("Playing track 002"));
  musicPlayer.startPlayingFile("/test1.mp3");
}

void loop(void) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(500);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(500);  

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    if (uidLength == 7)
    {
      uint8_t data[32];

      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");

      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:

      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225

      for (uint8_t i = 0; i < 42; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);

        // Display the current page number
        Serial.print("PAGE ");
        if (i < 10)
        {
          Serial.print("0");
          Serial.print(i);
        }
        else
        {
          Serial.print(i);
        }
        Serial.print(": ");

        // Display the results, depending on 'success'
        if (success)
        {
          // Dump the page data
          nfc.PrintHexChar(data, 4);
        }
        else
        {
          Serial.println("Unable to read the requested page!");
        }
      }
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }

    // Wait a bit before trying again
    Serial.println("\n\nSend a character to scan another tag!");
    Serial.flush();
    while (!Serial.available());
    while (Serial.available()) {
    Serial.read(); 
    }


      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
      delay(500);                      // wait for a second
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      delay(500); 

    Serial.flush();
  }
}

