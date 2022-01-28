#define REMOTEXY_MODE__SOFTSERIAL
#include <SoftwareSerial.h>
#include <String.h>
#include <Servo.h>
#include <RemoteXY.h>
#include <Wire.h>

#define REMOTEXY_SERIAL_RX 10
#define REMOTEXY_SERIAL_TX 11
#define REMOTEXY_SERIAL_SPEED 9600

const int LEFT_INF = 38;
const int RIGHT_INF = 40;
const int fdr = 1;
const int sdr = 2;


Servo MG995;
 
int hexadecimalToDecimal(String hexVal)
{
    int len = hexVal.length();
    int base = 1;
    int dec_val = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (hexVal[i] >= '0' && hexVal[i] <= '9') {
            dec_val += (int(hexVal[i]) - 48) * base;
            base = base * 16;
        }
        else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') {
            dec_val += (int(hexVal[i]) - 55) * base;
            base = base * 16;
        }
    }
    return dec_val;
}

void acommand(int x, String y, String z){
  Wire.begin();
  Wire.beginTransmission(1);
  Wire.write(hexadecimalToDecimal(y));
  if (z != "00"){
    Wire.write(hexadecimalToDecimal(z));
  }
  Serial.println(analogRead(A7));
  Wire.endTransmission();
  delay(10);
}
void bcommand(int x, String y, String z, String d){
  Wire.begin();
  Wire.beginTransmission(1);
  Wire.write(hexadecimalToDecimal(y));
  if (z != "00"){
    Wire.write(hexadecimalToDecimal(z));
  }
  if (d != "00"){
    Wire.write(hexadecimalToDecimal(d));
  }
  Wire.endTransmission();
  delay(10);
}

void awspeed(int c_id, String y, uint16_t speedMotor){
//выделяем первый байт скорости, накладываем маску, обнуляем последние 8 бит
  uint16_t speedFirstByte = speedMotor & 0b1111111100000000;
  speedFirstByte = speedFirstByte >> 8;
  //выделяем второй байт скорости, обнуляем первые 8 бит
  uint16_t speedSecondByte = speedMotor & 0b0000000011111111; 

  Wire.beginTransmission(c_id);
  Wire.write(hexadecimalToDecimal(y)); //Sets the speed parameter for Motor 1 in degrees per second
  Wire.write(speedFirstByte);//два байта на один мотор, это против часовой
  Wire.write(speedSecondByte);    
  Wire.endTransmission();
  delay(10);
}

void stopMotors(uint8_t c_id){
  Wire.beginTransmission(c_id);
  Wire.write(0x42);
  Wire.endTransmission();
}
// настройки соединения 



// конфигурация интерфейса  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =
  { 255,2,0,22,0,30,0,15,31,1,
  5,0,15,53,30,30,2,26,31,67,
  4,6,23,20,5,2,26,11,67,4,
  34,23,20,5,2,26,11 };
  
// структура определяет все переменные и события вашего интерфейса управления 
struct {

    // input variables
  int8_t joystick_1_x; // =-100..100 координата x положения джойстика 
  int8_t joystick_1_y; // =-100..100 координата y положения джойстика 

    // output variables
  char text_1[11];  // =строка UTF8 оканчивающаяся нулем 
  char text_2[11];  // =строка UTF8 оканчивающаяся нулем 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////



void setup() 
{
  RemoteXY_Init (); 
  pinMode(18,INPUT);
  pinMode(19,OUTPUT);
  Wire.begin();
  Wire.beginTransmission(fdr);
  Wire.write(0x25);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(sdr);
  Wire.write(0x25);
  Wire.endTransmission();
  delay(10);
  Serial.begin(9600);
 // acommand(fdr, String(42), String(79));
 // acommand(sdr, String(42), String(79));
  
}
int angle = 1;
int k = 0;
void loop() 
{ 
  RemoteXY_Handler ();
  RemoteXY.text_1[0] = RemoteXY.joystick_1_x%10;
  RemoteXY.text_2[0] = RemoteXY.joystick_1_y%10;
  if (RemoteXY.joystick_1_x > 0 and RemoteXY.joystick_1_y > 0){
    angle = atan(RemoteXY.joystick_1_x/RemoteXY.joystick_1_y)*180.0/3.14;
  }
  if (RemoteXY.joystick_1_x > 0 and RemoteXY.joystick_1_y < 0){
    angle = abs(atan(RemoteXY.joystick_1_x/RemoteXY.joystick_1_y))*180.0/3.14 + 90;
  }
  if (RemoteXY.joystick_1_x < 0 and RemoteXY.joystick_1_y > 0){
    angle = abs(atan(RemoteXY.joystick_1_x/RemoteXY.joystick_1_y))*180.0/3.14 + 270;
  }
  if (RemoteXY.joystick_1_x < 0 and RemoteXY.joystick_1_y < 0){
    angle = atan(RemoteXY.joystick_1_x/RemoteXY.joystick_1_y)*180.0/3.14 + 180;
  }
  k = map(sqrt(pow(RemoteXY.joystick_1_x, 2) + pow(RemoteXY.joystick_1_y, 2)), 0, 141.42, 0, 600);
  Serial.print(RemoteXY.joystick_1_x);
  Serial.print("   ");
  Serial.print(k);
  Serial.print("   ");
  Serial.println(RemoteXY.joystick_1_y);
  awspeed(fdr, "43", cos(2*3.14*(angle/360.0))*(-k));
  awspeed(sdr, "44", sin(2*3.14*(angle/360.0))*k);
  awspeed(sdr, "43", cos(2*3.14*(angle/360.0))*k);
  awspeed(fdr, "44", sin(2*3.14*(angle/360.0))*(-k)); 

}