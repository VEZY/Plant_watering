//Including the Telegram library for communication: 
#include "CTBot.h"

// ------- Telegram config --------
String ssid  = "xxxxx"      ; // Wifi name
String pass  = "xxxxx"; // Wifi Password
String token = "xxxxxx:xxxxxxxxxxxxxxxxxxxxxxx"   ; // Telegram bot token
int chatID = -000000000; // This the ID of our group chat for the plants

// Instantiate a bot object:
CTBot myBot;
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

#define interval 1000 // Interval between readings

// Define pins: 
#define relay 13
#define soil_moisture  A0
#define tank_floater  16

// Floater in the water tank: 
int water_level = 2;
bool is_level_change= false;
int prev_water_level= 2; // To keep trace of the water level from the previous loop execution

// Soil Moisture sensor: 
int moisture= 0; // Initializing moisture to 0
int moisture_perc= 0;
const int AirValue = 780;   // Driest value -> air value
const int WaterValue = 280;  // Wettest value -> water value

void setup() {
  // Set up the connexion to see the values from the moisture sensor:
  Serial.begin(9600);
  // Set the pin modes: 
  pinMode(relay, OUTPUT);
  pinMode(tank_floater, INPUT_PULLUP);
  
  Serial.println("Starting TelegramBot...");
  
  // connect the ESP8266 to the desired access point
  myBot.wifiConnect(ssid, pass);

  // set the telegram bot token
  myBot.setTelegramToken(token);
  
  // check if all things are ok
  if (myBot.testConnection()){
    Serial.println("\nTelegram connection test: OK");
  } else {
    Serial.println("\nTelegram connection test: problem");
  }
}

void loop() {
  // Read the input soil moisture on analog pin A0:
  moisture = analogRead(soil_moisture);
  moisture_perc = map(moisture,WaterValue,AirValue,100,0);
  
  // Read the input from the float sensor in the water tank:
  water_level= digitalRead(tank_floater);
  TBMessage msg; // Store telegram message

  // myBot.getNewMessage(msg);   
  if (water_level == prev_water_level) {
    is_level_change = false;
  } else {
    is_level_change = true;
  }

  if (water_level == HIGH) {
    // If there is water in the tank...
    // Print the moisture to the console: 
    Serial.print((String)"Soil Moisture : "+moisture_perc+"%"+"("+moisture+")"+"\n");

    if (is_level_change) {
      // Sending a message via Telegram (only if it wasn't said already, and it didn't change just before)
      if ((millis() > Bot_lasttime + Bot_mtbs))  {
        myBot.sendMessage(chatID, "Thank you for refilling the water tank !");
        Bot_lasttime = millis();
      }
    }
    if (moisture_perc <= 30) {
      // ...and if the soil is dry, turn on the water pump
      digitalWrite(relay, HIGH);
    } else {
      // ...and if the soil is wet, do nothing
      digitalWrite(relay, LOW);
    }
  } else {
    // If there is no water in the tank, problem !!
    // Powering down the pump in case it was running:
    digitalWrite(relay, LOW);
    Serial.print((String)"Water tank empty. Please refill with water !"+"(soil Moisture : "+moisture_perc+"%)"+"\n");

    // Sending a message via Telegram (only if it wasn't said already, and it didn't change just before)
    if ((millis() > Bot_lasttime + Bot_mtbs)&&is_level_change)  {
      myBot.sendMessage(chatID, "The water tank is empty, please refill before all plants die !");
      Bot_lasttime = millis();
    }
  }

  prev_water_level= water_level;
  delay(interval);    
}
