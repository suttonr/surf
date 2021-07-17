> :warning: **Work in Progress**: Be very careful here!
# controller.ino
Arduino sketch intended to control 4 DC linear actuators via a relay H-BRIDGE. Has provisons for 
storing configuration values into on EEPROM. By default listens on the first
serial port at 115200 baud for commands

# Circuit
## Motor
Depending on relay choice addiontal level shifting may be needed between coils and arduino.
```
                                     Vm
                                      |
                                      |
                                      |
                      +---------------+--------------+
                      | NO                        NO |
Motor.pinA-----++--+  +-+                          +-+ +--++------Motor.pinB
               ++c |  +-+         +-------+        +-+ |c ++
    +           |o |              |       |            |o |
                |i |       +------+   M   +-------+    |i |
               ++l |  +-+--+      |       |       ++-+ |l ++
               ++--+  +-+         +-------+        +-+ +--++
               |      | NC                        NC |     |
               |      |                              |     |
               |      |                              |     |
               |      |                              |     |
               |      |                              |     |
               +------+---------------+--------------+-----+
                                      |
                                      |
                                     GND
```

# Commands
Commands all start with a lowercase alpha character

### Run Motor
Runs motor in a direction for an ammount of time
```
m<motor><direction><millis>
```
* Motor - 1 digit - Motor index
* Direction - 1 digit - Motor Direction, 1=Forward 0=Backwards
* Millis - 5 digits - Amount of time in ms to run motor
#### Example
Run motor 1, forward for 1s
```
m1101000
```

### Run Motor to Position
Runs motor to a specific position
```
m<motor><position>
```
* Motor - 1 digit - Motor index
* Position - 3 digits - 0-100 indicition percentage of range to move to
#### Example
Run motor 1, to 20%
```
m1020
```

### Home Motors
Run motors to the home posistion, and reset position to 0
```
h
```

## Configuration
### Reset config to defaults
Write the default configuration to EEPROM then reads it into running config
```
i
```

### Read Configuration from EEPROM
Read config in EEPROM into running config
```
r
```

### Write Running Config to EEPROM
Write running configuration to EEPROM
```
w
```

### Configure Slot
write a slot configuration to running config
```
c<slot><m0><m1><m2><m3><deploy><retract><name>
```
* slot - 1 digit - slot number
* m0 position - 2 digits - position to set m0 to
* m1 position - 2 digits - position to set m1 to
* m2 position - 2 digits - position to set m2 to
* m3 position - 2 digits - position to set m3 to
* deploy - 2 digits - speed at which to deploy
* retract - 2 digits - speed at which to retract
* name - string - Text name for the slot
#### Example
```
s1112233440120USER1
```

### Configure Timings
write timing config to running config
```
t0<m0><m1><m2><m3>
```
* slot - 1 digit - slot number
* m0 time (ms) - 3 digits - (ms) per % of movement
* m1 time (ms) - 3 digits - (ms) per % of movement
* m2 time (ms) - 3 digits - (ms) per % of movement
* m3 time (ms) - 3 digits - (ms) per % of movement

#### Example
```
t0100100100100
```

## Print Commands
### Print Slots
Outputs all configuration and motor status
```
p
```

### Print Motor status
Outputs the status of all motors
```
l
```

# Output
Serial output is intended to be easily parsed. Detail/Debugging output is 
always preceeded with `>`.  All control output will start with an uppercase
alpha ascii character. 

### Motor Status
Indicates the status of a motor
```
M:<motor>:<run>:<direction>:<mills>
```
* Motor -  1 digit - Motor Index
* Run - 1 digit - Motor status, 1=Running 0=Stoped
* Direction - 1 digit - Motor Direction, 1=Forward 0=Backwards
* Mills - 5 digits - Number of millisecconds the motor will run for, or runtime remaining
* Position - 3 digits - Position of motor after current movement is complete

#### Example
```
M:0:1:1:00543:050
```

### Slot Details
Contains the configuration from a slot
```
S:<slot>:<m0>:<m1>:<m2>:<m3>:<deploy>:<retract>:<name>
```
* slot - 1 digit - slot number
* m0 position - 2 digits - position to set m0 to
* m1 position - 2 digits - position to set m1 to
* m2 position - 2 digits - position to set m2 to
* m3 position - 2 digits - position to set m3 to
* deploy - 2 digits - speed at which to deploy
* retract - 2 digits - speed at which to retract
* name - string - Text name for the slot
#### Example
```
S:0:11:22:33:44:02:10:USER1
```