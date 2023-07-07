#include <Arduino.h>
#include <LedControl.h>

// Enable/disable input1/2
int pinOutEnable12 = 13; //  L293D pin 1
bool stateEnable12 = false;

// DC motor 1
int pinOutInput1 = 6; // L293D pin 2
int pinOutInput2 = 5; // L293D pin 7

// Joystick
int pinInnButton = 2; // Joystick SW
int pinInnXAxis = A0; // Joystick VRX
int pinInnYAxis = A1; // Joystick VRY

// Motor 1 Read
int readMotorSpeedX = 0;
int readMotorSpeedY = 0;

// Motor 1 state
int stateMotor1Direction = 0; // 0 == stop; 1 == clockwise; -1 == counter_clockwise;
int stateMotor1Speed = 0;

// Setup of LedControl for printing on MAX7219 m/8x8 LED
LedControl lc = LedControl(12, 10, 11, 1);
int mappedSpeed = 0;

//Functions
int motorDirection(int x, int y);

void motorSpeed(int direction, int x, int y);

int mapSection1(int x, int y);

int mapSection2(int x, int y);

int mapSection3(int x, int y);

int mapSection4(int x, int y);

void printSingleNumber(int digit, int plusMinus);

void setup() {
    Serial.begin(9600);

//    Enable/disable input1/2
    pinMode(pinOutEnable12, OUTPUT);

//    Enable DC motor 1
    pinMode(pinOutInput1, OUTPUT); // clockwise spin
    pinMode(pinOutInput2, OUTPUT); // counterclockwise spin

//    Joystick
    pinMode(pinInnButton, INPUT_PULLUP);
    pinMode(pinInnXAxis, INPUT);
    pinMode(pinInnYAxis, INPUT);


//    The MAX72XX is in power-saving mode on startup,
//    we have to do a wakeup call
    lc.shutdown(0, false);
//    Set the brightness to a medium values
    lc.setIntensity(0, 1);
//    and clear the display
    lc.clearDisplay(0);
}

void loop() {
//    Reads state for motor on/off
    if (digitalRead(pinInnButton) == 0) {
        stateEnable12 = !stateEnable12;
        delay(25);
    }

//    Turns motor on
    if (stateEnable12) {
        digitalWrite(pinOutEnable12, HIGH);

//        Read analog values
        readMotorSpeedX = analogRead(pinInnXAxis);
        readMotorSpeedY = analogRead(pinInnYAxis);

//        Checks for what direction motor is supposed to spin from joy stick
        stateMotor1Direction = motorDirection(readMotorSpeedX, readMotorSpeedY);
//        Gives instruction on what direction and power
        motorSpeed(stateMotor1Direction, readMotorSpeedX, readMotorSpeedY);

//        Maps the speed value to single digit
        mappedSpeed = (int) map(stateMotor1Speed, 0, 250, 0, 9);
//        Prints out the speed and direction for the motor
        printSingleNumber(mappedSpeed, stateMotor1Direction);

    } else {
        digitalWrite(pinOutEnable12, LOW);
        lc.clearDisplay(0);
    }
}

int motorDirection(int x, int y) {
//  If joystick is middle center and given it a dead-zone with +-20
//  The Joystick is badly calibrated and have custom numbers
//  Default for X == 499 and Y == 527
    if ((x <= 520 && x >= 480) && (y <= 550 && y >= 510)) {
        return 0;
    } else if (x > 511) {
        return 1;
    } else if (x < 511) {
        return -1;
    }
    return 0;
}

void motorSpeed(int direction, int x, int y) {
//    Sort X and Y for what mapping it needs
    if (x >= 511 && y >= 511)
        stateMotor1Speed = mapSection1(x, y);
    else if (x < 511 && y >= 511)
        stateMotor1Speed = mapSection2(x, y);
    else if (x < 511 && y < 511)
        stateMotor1Speed = mapSection3(x, y);
    else if (x >= 511 && y < 511)
        stateMotor1Speed = mapSection4(x, y);

//    Runs motor
    switch (direction) {
        case 1: {
            analogWrite(pinOutInput1, stateMotor1Speed);
            analogWrite(pinOutInput2, 0);
            break;
        }
        case -1: {
            analogWrite(pinOutInput1, 0);
            analogWrite(pinOutInput2, stateMotor1Speed);
            break;
        }
        default: {
            analogWrite(pinOutInput1, 0);
            analogWrite(pinOutInput2, 0);
            break;
        }
    }
}

/*
 *      Section 4 | Section 1
 *      ---------------------
 *      Section 3 | Section 2
 *
 *      This illustrates a joystick area and each of them needs it own mapping to give correct speed!
 * */
int mapSection1(int x, int y) {
    x = (int) map(x, 511, 1023, 0, 255);
    y = (int) map(y, 511, 1023, 0, 255);
    if (x > y)
        return x;
    else
        return y;
}

int mapSection2(int x, int y) {
    x = (int) map(x, 510, 0, 0, 255);
    y = (int) map(y, 511, 1023, 0, 255);
    if (x > y)
        return x;
    else
        return y;
}

int mapSection3(int x, int y) {
    x = (int) map(x, 510, 0, 0, 255);
    y = (int) map(y, 510, 0, 0, 255);
    if (x > y)
        return x;
    else
        return y;
}

int mapSection4(int x, int y) {
    x = (int) map(x, 511, 1023, 0, 255);
    y = (int) map(y, 510, 0, 0, 255);
    if (x > y)
        return x;
    else
        return y;
}

// digit must be 0-9
// plusMinus: + == 1, - == -1
void printSingleNumber(int digit, int plusMinus) {
    byte number[10][4] = {
            {B01111100, B10000010, B10000010, B01111100},
            {B00000000, B11111110, B01000000, B00100000},
            {B01100010, B10010010, B10001010, B01000110},
            {B01101100, B10010010, B10010010, B01000100},
            {B00001000, B11111110, B01001000, B00111000},
            {B10011100, B10100010, B10100010, B11100100},
            {B01001100, B10010010, B10010010, B01111100},
            {B11100000, B10010000, B10001110, B10000000},
            {B01101100, B10010010, B10010010, B01101100},
            {B01111100, B10010010, B10010010, B01100100},
    };

    byte signe[3][3] = {
            {B00010000, B00010000, B00010000}, // prints '-'
            {B00000000, B00000000, B00000000}, // prints ''
    };

    for (int i = 0; i < 4; ++i)
        lc.setRow(0, i, number[digit][i]);

    if (plusMinus == -1) {
        for (int i = 0; i < 3; ++i)
            lc.setRow(0, i + 5, signe[0][i]);

    } else {
        for (int i = 0; i < 3; ++i)
            lc.setRow(0, i + 5, signe[1][i]);
    }
}