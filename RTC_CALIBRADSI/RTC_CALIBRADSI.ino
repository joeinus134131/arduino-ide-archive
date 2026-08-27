/*
 * kalibrasi untuk rtc d3231
 * by :  ajang rahmad modified by : made agus andi gunawan
 * 
 */
#include "DS3231_Simple.h"

DS3231_Simple Clock;

void setup() 
{
  Serial.begin(9600);
  Clock.begin();
}

void loop()
{ 
  DateTime waktu;
  waktu.Day    = 6;       //atur tanggal
  waktu.Month  = 5;      //atur bulan
  waktu.Year   = 20;      //atur tahun
  waktu.Hour   = 8;      //atur jam
  waktu.Minute = 48;      //atur menit
  waktu.Second = 30;      //atur detik
  Clock.write(waktu);
  
  Serial.print("Waktu telah di atur ke: ");
  Clock.printTo(Serial);
  Serial.println();
  
  Serial.print("Program berakhir (RESET untuk menjalakan lagi)");
  while(1);
}