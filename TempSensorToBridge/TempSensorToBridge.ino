#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

 float Celcius=0;
 float Fahrenheit=0;
void setup(void)
{
  
  Serial.begin(9600);
  sensors.begin();
}

void loop(void)
{ 
  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  
  /*Serial.print(" C  ");
  Serial.print(Celcius);
  Serial.print(" F  ");
  Serial.println(Fahrenheit);
  delay(1000);
  */
  
  Serial.print("#"); Serial.write(13);Serial.write(10);  // begin
  Serial.print(3); Serial.write(13);Serial.write(10);  // 3 fields
  Serial.print(14); Serial.write(13);Serial.write(10); // id sensor
  Serial.print(Celcius); Serial.write(13);Serial.write(10); // temp
  Serial.print(0); Serial.write(13);Serial.write(10);  // not used
  Serial.print("$"); Serial.write(13);Serial.write(10); // terminator
  delay(10000);  
}

