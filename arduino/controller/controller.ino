
#include <stdlib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define CONFIG_START 0x10
#define NUM_SLOTS 4
#define NUM_MOTORS 4
#define HOME_TIME 4000
#define HOME_DIR true
#define NEMA_GPS
#define TERMBAUD 115200
#define GPSBAUD 9600
#define SPEEDHOLD 10

// Data Types
typedef struct config_slot {
  char name[10] = "SLOT";
  byte positions[NUM_MOTORS] = {0};
  byte deploy = 6;
  byte retract = 40;
};

typedef struct config {
  uint16_t time[NUM_MOTORS] = {23,33,33,23};
  uint8_t launch[NUM_MOTORS] = {10,10,10,10};
  config_slot slot[NUM_SLOTS];
};

typedef struct motor {
  byte pinA=4;
  byte pinB=5;
  bool dir=true;
  bool run=false;
  uint16_t mills=0;
  uint8_t position=0;
  bool homing=false;
};

// Globals
motor motors[NUM_MOTORS];
config running_config;
uint8_t active_slot=0;
uint8_t active_surf=0;
uint8_t surf_armed=0;
SoftwareSerial GPS_Serial(12,13); // RX only
TinyGPS GPS;
int speed=0;
unsigned short gps_sent=0;
uint8_t speedState[3] = {0,0,0};

void initMotors() {
  motors[0].pinA=4;
  motors[0].pinB=5;
  motors[1].pinA=6;
  motors[1].pinB=7;
  motors[2].pinA=8;
  motors[2].pinB=9;
  motors[3].pinA=10;
  motors[3].pinB=11;

  for (uint8_t i=0;i<NUM_MOTORS;i++){
    pinMode(motors[i].pinA,OUTPUT);
    pinMode(motors[i].pinB,OUTPUT);
  }
}

/* Configuation */
void writeConfig() {
  EEPROM.put(CONFIG_START, running_config);
}

void readConfig() {
  EEPROM.get(CONFIG_START, running_config);
}

void resetConfig() {
  config clean_config;
  EEPROM.put(CONFIG_START, clean_config);
  EEPROM.get(CONFIG_START, running_config);
}

void writeName(String s){
  uint8_t slot = s.substring(0,1).toInt();
  Serial.print(">> slot write ");
  Serial.println(s.substring(1));        
  s.substring(2).toCharArray(running_config.slot[slot].name,10);
}

/* Motor Control */
void updateMotor(uint8_t m){
  if (motors[m].run) {
    if (motors[m].dir) {
      digitalWrite(motors[m].pinA,LOW);
      digitalWrite(motors[m].pinB,HIGH);
    } else {
      digitalWrite(motors[m].pinB,LOW);
      digitalWrite(motors[m].pinA,HIGH);
    }
  } else {
    digitalWrite(motors[m].pinA,LOW);
    digitalWrite(motors[m].pinB,LOW);
  }
}

void runMotor(uint8_t motor, bool dir, uint16_t  mills){
    motors[motor].mills=mills;
    motors[motor].dir=dir;
    motors[motor].run=true;
    int position = 0;
    position = (int)mills / (int)running_config.time[motor];
    if (motors[motor].dir != HOME_DIR){
      position = (int)motors[motor].position + (int)position;
    } else {
      position = (int)motors[motor].position - (int)position;
    }

    if (position > 100){
      motors[motor].position = 100;
    } else if (position < 0) {
      motors[motor].position = 0;
    } else {
      motors[motor].position = abs(position);
    }
    updateMotor(motor);
    printMotor(motor);
}

void runMotor(uint8_t motor, uint8_t position){ 
    if ((position >= 0) && (position <= 100)){
      runMotor( motor, ( motors[motor].position > position ), 
        abs(motors[motor].position - position) * running_config.time[motor]);
      motors[motor].position = position;
    }
}

void homeMotors() {
  for (uint8_t i=0;i<NUM_MOTORS;i++){
    motors[i].homing=true;
    runMotor(i,HOME_DIR,HOME_TIME);
  }
  active_surf=0;
}

/* Surf Control */
void surf(bool left){
  for (uint8_t m=0;m<NUM_MOTORS;m++){
    if ( !left && (m<(NUM_MOTORS/2)) ){
      runMotor(m,running_config.slot[active_slot].positions[m]);
      active_surf=1;
    } else if ( left && (m>=(NUM_MOTORS/2)) ){
      runMotor(m,running_config.slot[active_slot].positions[m]);
      active_surf=2;
    } else {
      motors[m].homing=true;
      runMotor(m,HOME_DIR,HOME_TIME);
    }
  }
}

