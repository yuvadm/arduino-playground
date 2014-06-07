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

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

int byteReceived;
int byteSend;

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

void togglePin(int pin) {
  if (digitalRead(pin) == HIGH) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
}

void loop()
{
  if (RS485Serial.available()) 
  {
    byteSend = RS485Serial.read();
    togglePin(byteSend - 97 - 8 + 4);
    Serial.println(byteSend);
  }
}
