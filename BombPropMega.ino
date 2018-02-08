//Created by Justin Nguyen
//For use by Texas Panic Room
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
//#include <NewTone.h>
#include <IRremote.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <RHDatagram.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

int normalRate = 1000;
int penaltyRate = normalRate / 5;
int delayRate = normalRate;
int initialRate = 30000;
int penaltyTxRate = 1000;
int transferRate = initialRate;
uint16_t initial = 6000;
uint16_t timer = initial;
const byte wire0 = 23;
const byte wire1 = 25;
const byte wire2 = 27;
const byte wire3 = 29;
const byte wire4 = 31;
const byte wire5 = 33;
const byte wire6 = 35;
const byte wire7 = 37;
const byte RECV_PIN = 53;
const byte buzzer = 41;
const byte signallight = A14;
const byte maglock = A13;
const byte RedLED = 43;
const byte GreenLED = 47;
const byte ppbutton = 2;
const byte rxPin = 11;
const byte txPin = 12;
const byte pttPin = 10;
int penaltyCounter = 0;
bool flag0;
bool flag1;
bool flag2;
bool flag3;
bool flag4;
bool flag5;
bool flag6;
bool flag7;
bool correct0 = false;
bool correct1 = false;
bool correct2 = false;
bool penalty = false;
bool paused = true;
bool deductionFlag = false;
bool lose = false;
bool* flags[8];
bool programming = false;
bool escaped = false;
int flagsCap = 8;
int digit = 0;
int digitPlace = 0;
int finalTime = 0;
unsigned long lastInterrupt = 0;
unsigned long lastSecond = 0;
unsigned long lastTransmit = 0;
unsigned long lastPenalty = 0;
int penaltyDelay = 2000;
bool pp;
bool ppprime = false;
uint8_t from = 0;
uint8_t to = 0;
uint8_t id = 0;
uint8_t bits = 0;
Adafruit_7segment matrix = Adafruit_7segment();
IRrecv irrecv(RECV_PIN);

decode_results results;

RH_ASK driver;
RHDatagram RHD = RHDatagram(driver, 0);

struct dataStruct{
  uint16_t timer;
  bool paused;
  bool penalty;
  unsigned long results;
}myData;

byte tx_buf[sizeof(myData)] = {0};

void setup() {
#ifndef __AVR_ATtiny85__
  Serial.begin(9600);
  Serial.println("Oval Office");
#endif
  /*if (!RHD.init())
    Serial.println("init failed");
  Serial.print("This address is: ");
  Serial.println(RHD.thisAddress());*/
  matrix.begin(0x70);
  pinMode(wire0, INPUT);
  pinMode(wire1, INPUT);
  pinMode(wire2, INPUT);
  pinMode(wire3, INPUT);
  pinMode(wire4, INPUT);
  pinMode(wire5, INPUT);
  pinMode(wire6, INPUT);
  pinMode(wire7, INPUT);
  pinMode(signallight, OUTPUT);
  pinMode(maglock, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ppbutton, INPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(ppbutton), pausePlay, RISING);
  flags[0] = &flag0;
  flags[1] = &flag1;
  flags[2] = &flag2;
  flags[3] = &flag3;
  flags[4] = &flag4;
  flags[5] = &flag5;
  flags[6] = &flag6;
  flags[7] = &flag7;
  digitalWrite(signallight, HIGH);
  digitalWrite(buzzer, LOW);
  digitalWrite(RedLED, HIGH);
  digitalWrite(GreenLED, LOW);
  /*myData.timer = timer;
  myData.paused = paused;
  myData.penalty = penalty;
  transmit();
  lastTransmit = millis();*/
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  irrecv.blink13(true);
}