void incrmentState(uint8_t s){
  for (uint8_t i=0;i<3;i++){
    if ((i==s) && (speedState[s] < 255)){
      speedState[i] = speedState[i] + 1;
    } else if (i!=s){
      speedState[i] = 0;
    }
  }
}

void checkSpeed(int s){
  if ((s > running_config.slot[active_slot].deploy) && 
      (s < running_config.slot[active_slot].retract)){
       incrmentState(1);
  } else if (s < running_config.slot[active_slot].deploy) {
    incrmentState(0);
  } else {
    incrmentState(2);
  }

  if ((active_surf == 0 ) && (surf_armed == 1) && (speedState[1] > 30)){
    Serial.println(">> gpssurf left");
    surf(true);
  } else if ((active_surf == 0 ) && (surf_armed == 2) && (speedState[1] > 30)){
    Serial.println(">> gpssurf right");
    surf(false);
  } else if ( (active_surf == 1 ) && 
    (surf_armed > 0) && 
    ((speedState[0] > SPEEDHOLD) || (speedState[2] > SPEEDHOLD)) ){
      Serial.println(">> gpssurf retract");
      homeMotors();
      active_surf=0;
      surf_armed=0;
  }
}

/* Timer Interupt (once per ms) */
SIGNAL(TIMER0_COMPA_vect) {
  for (uint8_t i=0;i<NUM_MOTORS;i++){
    if (( motors[i].run ) && (motors[i].mills > 0 )) {
      motors[i].mills -= 1;
    } else if (( motors[i].run ) && (motors[i].mills == 0 )) {
      motors[i].run = false;
      if (motors[i].homing){
        motors[i].homing = false;
        motors[i].position = 0;
      }
      updateMotor(i);
      printMotor(i);
    }
  }
}

/* Print */
void printConfig(){
  char outs[40];
  sprintf(outs, "C:0");
  for (uint8_t m=0;m<NUM_MOTORS;m++){
    sprintf(outs,"%s:%03d", outs, running_config.time[m]);
  }
  Serial.println(outs);
}

void printStatus(){
  char outs[40];

  sprintf(outs,"X:%03d,%03d,%03d,%03d,%03d",
    speed,speedState[0],speedState[1],speedState[2],gps_sent);
  Serial.println(outs);

  sprintf(outs,"Y:%03d,%03d,%03d",
    active_surf,surf_armed,active_slot);
  Serial.println(outs);
}

void printSlot(uint8_t slot){
  char outs[40];
  sprintf(outs, "S:%01d",slot);
  for (uint8_t m=0;m<NUM_MOTORS;m++){
    sprintf(outs,"%s:%02d", outs, running_config.slot[slot].positions[m]);
  }
  sprintf(outs,"%s:%02d:%02d:%s", outs,
    running_config.slot[slot].deploy,
    running_config.slot[slot].retract,
    running_config.slot[slot].name
  );
  Serial.println(outs);
}

void printMotor(uint8_t m){
  char outs[20];
  sprintf(outs, "M:%01d:%01d:%01d:%05d:%03d",
    m,
    motors[m].run,
    motors[m].dir,
    motors[m].mills,
    motors[m].position
  );
  Serial.println(outs);
}

/* Micro Init */
void setup() {
  // Setup Timer
  OCR0A = 0x01;
  TIMSK0 |= _BV(OCIE0A);

  Serial.begin(TERMBAUD);
  Serial.println(">> Booting");
  GPS_Serial.begin(GPSBAUD);
  initMotors();
  delay(10);
  readConfig();
  homeMotors();
  incrmentState(3);
  Serial.println(">> Setup Complete, homing motors");

}

