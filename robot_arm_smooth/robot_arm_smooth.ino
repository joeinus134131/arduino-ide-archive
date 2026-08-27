// Robot Arm - Smooth Movement + Pick & Place Sequence
// Modified: Non-blocking + Interpolated Movement + Accurate Step Completion
//
// KONTROL MANUAL: kirim "90,45,30,60,90,120\n" (6 nilai, pisah koma)
// JALANKAN PICK & PLACE OTOMATIS: kirim "PICK\n"
// HENTIKAN SEQUENCE: kirim "STOP\n"

#include <Servo.h>

Servo servo[7];
int servoPins[] = { 3, 4, 5, 6, 9, 10, 11 };
String servoNames[7] = { "Root", "Arm A1", "Arm A2", "Arm B", "Wrist A", "Wrist B", "Gripper" };

/*
Pin 3  = Root
Pin 4  = Arm A1
Pin 5  = Arm A2 (mirror dari A1)
Pin 6  = Arm B
Pin 9  = Wrist A
Pin 10 = Wrist B
Pin 11 = Gripper
*/

int currentAngle[7]; // posisi servo saat ini
int targetAngle[7];  // posisi tujuan

// Batas aman tiap servo (safety limits) - SESUAIKAN dengan mekanik lenganmu!
int servoMin[7] = { 0,   0,   0,   0,   0,   0,  10 };
int servoMax[7] = { 180, 180, 180, 180, 180, 180, 170 };

// Parameter kehalusan & kecepatan gerak
int stepSize = 1;                  // derajat per langkah (1-2 derajat untuk gerakan halus)
unsigned long stepInterval = 15;   // ms antar langkah (makin besar makin pelan)
unsigned long lastStepTime = 0;

// Parameter log monitoring ke Serial
unsigned long logInterval = 500;   // ms antar cetak status
unsigned long lastLogTime = 0;

// Buffer serial non-blocking
String inputBuffer = "";
bool commandReady = false;

// ===================== PICK & PLACE SEQUENCE =====================

const int GRIPPER_OPEN = 40;   // sudut gripper terbuka - SESUAIKAN
const int GRIPPER_CLOSE = 120; // sudut gripper menjepit - SESUAIKAN

struct ArmPose
{
  int root, armA1, armB, wristA, wristB, gripper;
  unsigned long holdDuration; // ms jeda tahan SETELAH posisi tercapai sebelum lanjut ke step berikutnya
};

// Waypoint sequence
ArmPose pickSequence[] = {
  {  90, 130, 150, 90, 90, GRIPPER_OPEN,  1000 }, // 0: posisi home
  {  90,  80, 100, 90, 90, GRIPPER_OPEN,  1000 }, // 1: bergerak ke atas objek
  {  45,  30, 100, 40, 90, GRIPPER_OPEN,   800 }, // 2: turun mendekati objek
  {  45,  30, 100, 40,  0, GRIPPER_CLOSE,  800 }, // 3: jepit objek
  {  90,  30, 100,  0,  0, GRIPPER_CLOSE,  800 }, // 4: bawa ke atas objek
  {  90,  30, 100, 90,  0, GRIPPER_CLOSE, 1000 }, // 5: bawa ke posisi drop
  {  90,  90, 100, 90, 90, GRIPPER_CLOSE,  800 }, // 6: persiapan lepas
  {  90, 130, 150, 90, 90, GRIPPER_OPEN,   800 }, // 7: kembali home
};
const int numSteps = sizeof(pickSequence) / sizeof(pickSequence[0]);

bool sequenceRunning = false;
int sequenceIndex = 0;
bool poseReached = false;
unsigned long reachedTime = 0;

// ===================================================================

void setup()
{
  Serial.begin(9600);
  inputBuffer.reserve(64);

  // Inisialisasi posisi awal (Home: 90 derajat kecuali A2 mirror)
  for (int i = 0; i < 7; i++)
  {
    currentAngle[i] = 90;
    targetAngle[i] = 90;
    servo[i].attach(servoPins[i]);
    servo[i].write(currentAngle[i]);
  }

  Serial.println("=================================================");
  Serial.println("ROBOT ARM READY");
  Serial.println("Kirim 'PICK' untuk menjalankan sequence otomatis");
  Serial.println("Kirim 'STOP' untuk menghentikan gerakan");
  Serial.println("Kirim 'root,armA1,armB,wristA,wristB,gripper' untuk manual");
  Serial.println("=================================================");
}

void loop()
{
  readSerial();
  if (commandReady)
  {
    parseCommand();
    commandReady = false;
  }

  updateServos();    // gerakkan servo halus menuju target
  runSequence();     // jalankan step sequence otomatis jika aktif
  logStatus();       // cetak status tiap servo ke Serial Monitor secara berkala
}

// Cek apakah semua servo sudah sampai di targetAngle
bool isAllServoReached()
{
  for (int i = 0; i < 7; i++)
  {
    if (currentAngle[i] != targetAngle[i])
    {
      return false;
    }
  }
  return true;
}

// Baca serial karakter per karakter, non-blocking
void readSerial()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == '\n' || c == '\r')
    {
      if (inputBuffer.length() > 0)
      {
        commandReady = true;
        break;
      }
    }
    else
    {
      inputBuffer += c;
    }
  }
}

