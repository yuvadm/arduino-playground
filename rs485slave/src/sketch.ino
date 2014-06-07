#include <SoftwareSerial.h>
#define SSerialRX        13   // RO
#define SSerialTX        10   // DI
#define SSerialTxControl 11   // DE + RE jumpered 12

#define RelayStartPin    2
#define RelayEndPin      9

#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define RS485Baud        115200

#define SerialDebug      0

// Manually flashed from 0 to 7
#define SlaveId          0

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

int packet;
int tree;
int channel;
int mode;

void setup()
{
  if (SerialDebug) {
    Serial.begin(9600);
    Serial.println("SerialRemote");
  }

  for (int i=RelayStartPin; i<=RelayEndPin; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  RS485Serial.begin(RS485Baud);
}

void loop()
{
  if (RS485Serial.available()) 
  {
    packet = RS485Serial.read();
    /*
      Packet structure is 1 byte == 8 bits
      | Tree ID (3 bits) | Channel ID (3 bits) | Mode (2 bits) |

      Tree ID is 0-7
      Channe ID is 0-7
      Mode is one of:
        0 - Off
        1 - On
        2 - Unused (assume on)
        3 - Unused (assume on)
    */
    tree = (packet & 224) >> 5;
    if (tree == SlaveId) {
      channel = (packet & 28) >> 2;
      mode = packet & 3;
      if (mode) {
        digitalWrite(channel, HIGH);
      } else {
        digitalWrite(channel, LOW);
      }
    }

    if (SerialDebug) {
      Serial.println(packet);
    }
  }
}
