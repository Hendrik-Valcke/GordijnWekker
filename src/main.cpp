//references
/*
https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
https://diyi0t.com/building-a-menu-with-buttons-for-your-microcontroller/

*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// put function declarations here:
void displayTime(int );
void buttonPressed();
void spinMotor(bool);
void brakeMotor();
//timer
const int startTime = 12*60*60;
const int alarmDuration = 0.5*60;//0.5 minutes
int alarmTime;
//OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//menu buttons
const int button0Pin = 34; // GPIO pin connected to the button
bool button0Flag = false;
const int button1Pin = 35; // GPIO pin connected to the button
bool button1Flag = false;
const int button2Pin = 32; // GPIO pin connected to the button
bool button2Flag = false;
const int button3Pin = 33; // GPIO pin connected to the button
bool button3Flag = false;

//motorpins
int motorBrake = 17;
int motorEnable= 18;
int motorPhase = 16;
int motorMode = 19;
// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 0;//enable is active low so lower dutycycle (0-255) means more power

//states
enum MachineState {
  CLOCK,
  ALARM,
  TEST
};
MachineState currentState;
enum EditState {
  NOT,
  HOUR,
  MINUTE
};
EditState currentEditState;

void setup() {
  //Motor pins:
  pinMode(motorBrake, OUTPUT);
  pinMode(motorEnable, OUTPUT);
  pinMode(motorMode, OUTPUT);
  pinMode(motorPhase, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(motorEnable, pwmChannel);

  //Menu buttons
  pinMode(button0Pin, INPUT_PULLUP); // Set button pin as input with pull-up resistor
  attachInterrupt(digitalPinToInterrupt(button0Pin), buttonPressed, FALLING); // Attach interrupt on falling edge
  pinMode(button1Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1Pin), buttonPressed, FALLING);
  pinMode(button2Pin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(button2Pin), buttonPressed, FALLING); 
  pinMode(button3Pin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(button3Pin), buttonPressed, FALLING); 
  //menu
  currentState = CLOCK;
  currentEditState = NOT;
  //limit switches 

  //OLED display
  Serial.begin(115200); 
  Serial.println("Start setup");

  //timer
  alarmTime = startTime + alarmDuration;  
  printf("alarmtime= %d\n",alarmTime);

  //display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
  Serial.println("SSD1306 allocation failed");
  for(;;); // Don't proceed, loop forever
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("Hello, world!");
  display.display();
  delay(1000);
  display.println("Hello, world2!");
  display.display();
/*
  brakeMotor();
  delay(1000);
  spinMotor(true);
  delay(2000);
  brakeMotor();
  delay(1000);
  spinMotor(false);
  delay(2000);
  brakeMotor();
  */
}
  
void loop() {
  int currentTime = startTime+millis()/ 1000; // Convert milliseconds to seconds
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  switch (currentState) {
    case CLOCK://just display time
      if(currentEditState==NOT)
      {
        display.println("Not editing");
        display.println("Press UP to edit");
        displayTime(currentTime);
      }
      else if(currentEditState){
        display.println("Editing hours");
        display.println("Press Right to edit minutes");
        displayTime(currentTime);
      }
      display.clearDisplay();
      displayTime(currentTime);
      if (button0Flag) {
        Serial.println("button0");
        button0Flag = false;
      }
      if (button1Flag) {
        Serial.println("button1"); 
        button1Flag = false;
      }
      if (button2Flag) {
        Serial.println("button2");
        button2Flag = false;
      }
      if (button3Flag) {
        Serial.println("button3");
        button3Flag = false;
        
      }
      break;

    case ALARM:
      // Code for the EDIT state
      break;

    case TEST:
      // Code for the DEFAULT state
      break;
  }
  
  /*
  //displayTime(currentTime);
  // Check if the current time has reached the alarm time
  if (currentTime >= alarmTime) {
    // Execute your function here
    Serial.println("Alarm triggered! Executing function...");

    // Reset the alarm time for the next day
    alarmTime += 86400; // 24 hours in seconds
  }
  //read buttons
  if (button0Flag) {
    // Handle button press in the main loop
    
    
    // Reset the flag
    button0Flag = false;
  }
  //ToDo write functions for other 3 buttons
  */
 display.display();
}

// put function definitions here:
void displayTime(int currentTime) {
  
  /*display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);*/

  int hours = (currentTime / 3600)%24;
  int minutes = (currentTime % 3600) / 60;
  int seconds =(currentTime %60);
  display.print("Time: ");
  display.print(hours);
  display.print(":");
  if (minutes < 10) display.print("0");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) display.print("0");
  display.print(seconds);
  
  /*
  display.print("\nAlarm at: ");
  display.print(alarmTime/3600);
  display.print(":");
  display.print((alarmTime % 3600) / 60);
  display.print(":");
  display.print(alarmTime % 60);
*/
  
  //display.display();
}



void spinMotor(bool forwards){
  if (forwards){
    Serial.println("Moving Forward");
    digitalWrite(motorBrake, HIGH);
    digitalWrite(motorPhase, HIGH);
    digitalWrite(motorMode, HIGH);
    //pwm
    ledcWrite(pwmChannel, dutyCycle);
  }else{
    Serial.println("Moving Forward");
    digitalWrite(motorBrake, HIGH);
    digitalWrite(motorPhase, LOW);
    digitalWrite(motorMode, HIGH);
    //pwm
    ledcWrite(pwmChannel, dutyCycle);
  }

}

void brakeMotor(){
  Serial.println("Stopping motor");
    digitalWrite(motorBrake, LOW);
}

void buttonPressed() {
  int buttonNumber = -1;

  if (digitalRead(button0Pin) == LOW) {
    button0Flag=true;
  } else if (digitalRead(button1Pin) == LOW) {
    button1Flag=true;
  } else if (digitalRead(button2Pin) == LOW) {
    button2Flag=true;
  } else if (digitalRead(button3Pin) == LOW) {
    button3Flag=true;
  }
}
  
