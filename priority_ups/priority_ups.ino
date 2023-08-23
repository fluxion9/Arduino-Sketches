#define l1 11
#define l2 12
#define l3 9
#define l4 10
#define l5 8

#define DC_in A5

#define invEN 3
#define changeOverPin 13

#define defaultPriority 1

#define Grid 0
#define Inverter 1

int priority = defaultPriority;

byte loads[5] = {l1, l2, l3, l4, l5};
bool load_states[5] = {1, 1, 1, 1, 1};

String Buffer = "", data = "", mem = "";

unsigned long lastSendTime = 0;

struct priority_ups
{
    void init()
    {
        Serial.begin(9600);
        for (int i = 0; i < 5; i++)
        {
            pinMode(loads[i], 1);
        }
        pinMode(changeOverPin, 1);
        pinMode(invEN, 1);
        pinMode(DC_in, 0);
        changeOverTo(Grid);

        Buffer.reserve(64);
        data.reserve(64);
        mem.reserve(64);
    }

    void changeOverTo(bool input)
    {
        digitalWrite(changeOverPin, input);
    }

    float measureVoltageDC(byte pin, float vdr)
    {
        float value = analogRead(pin);
        value = (value * 5.0) / 1023.0;
        value = value * vdr;
        return value;
    }

    bool gridActive()
    {
        return measureVoltageDC(DC_in, 11.0) >= 10.0 ? true : false;
    }

    bool isListData(String *data)
    {
        if (data->startsWith("[") && data->endsWith("]"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    String readStrList(String *memory, String strList, byte position)
    {
        byte index = 0;
        *memory = "";
        for (int i = 0; i < strList.length(); i++)
        {
            if (strList[i] == ',')
            {
                index++;
            }
            if (index == position - 1)
            {
                memory->concat(strList[i]);
            }
        }
        if (memory->startsWith(","))
        {
            *memory = memory->substring(memory->indexOf(',') + 1);
        }
        return *memory;
    }

    void checkSerial()
    {
        if (Serial.available())
        {
            while (Serial.available() > 0)
            {
                delay(3);
                char c = Serial.read();
                data += c;
            }
        }
        if (data.length() > 0)
        {
            data.trim();
            if (isListData(&data))
            {
                data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
                String command = readStrList(&mem, data, 1);
                if (command == "spr")
                {
                    int val = readStrList(&mem, data, 2).toInt();
                    val = constrain(val, 1, 5);
                    priority = val;
                }
                else if (command == "onoff")
                {
                    int val = readStrList(&mem, data, 2).toInt();
                    val = constrain(val, 1, 5);
                    if(load_states[val - 1])
                    {
                      load_states[val - 1] = 0;
                    }
                    else {
                      load_states[val - 1] = 1;
                    }
                }
            }
        }
        data = "";
    }

    void sendData()
    {
        if (millis() - lastSendTime > 1500)
        {
            load_buffer();
            Serial.println(Buffer);
            lastSendTime = millis();
        }
    }

    void startInveter()
    {
        digitalWrite(invEN, 0);
    }

    void stopInveter()
    {
        digitalWrite(invEN, 1);
    }

    void load_buffer(void)
    {
        Buffer = "";
        Buffer.concat("{\"l1\":");
        Buffer.concat(load_states[0]);
        Buffer.concat(",\"l2\":");
        Buffer.concat(load_states[1]);
        Buffer.concat(",\"l3\":");
        Buffer.concat(load_states[2]);
        Buffer.concat(",\"l4\":");
        Buffer.concat(load_states[3]);
        Buffer.concat(",\"l5\":");
        Buffer.concat(load_states[4]);
        Buffer.concat(",\"pr\":");
        Buffer.concat(priority);
        Buffer.concat(",\"gatv\":");
        Buffer.concat(gridActive());
        Buffer.concat("}");
    }

    void run()
    {
        checkSerial();
        sendData();
        if(gridActive())
        {
            changeOverTo(Grid);
            for(int i = 0; i < 5; i++)
            {
                digitalWrite(loads[i], load_states[i]);
            }
            stopInveter();
        }
        else{
            startInveter();
            for(int i = 0; i < 5; i++)
            {
                digitalWrite(loads[i], 0);
            }
            changeOverTo(Inverter);
            digitalWrite(loads[priority - 1], 1);
        }
    }
}ups;

void setup()
{
  ups.init();
}

void loop()
{
  ups.run();
}
