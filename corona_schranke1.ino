// include the library code:
#include <LiquidCrystal.h>

// Const declaration
const int RED_PIN_IN = 6;
const int GREEN_PIN_IN = 9;
const int RED_PIN_OUT = 11;
const int GREEN_PIN_OUT = 10;
const int BUZZER = A3;
const int SENSOR_IN = 8;
const int SENSOR_OUT = 7;
const int BUTTON = A1;
const int POTENTIOMETER = A0;

const int EMPTY = 0;
const int START_ENTERING = 8;
const int ENTERING = 2;
const int END_ENTERING = 3;
const int START_LEAVING = 4;
const int LEAVING = 5;
const int END_LEAVING = 6;

// size of the array and sensor measurements
const int MY_SIZE = 5;

// Global Variable declaration
bool sensorIn = false;
bool sensorOut = false;
int inside = 0;
int outside = 0;
int total = 0;
int state = EMPTY;
int inSensorDistance;
int outSensorDistance;
int sensorValue = 0;
bool isStartProgramm = true;
bool buttonPress = false;
bool sensorError = false;
int triggerDistance = 0;
int distance = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 13, 5, 4, 3, 2);

// Function declaration
int checkStatus(int state);
void RGB_color(int pin1, int pin2, int red, int green);
long getDistance(int pin1, int pin2);
void setLEDColorByState(int state);
void writeLCD();
void sensorSetup();
bool checkButton();
int readPotentiometerInput();
void error();
bool checkSensorError(int pin1, int pin2);


// Setup
void setup() {
  // set up RBG Led In
  pinMode(RED_PIN_IN, OUTPUT);
  pinMode(GREEN_PIN_IN, OUTPUT);
  // set up RGB Led Out
  pinMode(RED_PIN_OUT, OUTPUT);
  pinMode(GREEN_PIN_OUT, OUTPUT);
  // set up piezo
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(BUTTON, INPUT_PULLUP);  
  pinMode(POTENTIOMETER, INPUT);
}

// Programm Loop
void loop() { 
  if (isStartProgramm) sensorSetup();
  setLEDColorByState(state);
  inSensorDistance = getDistance(SENSOR_IN, SENSOR_IN);
  sensorIn = inSensorDistance < triggerDistance ? true : false;
  outSensorDistance = getDistance(SENSOR_OUT, SENSOR_OUT);
  sensorOut = outSensorDistance < triggerDistance ? true : false;
  state = checkState(state);
  total = inside - outside;
  writeLCD(); 
}

// Functions definitions
int checkState(int state) {
  switch(state){
    case EMPTY:
    	if(sensorIn) state = START_ENTERING;
    	if(sensorOut) state = START_LEAVING;
    	break;
    case START_ENTERING:
    	if(!sensorIn) state = EMPTY;
    	if(sensorOut) state =  ENTERING;
    	break;
    case ENTERING:
    	if(!sensorIn) state = END_ENTERING;
    	if(!sensorOut) state = START_ENTERING;
    	break;
    case END_ENTERING:
    	if(sensorIn) state = ENTERING;
    	if(!sensorOut) {
          inside++;
          buzzerIn();
          state = EMPTY;
    	}
    	break;
    case START_LEAVING:
    	if(!sensorOut) state = EMPTY;
    	if(sensorIn) state = LEAVING;
    	break;
    case LEAVING:
    	if(!sensorOut) state = END_LEAVING;
    	if(!sensorIn) state = START_LEAVING;
    	break;
    case END_LEAVING:
    	if(sensorOut) state = LEAVING;
    	if(!sensorIn) {
      		outside++;
      		buzzerOut();
      		state = EMPTY;
      		break;
    	}
  }
  return state;
}

//sets the LEDs to green or red
void RGB_color(int redPin, int greenPin, int redValue, int greenValue)
 {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
}

