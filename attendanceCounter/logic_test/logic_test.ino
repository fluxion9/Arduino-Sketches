#define trig0 6
#define echo0 5
#define trig1 10
#define echo1 11

#define rst 10


struct attC
{
  bool state[2] = {0, 0};
  float threshold = 100.0; //cm
  float waitTime = 5.0; //second(s)
  unsigned long millisNow, dMillis = 0;
  int count;

  void Init( void )
  {
    Serial.begin(115200);
    count = 0;
    Serial.println("Device Initialized..");

  }

  bool ping( byte trig, byte echo )
  {
    pinMode( trig, 1 );
    pinMode( echo, 0 );
    digitalWrite( trig, 0 );
    digitalWrite( trig, 1 );
    delayMicroseconds( 10 );
    digitalWrite( trig, 0 );
    unsigned long pong = pulseIn( echo, 1 );
    pong /= 2;
    float distance = 0.0332 * pong;
    if ( distance <= threshold)
    {
      return 1;
    }
    else {
      return 0;
    }
  }

  byte checkMo( void )
  {
    state[0] = ping(trig0, echo0);
    state[1] = ping(trig1, echo1);
    if ( !state[0] && !state[1] || state[0] && state[1])
    {
      return 0;
    }
    else if ( state[0] && !state[1] )
    {
      if ( expect(1, waitTime) )
      {
        return 1;
      }
      else {
        return 0;
      }
    }
    else if ( !state[0] && state[1] )
    {
      if ( expect(0, waitTime) )
      {
        return 2;
      }
      else {
        return 0;
      }
    }
  }

  bool expect( byte eventId, float interval )
  {
    if ( eventId == 1 )
    {
      millisNow = millis();
      while ( millis() - millisNow < int( interval * 1000 ) )
      {
        if ( ping(trig1, echo1) )
        {
          return 1;
        }
      }
      return 0;
    }
    else if ( eventId == 0 )
    {
      millisNow = millis();
      while ( millis() - millisNow < int( interval * 1000 ) )
      {
        if ( ping(trig0, echo0) )
        {
          return 1;
        }
      }
      return 0;
    }


  }

  void Run( void )
  {
    switch ( checkMo() )
    {
      case 1:
        Serial.println("MOTION: RIGHT");
        break;
      case 2:
        Serial.println("MOTION: LEFT");
      default:
        break;
    }
  }

  void runLoop()
  {
    Run();
  }

} counter;

void setup()
{
  counter.Init();
}

void loop()
{
  counter.runLoop();
}
