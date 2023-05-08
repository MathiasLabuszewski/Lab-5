#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>
//Both includes for serial and the mp3 player library
#define rPwm 4
#define lPwm 5
#define rF 17
#define rB 18
#define lF 10
#define lB 9
#define btn1 4

#define espRx 8
#define espTx 7 // not used
#define dfRx 11
#define dfTx 12

DFPlayerMini_Fast df;

SoftwareSerial espSerial(espRx, espTx); // RX, TX
SoftwareSerial dfSerial(dfRx, dfTx); // RX, TX  
  //create serials for the mp3 player and the esp32 for reciving commands.
void setup() {
  //pin setup
  pinMode(rPwm, OUTPUT);
  pinMode(lPwm, OUTPUT);
  pinMode(rF, OUTPUT);
  pinMode(rB, OUTPUT);
  pinMode(lF, OUTPUT);
  pinMode(lB, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(btn1, INPUT_PULLUP);
  analogWrite(rPwm, 255);
  analogWrite(lPwm, 255);

  Serial.begin(9600); // usb
  Serial.println("arduino on");
//setup serial
  dfSerial.begin(9600);
  espSerial.begin(9600);

  //mp3 player setup
  df.begin(dfSerial, true);
  df.EQSelect(dfplayer::EQ_NORMAL);
  df.startDAC();
}


//helper functions
void foward()
{
  digitalWrite(rF,1);
  digitalWrite(rB,0);
  digitalWrite(lF,1);
  digitalWrite(lB,0);
}

void backward()
{
  digitalWrite(rF,0);
  digitalWrite(rB,1);
  digitalWrite(lF,0);
  digitalWrite(lB,1);
}

void right()
{
  digitalWrite(rF,1);
  digitalWrite(rB,0);
  digitalWrite(lF,0);
  digitalWrite(lB,1);
}

void left()
{
  digitalWrite(rF,0);
  digitalWrite(rB,1);
  digitalWrite(lF,1);
  digitalWrite(lB,0);
}

void stop()
{
  digitalWrite(rF,0);
  digitalWrite(rB,0);
  digitalWrite(lF,0);
  digitalWrite(lB,0);
}

void speed(byte p)
{ 
  analogWrite(rPwm, (p*17));
  analogWrite(lPwm, (p*17));
}

void playSound1(byte p)
{
  int n = p-'0';
  df.playFromMP3Folder((uint16_t)n);
}

void changeVol(byte p)
{
  p *= 2;
  if (p>30){
    p = 30;
  }
 df.volume(p);
}

void xxx();

void (*functions[6])() = {xxx, foward, backward, left, right, stop}; // array of function pointers.

void (*functions2[6])(char v) = {speed, playSound1, xxx, xxx, xxx, xxx};// array of function pointers, with input args.


void loop() {
  if (espSerial.available() > 0) { // look and see if any data has been sent
    char byte1 = espSerial.read(); // read the data
    Serial.println(byte1);
    if (byte1 <= 'z' && byte1 >= 'a'){ //if the data falls in range, then a command without a value was send, so just call function
      byte cmd = byte1 & 0xF; // array index
      Serial.write(cmd);
      (*functions[cmd])(); // call function with array index
    } else if (byte1 >= '0' && byte1 <= 'Z'){ //if the data falls within this range, two bytes were sent, the second being the data.
      while (espSerial.available() == 0); // stall and make sure the other byte was recived.
      int data2 = espSerial.read(); // read the data
      byte cmd = byte1 - '0'; // get array index, need to subtract from 0, as 0 is not "0" in binary.
      (*functions2[cmd])(data2); // call array index with argumenets to be passed in, and pass in second peice of data.
    }
  }

}