void loop() {
  /*receive();
  if (myData.results > 0){
    Serial.println(myData.results, DEC);
    remoteController(myData.results);
  }*/
  
  if (irrecv.decode(&results)) {
    Serial.println(results.value, DEC);
    irrecv.resume(); // Receive the next value
    remoteController(results.value);
  }
  
  readValues();
  //debug();

  if (programming) {
    programBomb();
  }

  penalty = false;
  checkSetFlags();
  checkWinLose();

  if (!paused) { //if not paused
     checkPenalty();
     
    if (millis() - lastSecond >= delayRate) {
      updateTimer();
      lastSecond = millis();
    }
    if (timer <= 0 || timer > 10000) {
      paused = true;
      lose = true;
    }
    //delay(delayRate);
  }
  else { // if it is paused
    if (!flag0 && !flag1 && !flag2 && !flag3 && !flag4 && !flag5 && !flag6 && !flag7) {
      programmingPrompt();
    }
    else if (timer <= 0 || timer > 10000) {
      matrix.writeDigitNum(0, 0);
      matrix.writeDigitNum(1, 0);
      matrix.writeDigitNum(3, 0);
      matrix.writeDigitNum(4, 0);
      /*myData.timer = timer;
      myData.paused = paused;
      myData.penalty = penalty;
      transmit();
      lastTransmit = millis();*/
    }
    else {
      matrix.print(timer, DEC);
    }
    penalty = false;
    matrix.drawColon(true);
    matrix.blinkRate(0);
    matrix.writeDisplay();
    //delay(1000);
  }
}

void reset() {
  timer = initial;
  correct0 = false;
  correct1 = false;
  correct2 = false;
  paused = true;
  penalty = false;
  penaltyCounter = 0;
  deductionFlag = false;
  lose = false;
  digitalWrite(maglock, HIGH);
  digitalWrite(signallight, HIGH);
  digitalWrite(buzzer, LOW);
  digitalWrite(RedLED, HIGH);
  digitalWrite(GreenLED, LOW);
  escaped = false;
  /*myData.timer = timer;
  myData.paused = paused;
  transmit();
  lastTransmit = millis();*/
}

void pausePlay() {
  paused = !paused;
  if (!paused) {
    for (int a = 0; a < 3; a++) {
      digitalWrite(signallight, LOW);
      delay(500);
      digitalWrite(signallight, HIGH);
      delay(500);
    }
    lastSecond = millis();
  }
  else {
    digitalWrite(signallight, LOW);
    delay(1000);
    digitalWrite(signallight, HIGH);
  }
  /*myData.timer = timer;
  myData.paused = paused;
  transmit();
  lastTransmit = millis();*/
}

void setTimer() {
  boolean finished = false;
  matrix.print(0xFEED, HEX);
  matrix.writeDisplay();
  for (int a = 0; a < 2; a++) {
    digitalWrite(signallight, LOW);
    delay(500);
    digitalWrite(signallight, HIGH);
    delay(500);
  }
  results.value = 0;
  while (!finished) {
    if (irrecv.decode(&results)) {
      Serial.println(results.value, DEC);
      irrecv.resume(); // Receive the next value
    }

    switch (results.value) {
      case 551520375:
        processTimer(1);
        results.value = 0;
        break;
      case 551504055:
        processTimer(2);
        results.value = 0;
        break;
      case 551536695:
        processTimer(3);
        results.value = 0;
        break;
      case 551495895:
        processTimer(4);
        results.value = 0;
        break;
      case 551528535:
        processTimer(5);
        results.value = 0;
        break;
      case 551512215:
        processTimer(6);
        results.value = 0;
        break;
      case 551544855:
        processTimer(7);
        results.value = 0;
        break;
      case 551491815:
        processTimer(8);
        results.value = 0;
        break;
      case 551524455:
        processTimer(9);
        results.value = 0;
        break;
      case 551487735:
        processTimer(0);
        results.value = 0;
        break;
      case 551522415:
        finished = true;
        digitalWrite(signallight, LOW);
        delay(500);
        digitalWrite(signallight, HIGH);
        delay(500);
        results.value = 0;
        break;
      default:
        results.value = 0;
    }
  }
  timer = finalTime;
}