// returns the measured distance of the sensor
long getDistance(int trigger, int echo)
{
  int measurements[MY_SIZE];
// Number of measurements using the constant My Size
  for(int i = 0; i < MY_SIZE; i++) {
    long distance;
  	pinMode(trigger, OUTPUT);
  	digitalWrite(trigger, LOW);
  	delayMicroseconds(2);
  	digitalWrite(trigger, HIGH);
  	delayMicroseconds(10);
  	digitalWrite(trigger, LOW);
  	pinMode(echo, INPUT);
  	distance = pulseIn(echo, HIGH);
  	measurements[i] = distance * 0.01723;
    if(measurements[i] > 335){
      error();
      sensorError = true;
    }
  }
  // Sorts the array 
  int holder = 0;
  for(int x = 0; x < MY_SIZE; x++){
    for(int y = 0; y < MY_SIZE -1; y++){
     if(measurements[y] > measurements[y+1]) {
       holder = measurements[y+1];
       measurements[y+1] = measurements[y];
       measurements[y] = holder;
     }
    }
  }
  // Calculates the average without the largest and smallest measurement
  long average = 0;
  for(int i = 1; i < MY_SIZE - 1; i++) {
    average += measurements[i];
  }
  return average / 3;
}

// Sets the LEDs to the desired color using the state
void setLEDColorByState(int state) {
  if (state == EMPTY) {
    RGB_color(RED_PIN_OUT,GREEN_PIN_OUT,0,255); // Green
    RGB_color(RED_PIN_IN, GREEN_PIN_IN, 0, 255); // Green
  }
  else if (state == START_ENTERING) {
    RGB_color(RED_PIN_OUT,GREEN_PIN_OUT,0,255); // Green
    RGB_color(RED_PIN_IN, GREEN_PIN_IN, 255, 0); // Red
  }
  else if (state == START_LEAVING) {
    RGB_color(RED_PIN_OUT,GREEN_PIN_OUT,255,0); // Red
    RGB_color(RED_PIN_IN, GREEN_PIN_IN, 0,255); // Green
  }
  else {
    RGB_color(RED_PIN_OUT,GREEN_PIN_OUT,255, 0); // Red
    RGB_color(RED_PIN_IN, GREEN_PIN_IN, 255, 0); // Red
  }
}

void buzzerIn() {
  tone(BUZZER, 600, 100);
  delay(200);
  tone(BUZZER, 1200, 100);
  delay(200);
}

void buzzerOut(){
  tone(BUZZER, 1200, 100); 
  delay(200);           
  tone(BUZZER, 600, 100); 
  delay(200);          
}

// Writes the message to the LCD display
void writeLCD() {
  lcd.setCursor(0,0);
  lcd.print("Aktuell: ");
  lcd.print(total);
  lcd.setCursor(0, 1);
  lcd.print("Rein:");
  lcd.print(inside);
  lcd.setCursor(8,1);
  lcd.print("Raus:");
  lcd.print(outside);
}
void sensorSetup() {
  buttonPress = false;
  while(!buttonPress) {
    buttonPress = checkButton();
    triggerDistance = readPotentiometerInput();
    lcd.setCursor(0,0);
    lcd.print("Sensor ausloesen");
    lcd.setCursor(0,1);
    lcd.print("bei ");
    lcd.print(triggerDistance);
    lcd.print(" cm");
    delay(20);
  }
  lcd.clear();
  writeLCD();
  isStartProgramm = false;
}
bool checkButton() {
  return digitalRead(BUTTON) == HIGH ? false : true;
}
int readPotentiometerInput() {
  // read the input on analog pin A0:
  sensorValue = analogRead(A0);
  sensorValue = sensorValue / 3.2;
  if (sensorValue <=0) sensorValue += 5;
  delay(10); // Delay a little bit to improve simulation performance
  return sensorValue;
}
void error() {
    lcd.clear();
  	lcd.setCursor(0,0);
  	lcd.print("ERROR ! Sensor");
  	lcd.setCursor(0,1);
  	lcd.print("Distanz zu weit");
  while(sensorError) {
    sensorError = checkSensorError(SENSOR_IN, SENSOR_IN);
    if(!sensorError) sensorError = checkSensorError(SENSOR_OUT, SENSOR_OUT);
  }
  lcd.clear();
}
bool checkSensorError(int trigger, int echo){
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  pinMode(echo, INPUT);
  distance = pulseIn(echo, HIGH);
  distance *= 0.01723;
  return distance < 335 ? false : true;
}