void parseCommand()
{
  String cmd = inputBuffer;
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "PICK")
  {
    startSequence();
    inputBuffer = "";
    return;
  }

  if (cmd == "STOP")
  {
    stopSequence();
    inputBuffer = "";
    return;
  }

  // Kalau bukan perintah khusus, anggap sebagai data manual "a,b,c,d,e,f"
  int values[6];
  int idx = 0;
  int startPos = 0;

  for (int i = 0; i <= (int)inputBuffer.length(); i++)
  {
    if (i == (int)inputBuffer.length() || inputBuffer.charAt(i) == ',')
    {
      if (idx < 6)
      {
        values[idx] = inputBuffer.substring(startPos, i).toInt();
        idx++;
      }
      startPos = i + 1;
    }
  }

  if (idx == 6)
  {
    stopSequence(); // kontrol manual membatalkan sequence otomatis yang sedang jalan
    setTargetPose(values[0], values[1], values[2], values[3], values[4], values[5]);
    Serial.println("-> Target Pose Manual Diterapkan");
  }
  else
  {
    Serial.print("Perintah tidak dikenal: ");
    Serial.println(inputBuffer);
  }

  inputBuffer = "";
}

// Terapkan satu pose ke targetAngle[] dengan mirror Arm A2 & batas aman
void setTargetPose(int root, int armA1, int armB, int wristA, int wristB, int gripper)
{
  targetAngle[0] = constrain(root, servoMin[0], servoMax[0]);         // Root
  targetAngle[1] = constrain(armA1, servoMin[1], servoMax[1]);        // Arm A1
  targetAngle[2] = constrain(180 - armA1, servoMin[2], servoMax[2]);  // Arm A2 (mirror)
  targetAngle[3] = constrain(armB, servoMin[3], servoMax[3]);         // Arm B
  targetAngle[4] = constrain(wristA, servoMin[4], servoMax[4]);       // Wrist A
  targetAngle[5] = constrain(wristB, servoMin[5], servoMax[5]);       // Wrist B
  targetAngle[6] = constrain(gripper, servoMin[6], servoMax[6]);      // Gripper
}

void startSequence()
{
  sequenceRunning = true;
  sequenceIndex = 0;
  loadStep(sequenceIndex);
  Serial.println(">>> Sequence dimulai: PICK & PLACE <<<");
}

void stopSequence()
{
  if (sequenceRunning) Serial.println(">>> Sequence DIHENTIKAN <<<");
  sequenceRunning = false;
}

void loadStep(int i)
{
  ArmPose p = pickSequence[i];
  setTargetPose(p.root, p.armA1, p.armB, p.wristA, p.wristB, p.gripper);
  poseReached = false;

  Serial.print("[Step ");
  Serial.print(i);
  Serial.print("/");
  Serial.print(numSteps - 1);
  Serial.println("] Menuju target pose...");
}

// Jalankan step demi step sequence secara teratur dan presisi (non-blocking)
void runSequence()
{
  if (!sequenceRunning) return;

  // Cek apakah semua servo sudah tiba di target sudut
  if (!poseReached)
  {
    if (isAllServoReached())
    {
      poseReached = true;
      reachedTime = millis();
      Serial.print("[Step ");
      Serial.print(sequenceIndex);
      Serial.println("] Target pose tercapai! Menahan posisi...");
    }
  }
  else
  {
    // Jika sudah tercapai, tahan selama holdDuration sebelum lanjut ke step berikutnya
    if (millis() - reachedTime >= pickSequence[sequenceIndex].holdDuration)
    {
      sequenceIndex++;
      if (sequenceIndex >= numSteps)
      {
        sequenceRunning = false;
        Serial.println(">>> SEQUENCE SELESAI SEMPURNA <<<");
      }
      else
      {
        loadStep(sequenceIndex);
      }
    }
  }
}

// Gerakkan tiap servo selangkah demi selangkah menuju target (non-blocking)
void updateServos()
{
  unsigned long now = millis();
  if (now - lastStepTime < stepInterval) return;
  lastStepTime = now;

  for (int i = 0; i < 7; i++)
  {
    if (currentAngle[i] < targetAngle[i])
    {
      currentAngle[i] = min(currentAngle[i] + stepSize, targetAngle[i]);
      servo[i].write(currentAngle[i]);
    }
    else if (currentAngle[i] > targetAngle[i])
    {
      currentAngle[i] = max(currentAngle[i] - stepSize, targetAngle[i]);
      servo[i].write(currentAngle[i]);
    }
  }
}

// Cetak status tiap servo secara berkala ke Serial Monitor
void logStatus()
{
  unsigned long now = millis();
  if (now - lastLogTime < logInterval) return;
  lastLogTime = now;

  bool anyMoving = !isAllServoReached();

  if (sequenceRunning || anyMoving)
  {
    Serial.print("Status: ");
    for (int i = 0; i < 7; i++)
    {
      Serial.print(servoNames[i]);
      Serial.print(":");
      Serial.print(currentAngle[i]);
      Serial.print("/");
      Serial.print(targetAngle[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}