void processTimer (int x) {
  if (digitPlace >= 4) {
    digitPlace = 0;
    finalTime = 0;
  }
  if (digitPlace == 0) {
    finalTime += (x * 1000);
    digitPlace++;
  }
  else if (digitPlace == 1) {
    finalTime += (x * 100);
    digitPlace++;
  }
  else if (digitPlace == 2) {
    if (x >= 6) {
      x = 5;
    }
    finalTime += (x * 10);
    digitPlace++;
  }
  else if (digitPlace == 3) {
    finalTime += x;
    digitPlace++;
  }
  matrix.print(finalTime, DEC);
  matrix.drawColon(true);
  matrix.writeDisplay();
}

void remoteController(unsigned long command) {
  switch (command) {
    case 551522415: //  Pause/Play
      pausePlay();
      results.value = 0;
      break;
    case 551534655: //Subtract Time
      timer -= 100;
      results.value = 0;
      break;
    case 551502015: //Add Time
      timer += 100;
      results.value = 0;
      break;
    case 11815:   //Reset
      reset();
      digitalWrite(signallight, LOW);
      delay(500);
      digitalWrite(signallight, HIGH);
      delay(500);
      results.value = 0;
      break;
    case 551531085: //Set Time
      setTimer();
      results.value = 0;
      break;
    default:
      results.value = 0;
  }
}

void debug() {
  Serial.print("Correct0 is ");
  Serial.println(correct0);
  Serial.print("Correct1 is ");
  Serial.println(correct1);
  Serial.print("Correct2 is ");
  Serial.println(correct2);
  Serial.print("Wire0 is ");
  Serial.println(flag0);
  Serial.print("Wire1 is ");
  Serial.println(flag1);
  Serial.print("Wire2 is ");
  Serial.println(flag2);
  Serial.print("Wire3 is ");
  Serial.println(flag3);
  Serial.print("Wire4 is ");
  Serial.println(flag4);
  Serial.print("Wire5 is ");
  Serial.println(flag5);
  Serial.print("Wire6 is ");
  Serial.println(flag6);
  Serial.print("Wire7 is ");
  Serial.println(flag7);
}

void checkSetFlags(){
  for (int i = 0; i < flagsCap; i++) {
    if (i <= 1 || i == 4 || i == 7) {
      if (!*flags[i]) {
        penalty = true;
        /*myData.timer = timer;
        myData.paused = paused;
        myData.penalty = penalty;
        transmit();
        lastTransmit = millis();*/
        break;
      }
    }
    if (!*flags[i] && i == 3 && !penalty) {
      correct0 = true;
    }
    if (*flags[i] && i == 3) {
      correct0 = false;
    }
    if (!*flags[i] && i == 2 && !penalty && correct0) {
      correct1 = true;
    }
    else if (!*flags[i] && i == 2 && !correct0) {
      penalty = true;
      break;
    }
    if (*flags[i] && i == 2) {
      correct1 = false;
    }
    if (!*flags[i] && i == 6 && !penalty && correct1) {
      correct2 = true;
    }
    else if (!*flags[i] && i == 6 && !correct1) {
      penalty = true;
      break;
    }
    if (*flags[i] && i == 5) {
      correct2 = false;
    }
    if (!*flags[i] && i == 5 && !penalty && correct2) {
      paused = true;
      escaped = true;
      /*myData.timer = timer;
      myData.paused = paused;
      myData.penalty = penalty;
      transmit();
      lastTransmit = millis();*/
      //digitalWrite(maglock, LOW);
    }
    else if (!*flags[i] && i == 5 && !correct2) {
      penalty = true;
      break;
    }
  }
}

void checkPenalty() {
  if (penalty) {
    Serial.println("PENALTY!");
    matrix.blinkRate(1);
    delayRate = penaltyRate;
    transferRate = penaltyTxRate;
    digitalWrite(buzzer, HIGH);
    digitalWrite(RedLED, LOW);
    delay(50);
    digitalWrite(RedLED, HIGH);
    if (!deductionFlag && (millis() - lastPenalty >= penaltyDelay)) {
      if (penaltyCounter < 6) {
        penaltyCounter++;
      }
      else {
        penaltyCounter = 6;
      }
      timer = timer - ((penaltyCounter - 1) * 100);
      deductionFlag = true;
      lastPenalty = millis();
    }
  }
  else {
    deductionFlag = false;
    matrix.blinkRate(0);
    delayRate = normalRate;
    transferRate = initialRate;
    digitalWrite(buzzer, LOW);
    //noNewTone(buzzer);
  }
}

