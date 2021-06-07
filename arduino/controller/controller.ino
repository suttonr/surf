
#include <stdlib.h>
#include <EEPROM.h>

#define CONFIG_START 0x10
#define NUM_SLOTS 4
#define NUM_MOTORS 4
#define HOME_TIME 10000

// Data Types
struct config_slot {
  char name[10]="SLOT";
  byte angle=10;
  byte deploy=1;
  byte retract=10;
};

struct config {
  int time=10;
  config_slot slot[NUM_SLOTS];
};

struct motor {
  byte pinA=4;
  byte pinB=5;
  bool dir=true;
  bool run=false;
  uint16_t mills=0;
};

// Globals
motor motors[NUM_MOTORS];
config running_config;
int active_slot=0;

void initMotors() {
  motors[0].pinA=4;
  motors[0].pinB=5;
  motors[1].pinA=6;
  motors[1].pinB=7;
  motors[2].pinA=8;
  motors[2].pinB=9;
  motors[3].pinA=10;
  motors[4].pinB=11;

  for (int i=0;i<NUM_MOTORS;i++){
    pinMode(motors[i].pinA,OUTPUT);
    pinMode(motors[i].pinB,OUTPUT);
  }
}

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

void updateMotor(int m){
  if (motors[m].run) {
    if (motors[m].dir) {
      digitalWrite(motors[m].pinA,LOW);
      digitalWrite(motors[m].pinB,HIGH);
    } else {
      digitalWrite(motors[m].pinA,HIGH);
      digitalWrite(motors[m].pinB,LOW);   
    }
  } else {
    digitalWrite(motors[m].pinA,LOW);
    digitalWrite(motors[m].pinB,LOW);
  }
}

void homeMotors() {
  for (int i=0;i<NUM_MOTORS;i++){
    motors[i].mills=HOME_TIME;
    motors[i].dir=true;
    motors[i].run=true;
    updateMotor(i);
  }
}

SIGNAL(TIMER0_COMPA_vect) {
  for (int i=0;i<NUM_MOTORS;i++){
    if (( motors[i].run ) && (motors[i].mills > 0 )) {
      motors[i].mills -= 1;
    } else if (( motors[i].run ) && (motors[i].mills == 0 )) {
      motors[i].run = false;
      updateMotor(i);
      Serial.print("> Motor Stop ");
      Serial.println(i);
    }
  }
}

void setup() {
  // Setup Timer
  OCR0A = 0x01;
  TIMSK0 |= _BV(OCIE0A);
  // put your setup code here, to run once:
  Serial.begin(115200);
  initMotors();
  homeMotors();
}

void writeName(String s){
  int slot = s.substring(0,1).toInt();
  Serial.print(">> slot write ");
  Serial.println(s.substring(1));        
  s.substring(2).toCharArray(running_config.slot[slot].name,10);
}

void printSlot(int slot){
  char outs[80];
  sprintf(outs, "S:%01d:%03d:%03d:%03d:%s",
    slot,
    running_config.slot[slot].angle,
    running_config.slot[slot].deploy,
    running_config.slot[slot].retract,
    running_config.slot[slot].name
  );
  Serial.println(outs);
}

void printMotor(int m){
  char outs[80];
  sprintf(outs, "M:%01d:%01d:%01d:%05d",
    m,
    motors[m].run,
    motors[m].dir,
    motors[m].mills
  );
  Serial.println(outs);
}

void loop() {
  // put your main code here, to run repeatedly:
  int s[] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  String inString;
   if (Serial.available() > 0) {
    // get incoming string:
    inString = Serial.readString();
    inString.trim();
    Serial.print("> ");
    Serial.println(inString[0]);
    switch (inString[0]) {
      case 'p':
        Serial.println(">> print slots");
        for (int i=0;i<NUM_SLOTS;i++) {
          Serial.print(">> ");
          printSlot(i);
        }
        break;
      case 'l':
        Serial.println(">> print motors");
        for (int i=0;i<NUM_MOTORS;i++) {
          printMotor(i);
        }
        break;
      case 'i':
        resetConfig();
        Serial.println(">> config reset");
        Serial.println(running_config.slot[0].name);
        break;
      case 'r':
        Serial.println(">> read config");
        readConfig();
        break;
      case 'w':
        Serial.println(">> write config");
        writeConfig();
        break;
      case 'h':
        Serial.println(">> home motors");
        homeMotors();
        break;
      case 'm':
        int m = inString.substring(1,2).toInt();
        int d = inString.substring(2,3).toInt();
        Serial.print(">> start motor ");
        Serial.print(m);
        Serial.print(" ");
        motors[m].mills=inString.substring(3).toInt();
        motors[m].dir= ( d > 0 );
        motors[m].run=true;
        Serial.println(motors[m].mills);
        break;
      case 's':
        int slot = inString.substring(1,2).toInt();
        int angle = inString.substring(2,5).toInt();
        int deploy = inString.substring(5,8).toInt();
        int retract = inString.substring(8,11).toInt();
        char outs[80];
        sprintf(outs,"s:%x a:%x d:%x r:%x",slot,angle,deploy,retract);
        Serial.println(outs);
        if ((slot < NUM_SLOTS) && (inString.length()>11)) {
          if ((angle>0)&&(deploy>0)&&(retract>0)) {
            Serial.print(">> slot write ");
            Serial.println(inString.substring(2));
            running_config.slot[slot].angle = angle;
            running_config.slot[slot].deploy = deploy;
            running_config.slot[slot].retract = retract;
            inString.substring(11).toCharArray(running_config.slot[slot].name,10);
          }
        }
        break;
      default:
        Serial.print("> default ");
        Serial.println(inString);
        break;
    }
   }
}
