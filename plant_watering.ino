#define relay 13
#define interval 1000 // Interval between readings
#define soil_moisture  A0
#define tank_floater  16

// Floater in the water tank: 
int water_level = 2;

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
}

void loop() {
   // Read the input soil moisture on analog pin A0:
   moisture = analogRead(soil_moisture);
   moisture_perc = map(moisture,WaterValue,AirValue,100,0);

   // Read the input from the float sensor in the water tank:
   water_level= digitalRead(tank_floater);

   if (water_level == HIGH) {
       // If there is water in the tank...
       // Print the moisture to the console: 
       Serial.print((String)"Soil Moisture : "+moisture_perc+"%"+"("+moisture+")"+"\n");

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
   }
   
   delay(interval);    
}