void checkWinLose() {
  if (escaped) {
    digitalWrite(maglock, LOW);
    digitalWrite(GreenLED, HIGH);
    digitalWrite(RedLED, LOW);
  }
  else {
    digitalWrite(maglock, HIGH);
    digitalWrite(GreenLED, LOW);
    digitalWrite(RedLED, HIGH);
  }

  if (lose) {
    digitalWrite(signallight, LOW);
    digitalWrite(buzzer, HIGH);
  }
}

void readValues() {
  flag0 = digitalRead(wire0);
  delay(10);
  flag1 = digitalRead(wire1);
  delay(10);
  flag2 = digitalRead(wire2);
  delay(10);
  flag3 = digitalRead(wire3);
  delay(10);
  flag4 = digitalRead(wire4);
  delay(10);
  flag5 = digitalRead(wire5);
  delay(10);
  flag6 = digitalRead(wire6);
  delay(10);
  flag7 = digitalRead(wire7);
  delay(10);
}

void updateTimer() {
  if (timer <= 0 || timer > 10000) {
    matrix.writeDigitNum(0, 0);
    matrix.writeDigitNum(1, 0);
    matrix.writeDigitNum(3, 0);
    matrix.writeDigitNum(4, 0);
  }
  else {
    matrix.print(timer, DEC);
  }
  matrix.drawColon(true);
  matrix.writeDisplay();
  
  if (timer % 100 == 0) {
    timer -= 41;
  }
  else {
    timer -= 1;
  }

  /*myData.timer = timer;
  myData.paused = paused;
  myData.penalty = penalty;
  if (millis() - lastTransmit >= transferRate){
    transmit();
    lastTransmit = millis();
  }*/
}

void programmingPrompt(){
  bool timeout = false;
  unsigned long ref = millis();
  while (!timeout) {
    if (millis() - ref > 5000) {
      timeout = true;
    }
    else {
      bool button = digitalRead(ppbutton);
      if (button) {
        programming = true;
        timeout = true;
        paused = false;
      }
    }
  }
}

void programBomb() {
  Serial.println("Entering programming mode.");
  reset();
  bool checked[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool* newFlags[8];
  digitalWrite(RedLED, HIGH);
  digitalWrite(GreenLED, HIGH);
  matrix.print(0xFADE, HEX);
  matrix.writeDisplay();
  int counter = 7;
  while (counter >= 0) {
    readValues();
    Serial.print("Counter: ");
    Serial.println(counter);
    for (int i = 0; i < flagsCap; i++) {
      if (*flags[i] && !checked[i]) {
        newFlags[counter] = flags[i];
        checked[i] = true;
        counter--;
      }
    }
  }
  flags[0] = newFlags[0];
  flags[1] = newFlags[1];
  flags[2] = newFlags[2];
  flags[3] = newFlags[3];
  flags[4] = newFlags[4];
  flags[5] = newFlags[5];
  flags[6] = newFlags[6];
  flags[7] = newFlags[7];
  programming = false;
  Serial.println("Programming Complete");
}

void transmit(){
  memcpy(tx_buf, &myData, sizeof(myData) );
  byte zize = sizeof(myData);

  RHD.sendto((uint8_t *)tx_buf, zize, RH_BROADCAST_ADDRESS);

  if(RHD.waitPacketSent()){
    Serial.println("Package Sent");
  }
  //delay(200);
}

void receive(){
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  if (RHD.available()) // Non-blocking
    {
      RHD.recvfrom(buf, &buflen, &from, &to, &id, &bits);
      memcpy(&myData, buf, sizeof(myData));
      Serial.print("Results: ");
      Serial.println(myData.results);
      //logging();
    }
}

