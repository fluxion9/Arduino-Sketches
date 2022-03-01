//123456
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
SoftwareSerial serial(5, 6);
#define cs 10
String buff;
void setup() {
  serial.begin(9600);
  if (!SD.begin(cs)) {
    serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  serial.println("card initialized.");

}

void loop() {
  if(serial.available())
  {
    char c = serial.read();
    if (c == '1')
    {
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        while(dataFile.available())
        {
          serial.write(dataFile.read());
        }
      }
      dataFile.close();
    }
    else if(c == '2')
    {
      
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        serial.print("Position is: ");
        serial.println(dataFile.position());
      }
      dataFile.close();
    }
    else if(c == '3')
    {
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        int pos = 0;
        while(dataFile.seek(pos))
        {
          serial.print("Position ");
          serial.print(pos);
          serial.println(" available.");
          pos += 1;
        }
      }
      dataFile.close();
    }
    else if(c == '4')
    {
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        serial.print("data.txt has a size of: ");
        serial.println(dataFile.size());
      }
      dataFile.close();
    }
    else if(c == '5')
    {
      buff = "";
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        byte lines = 0;
        while(dataFile.available())
        {
          char d = dataFile.read();
          if(d == '\n')
          {
            lines++;
          }
          if(lines == 2)
          {
            buff += d;
          }
        }
        serial.print("On line 3, i got: ");
        serial.println(buff);
      }
      dataFile.close();
    }
    else if(c == '6')
    {
      File dataFile = SD.open("data.txt", FILE_READ);
      if(dataFile)
      {
        byte lines = 0;
        while(dataFile.available())
        {
          char d = dataFile.read();
          if(d == '\n')
          {
            lines++;
          }
        }
        serial.print("data.txt file has ");
        serial.print(lines);
        serial.println(" lines.");
      }
      dataFile.close();
    }
    else
    {
      ;
    }
  }
}
