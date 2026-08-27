// Servo Jog Tester - buat cek posisi 0 derajat & rentang aman tiap servo
// SEBELUM dipasang ke linkage/lengan (test lepas dulu, horn belum dikaitkan)
//
// Cara pakai lewat Serial Monitor (baud 9600, line ending bebas):
//   1-7   = pilih servo yang mau dites (lihat menu di Serial Monitor)
//   +     = naik 1 derajat
//   -     = turun 1 derajat
//   0     = jalan PELAN menuju 0 derajat, 1 derajat per langkah
//   9     = jalan PELAN menuju 90 derajat (posisi tengah/aman)
//   x     = STOP DARURAT -> servo langsung di-detach (power motor terputus, bebas diputar tangan)
//   ?     = tampilkan menu lagi
//
// PENTING: kirim 'x' SEGERA begitu kerasa/kedengeran servo mulai berat atau ada bunyi aneh,
// jangan tunggu sampai mentok total.

#include <Servo.h>

Servo testServo;

int servoPins[] = { 3, 4, 5, 6, 9, 10, 11 };
String servoNames[] = { "Root", "Arm A1", "Arm A2", "Arm B", "Wrist A", "Wrist B", "Gripper" };

int activeIndex = -1;
int currentAngle = 90;
bool attached = false;

int stepDelay = 200; // ms jeda tiap 1 derajat waktu gerak otomatis (command 0 / 9) - naikin kalau mau lebih pelan lagi

void setup()
{
  Serial.begin(9600);
  delay(300);
  printMenu();
}

void loop()
{
  if (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == '\n' || c == '\r') return;
    handleCommand(c);
  }
}

void printMenu()
{
  Serial.println();
  Serial.println("===== SERVO TESTER (1 servo aktif per waktu) =====");
  for (int i = 0; i < 7; i++)
  {
    Serial.print(i + 1);
    Serial.print(" = ");
    Serial.print(servoNames[i]);
    Serial.print(" (pin ");
    Serial.print(servoPins[i]);
    Serial.println(")");
  }
  Serial.println("Ketik 1-7 buat pilih servo.");
  Serial.println("+ / -  = nudge 1 derajat naik/turun");
  Serial.println("0      = jalan pelan ke 0 derajat (step per step)");
  Serial.println("9      = jalan pelan ke 90 derajat (posisi tengah/aman)");
  Serial.println("x      = STOP + DETACH sekarang juga (pakai kalau berat/mentok!)");
  Serial.println("?      = tampilkan menu ini lagi");
  Serial.println("===================================================");
}

void handleCommand(char c)
{
  if (c >= '1' && c <= '7')
  {
    selectServo(c - '1');
    return;
  }

  if (c == '?')
  {
    printMenu();
    return;
  }

  if (activeIndex == -1)
  {
    Serial.println("Pilih servo dulu (ketik 1-7).");
    return;
  }

  if (c == 'x' || c == 'X')
  {
    emergencyStop();
    return;
  }

  if (!attached)
  {
    Serial.println("Servo lagi ke-detach. Ketik ulang nomor servonya (1-7) buat attach lagi.");
    return;
  }

  if (c == '+')
  {
    stepTo(currentAngle + 1);
  }
  else if (c == '-')
  {
    stepTo(currentAngle - 1);
  }
  else if (c == '0')
  {
    Serial.println("Jalan pelan menuju 0 derajat... kirim 'x' kapan aja buat stop paksa!");
    slowMoveTo(0);
  }
  else if (c == '9')
  {
    Serial.println("Jalan pelan menuju 90 derajat... kirim 'x' kapan aja buat stop paksa!");
    slowMoveTo(90);
  }
}

void selectServo(int index)
{
  if (attached)
  {
    testServo.detach(); // lepas servo lama dulu, biar cuma 1 yang aktif/dapet power
    attached = false;
  }

  activeIndex = index;
  currentAngle = 90; // asumsi posisi tengah/pabrik, aman buat titik awal
  testServo.attach(servoPins[activeIndex]);
  testServo.write(currentAngle);
  attached = true;

  Serial.print("-> Servo aktif: ");
  Serial.print(servoNames[activeIndex]);
  Serial.print(" (pin ");
  Serial.print(servoPins[activeIndex]);
  Serial.println(")");
  Serial.print("Posisi sekarang: ");
  Serial.println(currentAngle);
  Serial.println("Perhatikan dulu fisiknya sebelum lanjut gerak!");
}

void stepTo(int target)
{
  target = constrain(target, 0, 180);
  currentAngle = target;
  testServo.write(currentAngle);
  Serial.print("Posisi: ");
  Serial.println(currentAngle);
}

void slowMoveTo(int target)
{
  target = constrain(target, 0, 180);
  int dir = (target > currentAngle) ? 1 : -1;

  while (currentAngle != target)
  {
    // Cek kalau ada command stop darurat masuk di tengah gerakan
    if (Serial.available() > 0)
    {
      char c = Serial.read();
      if (c == 'x' || c == 'X')
      {
        emergencyStop();
        return;
      }
    }

    currentAngle += dir;
    testServo.write(currentAngle);
    Serial.print("Posisi: ");
    Serial.println(currentAngle);
    delay(stepDelay);
  }

  Serial.println("Sampai tujuan.");
}

void emergencyStop()
{
  testServo.detach(); // detach = motor gak dapet sinyal PWM lagi, langsung berhenti ngedorong
  attached = false;
  Serial.print("STOP! Servo ");
  Serial.print(servoNames[activeIndex]);
  Serial.println(" di-detach (bebas power, sekarang aman diputar pakai tangan).");
  Serial.println("Ketik ulang nomor servo (1-7) buat lanjut tes lagi.");
}