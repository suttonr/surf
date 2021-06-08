# controller.ino
Arduino sketch intended to control 4 DC linear actuators. Has provisons for 
storing configuration values into on EEPROM. By default listens on the first
serial port at 115200 baud for commands

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

### Home Motors
Run motors to the home posistion
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
s<slot><angle><deploy><retract><name>
```
* slot - 1 digit - slot number
* angle - 3 digits - set angle
* deploy - 3 digits - speed at which to deploy
* retract - 3 digits - speed at which to retract
* name - string - Text name for the slot
#### Example
```
s1045002010USER1
```

## Print Commands
### Print Slots
Outputs all slot configuration
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

#### Example
```
M:0:1:1:00543
```

### Slot Details
Contains the configuration from a slot
```
S:<slot>:<angle>:<deploy>:<retract>:<name>
```
* slot - 1 digit - slot number
* angle - 3 digits - set angle
* deploy - 3 digits - speed at which to deploy
* retract - 3 digits - speed at which to retract
* name - string - Text name for the slot
#### Example
```
S:0:045:2:10:USER1
```