/* Pi-Interface für Christophs Geldautomaten
    Der nano wartet auf einen seriellen String
    in der Form
    "P <x>" wobei <x> zwischen 1 und 10 liegt.
    für jede ausgegebene Münze sendet der Nano
    "H", wenn keine Münzen mehr vorhanden "L"
    oder
    "DEBUG" zum Ein- und Ausschalten von Debug-Info

    Ausgelegt für Arduino Pro
*/

#include <Servo.h>
#include <CmdParser.hpp>
#include <SoftwareSerial.h>

#define ServoPin 9
#define STEP 5
#define WAIT 150
#define CoinSens 8
#define PayTest 5
#define LED 13
#define SpeedPot A0
#define COIN_MIN 1
#define COIN_MAX 20
int SetWait = 4;

Servo PayServo;
CmdParser cmdParser;
CmdBuffer<32> myBuffer;
#define rxPin 2
#define txPin 3
SoftwareSerial PiSerial (rxPin, txPin);

void Payout(int Coins) {
  int pos = 0;
  Serial.print(Coins);
  Serial.println(" Muenzen auszahlen");
  Serial.print("Schrittzeit: ");
  Serial.println(SetWait);
  while (Coins > 0) {
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in Schritten von 1 Grad
      PayServo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SetWait);                       // waits 15ms for the servo to reach the position
    }
    delay(WAIT);
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      PayServo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SetWait);                       // waits 15ms for the servo to reach the position
    }
    delay(WAIT);
    Coins--;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  // set the data rate for the SoftwareSerial port
  PiSerial.begin(9600);
  myBuffer.setEndChar('\r'); // default ist eigentlich \n
  pinMode(ServoPin, OUTPUT);
  pinMode(CoinSens, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(PayTest, INPUT_PULLUP);
  digitalWrite(LED, LOW);
  PayServo.attach(9);  // attaches the servo on pin 9 to the servo object
  PayServo.write(0);
  Serial.println("c ottO 2018");
  delay(100);
}

void loop() {
  int NumberOf50c = 0;
  //Testschalter
  /*
  if (!digitalRead(PayTest)) {
    PiSerial.println("TEST");
    Payout(1);
  }
  else {*/
  Serial.println("Start Reading");
  // Read line from Serial until timeout
  //SetWait = map(analogRead(SpeedPot), 0, 1023, 20, 2);
  if (myBuffer.readFromSerial(&PiSerial, 30000)) {
    //if (digitalRead(PayTest)) {
    if (cmdParser.parseCmd(&myBuffer) != CMDPARSER_ERROR) {
        Serial.println("Tick");
        if (cmdParser.equalCommand("P")) {
          const size_t count = cmdParser.getParamCount();
          Serial.println("Auszahlen");
          if (count != 2) {
            Serial.println("Fehler: Zahl Parameter <> 2");
            PiSerial.println("NACK");
          }
          else {
            //Serial.println(cmdParser.getCmdParam(1));
            NumberOf50c = atoi(cmdParser.getCmdParam(1));
            if ((NumberOf50c < COIN_MIN) || (NumberOf50c > COIN_MAX)) {
              Serial.println("Anzahlfehler: 1 < Anzahl > 10");
              PiSerial.println("NACK");
            }
            else {
              //Serial.print("Auszahlung ");
              //Serial.println(NumberOf50c);
              digitalWrite(LED, HIGH);
              Payout(NumberOf50c);
              digitalWrite(LED, LOW);
              PiSerial.println("ACK");
            }
          }
        }
        else if (cmdParser.equalCommand("STATUS")) {
          Serial.println("Status ausgeben");
          PiSerial.print(COIN_MIN);
          PiSerial.print(" ");
          PiSerial.println(COIN_MAX);
          PiSerial.println("ACK");
        }
        else {
          PiSerial.println("NACK");
        }
      }
      else {
        Serial.println("Parser error!");
        PiSerial.println("NACK");
      }
    }
  //}
}