/* Main program loop */
void loop() {
  String inString;
  int new_speed;
  int new_sent;
  unsigned long chars = 0;
  unsigned short sentences = 0, failed = 0;

  while (GPS_Serial.available()) {
    GPS.encode(GPS_Serial.read());
  }
  new_speed = GPS.f_speed_kmph();
  GPS.stats(&chars,&sentences,&failed);
  if (gps_sent != sentences) {
    gps_sent = sentences;
    checkSpeed(new_speed);
    if (new_speed != speed) {
      speed = new_speed;
      printStatus();
    }
  }

  if (Serial.available() > 0) {
    // get incoming string:
    inString = Serial.readString();
    inString.trim();
    Serial.print("> ");
    Serial.println(inString[0]);

      if (inString[0]=='p'){
        Serial.println(">> print config");
        printConfig();
        printStatus();
        Serial.println(">> print slots");
        for (uint8_t i=0;i<NUM_SLOTS;i++) {
          printSlot(i);
        }
        Serial.println(">> print motors");
        for (uint8_t i=0;i<NUM_MOTORS;i++) {
          printMotor(i);
        }
      } else if (inString[0]=='i') {
        resetConfig();
        Serial.println(">> config reset");
      } else if (inString[0]=='r') {
        Serial.println(">> read config");
        readConfig();
      } else if (inString[0]=='w') {
        Serial.println(">> write config");
        writeConfig();
      } else if (inString[0]=='h') {
        Serial.println(">> home motors");
        homeMotors();
      } else if (inString[0]=='m') {
        uint8_t m = inString.substring(1,2).toInt();
        uint8_t d = inString.substring(2,3).toInt();
        runMotor(m,( d > 0 ),inString.substring(3).toInt());
      } else if (inString[0]=='n') {
        uint8_t m = inString.substring(1,2).toInt();
        uint8_t pos = inString.substring(2).toInt();
        if ((m < NUM_MOTORS) && (pos >= 0) && (pos <= 100) ){
          runMotor(m,pos);
        }
      } else if (inString[0]=='c') {
         Serial.println(">> config slot");
         unsigned int strptr=1;
         uint8_t slot;
         slot = inString.substring(1,2).toInt();
         byte positions[NUM_MOTORS] = {0};
         for ( uint8_t a=0;a<NUM_MOTORS;a++ ) {
           positions[a] = inString.substring(a*2+2,(a+1)*2+2).toInt();
           strptr = (a+1)*2+2;
         }
         uint8_t deploy = 5;
         deploy = (uint8_t)inString.substring(strptr,strptr+2).toInt();
         strptr += 2;
         uint8_t retract = inString.substring(strptr,strptr+2).toInt();
         strptr += 2;
         if ((slot < NUM_SLOTS) && (inString.length()>strptr)) {
           if ((positions[0]>0)&&(deploy>0)&&(retract>0)) {
             Serial.print(">> slot write ");
             Serial.println(inString.substring(2));
             memcpy(running_config.slot[slot].positions, positions, sizeof(positions[0])*NUM_MOTORS);
             running_config.slot[slot].deploy = deploy;
             running_config.slot[slot].retract = retract;
             inString.substring(strptr).toCharArray(running_config.slot[slot].name,strptr);
           }
         }
      } else if (inString[0]=='s') {
        surf_armed = 0;
        if ( inString.substring(1,2)[0] == 'l' ){
          Serial.println(">> surf left");
          surf(true);
        } else if ( inString.substring(1,2)[0] == 'r' ){
          Serial.println(">> surf right");
          surf(false);
        } 
      } else if (inString[0]=='a') {
        if ( inString.substring(1,2)[0] == 'l' ){
          Serial.println(">> armed left");
          surf_armed = 1;
        } else if ( inString.substring(1,2)[0] == 'r' ){
          Serial.println(">> armed right");
          surf_armed = 2;
        } else if ( inString.substring(1,2)[0] == 'c' ){
          Serial.println(">> disarmed");
          surf_armed = 0;
        } 
      } else if (inString[0]=='t') {
         unsigned int strptr=1;
         uint8_t slot;
         slot = inString.substring(1,2).toInt();
         uint16_t positions[NUM_MOTORS] = {0};
         for ( uint8_t a=0;a<NUM_MOTORS;a++ ) {
           positions[a] = inString.substring(a*3+2,(a+1)*3+2).toInt();
           strptr = (a+1)*3+2;
         }
         if ((slot < NUM_SLOTS) && (inString.length()>=strptr)) {
           if ( positions[0]>0 ) {
             Serial.print(">> time write ");
             Serial.println(inString.substring(2));
             memcpy(running_config.time, positions, sizeof(positions[0])*NUM_MOTORS);
           }
         }
      } else if (inString[0]=='l') {
         unsigned int strptr=1;
         uint8_t slot;
         slot = inString.substring(1,2).toInt();
         uint16_t positions[NUM_MOTORS] = {0};
         for ( uint8_t a=0;a<NUM_MOTORS;a++ ) {
           positions[a] = inString.substring(a*3+2,(a+1)*3+2).toInt();
           strptr = (a+1)*3+2;
         }
         if ((slot < NUM_SLOTS) && (inString.length()>=strptr)) {
           if ( positions[0]>0 ) {
             Serial.print(">> launch write ");
             Serial.println(inString.substring(2));
             memcpy(running_config.launch, positions, sizeof(positions[0])*NUM_MOTORS);
           }
         }
      } else {
        Serial.print("> default ");
        Serial.println(inString);
      } 
   }
}
