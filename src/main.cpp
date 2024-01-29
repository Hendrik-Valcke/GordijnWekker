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
int startTime = 22*60*60;
int alarmTime;
bool alarmed= false;
//OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//menu buttons, 0=UP, 1= RIGHT, 2=LEFT,3=DOWN
const int button0Pin = 34; // GPIO pin connected to the button
bool button0Flag = false;
const int button1Pin = 35; 
bool button1Flag = false;
const int button2Pin = 32; 
bool button2Flag = false;
const int button3Pin = 33; 
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
  alarmTime = startTime +60*60;  
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
  if (alarmed==true && currentTime%(24*60*60)<alarmTime%(24*60*60))
  {
    alarmed=false;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  switch (currentState) {
    case CLOCK://just display time
      if(currentEditState==NOT)
      {
        display.println("CLOCK mode");
        displayTime(currentTime);
        display.println("RIGHT to edit time");
        display.println("UP/DOWN to change modes");
        if (button0Flag) {
          Serial.println("button0");
          delay(250);
          button0Flag = false;
          currentState=TEST;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=HOUR;
        }
        if (button2Flag) {
          Serial.println("button2");
          delay(250);
          button2Flag = false;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;
          currentState=ALARM;          
        }
        break;
        
      }
      else if(currentEditState==HOUR){
        display.println("CLOCK mode");
        displayTime(currentTime); 
        display.println("Editing hours");
        display.println("RIGHT to edit minutes, LEFT to exit edit mode");
        if (button0Flag) {
          Serial.println("button0");//up
          delay(250);
          button0Flag = false;
          startTime=startTime+60*60;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=MINUTE;
        }
        if (button2Flag) {
          Serial.println("button2");//left
          delay(250);
          button2Flag = false;
          currentEditState=NOT;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;          
          if(startTime-60*60<0)
          {
            startTime=startTime+23*60*60;
          }else{
            startTime=startTime-60*60; 
          }      
        }
        break;
      }
      else if(currentEditState==MINUTE){
        display.println("CLOCK mode");
        displayTime(currentTime); 
        display.println("Editing minutes");
        display.println("Press LEFT to edit hours, RIGHT to exit edit mode");               
        if (button0Flag) {
          Serial.println("button0");//up
          delay(250);
          button0Flag = false;
          startTime=startTime+60;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=NOT;
        }
        if (button2Flag) {
          Serial.println("button2");//left
          delay(250);
          button2Flag = false;
          currentEditState=HOUR;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;          
          if(startTime-60<0)
          {
            startTime=startTime+24*60*60-60;
          }else{
            startTime=startTime-60; 
          }      
        }
        break;
      }      

    case ALARM:
      if(currentEditState==NOT)
      {
        display.println("ALARM mode");
        display.print("CLOCK: ");
        displayTime(currentTime);
        display.print("ALARM: ");
        displayTime(alarmTime);
        display.println(" ");
        display.println("RIGHT to edit alarm time");
        display.println("UP/DOWN to change modes");
        
        if (button0Flag) {
          Serial.println("button0");
          delay(250);
          button0Flag = false;
          currentState=CLOCK;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=HOUR;
        }
        if (button2Flag) {
          Serial.println("button2");
          delay(250);
          button2Flag = false;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;
          currentState=TEST;          
        }
        if((currentTime%(24*60*60))>=(alarmTime%(24*60*60))&&alarmed==false)
        {
          alarmed = true;
          spinMotor(true);
          delay(5000);
          brakeMotor();
        }
        break;
      }
      else if(currentEditState==HOUR){
        display.println("ALARM mode");
        display.print("alarm time: ");
        displayTime(alarmTime); 
        display.println("Editing hours");
        display.println("RIGHT to edit minutes, LEFT to exit editmode");
        if (button0Flag) {
          Serial.println("button0");//up
          delay(250);
          button0Flag = false;
          alarmTime=alarmTime+60*60;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=MINUTE;
        }
        if (button2Flag) {
          Serial.println("button2");//left
          delay(250);
          button2Flag = false;
          currentEditState=NOT;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;          
          if(alarmTime-60*60<0)
          {
            alarmTime=alarmTime+23*60*60;
          }else{
            alarmTime=alarmTime-60*60; 
          }      
        }
        break;
      }
      else if(currentEditState==MINUTE){
        display.println("ALARM mode");
        display.print("alarm time: ");
        displayTime(alarmTime); 
        display.println("Editing minutes");
        display.println("LEFT to edit hours, RIGHT to exit editmode");
        if (button0Flag) {
          Serial.println("button0");//up
          delay(250);
          button0Flag = false;
          alarmTime=alarmTime+60;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          currentEditState=NOT;
        }
        if (button2Flag) {
          Serial.println("button2");//left
          delay(250);
          button2Flag = false;
          currentEditState=HOUR;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;          
          if(alarmTime-60<0)
          {
            alarmTime=alarmTime+24*60*60-60;
          }else{
            alarmTime=alarmTime-60; 
          }      
        }
        break;
      }
      break;

    case TEST:
      display.println("RIGHT to close curtain");
      display.println("UP/DOWN to change modes");
      if (button0Flag) {
          Serial.println("button0");
          delay(250);
          button0Flag = false;
          currentState=ALARM;
        }
        if (button1Flag) {
          Serial.println("button1"); //right
          delay(250);
          button1Flag = false;
          spinMotor(false);
          delay(5000);
          brakeMotor();

        }
        if (button2Flag) {
          Serial.println("button2");
          delay(250);
          button2Flag = false;
        }
        if (button3Flag) {
          Serial.println("button3");//down
          delay(250);
          button3Flag = false;
          currentState=CLOCK;          
        }
        
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
  }*/
 
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
  //display.print("Time: ");
  display.print(hours);
  display.print(":");
  if (minutes < 10) display.print("0");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) display.print("0");
  display.print(seconds);
  display.println();
  
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
  
