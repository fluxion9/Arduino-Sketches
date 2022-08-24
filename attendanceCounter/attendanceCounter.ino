#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define trig0 6
#define echo0 7
#define trig1 8
#define echo1 9

#define rst 10


struct attC
{
	bool state[2] = {0, 0};
	float threshold = 100.0; //cm
	float waitTime = 1; //second(s)
  	unsigned long millisNow, dMillis = 0;
	int count;
	
	void Init( void )
	{
    		pinMode( rst, 2 );
		count = 0;	
    		lcd.begin( 16, 2 );
    		lcd.clear();
    		lcd.setCursor( 3, 1 );
    		lcd.print( "ATTENDANCE" );
    
	}
 	void updateDisplay( int rate )
 	{
  		if( millis() - dMillis >= rate )
  		{
    			lcd.clear();
    			lcd.setCursor( 3, 0 );
    			lcd.print( "ATTENDANCE" );
    			lcd.setCursor( 0, 1 );
    			lcd.print( "count = " + String( count ) );
    			dMillis = millis();
    
  		}
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
		if( distance <= threshold)
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
		if( !state[0] && !state[1] || state[0] && state[1])
		{
			return 0;
		}
		else if( state[0] && !state[1] )
		{
			if( expect(1, waitTime) )
			{
				return 1;
			}
			else {
				return 0;
			}
		}
		else if( !state[0] && state[1] )
		{
			if( expect(0, waitTime) )
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
		if( eventId == 1 )
		{
			millisNow = millis();
			while( millis() - millisNow < int( interval * 1000 ) )
			{
				if( ping(trig1, echo1) )
				{
					return 1;
				}
				return 0;
			}
		}
		else if( eventId == 0 )
		{
			millisNow = millis();
			while( millis() - millisNow < int( interval * 1000 ) )
			{
				if( ping(trig0, echo0) )
				{
					return 1;
				}
				return 0;
			}
		}
				
				
	}
	
	void Run( void )
	{
    		if( !digitalRead( rst ) )
    		{
      			count = 0;
    		}
		switch( checkMo() )
		{
			case 1:
				count++;
				break;
			case 2:
				count--;
				if( count < 0 )
				{
					count = 0;
				}
			
			default:
				break;
		}	
	}

	void runLoop()
  	{
    		Run();
    		updateDisplay( 1000 );
  	}
	
}counter;

  void setup()
  {
    counter.Init();
  }

  void loop()
  {
    counter.runLoop();
  }
 
