#include "mh-utils.c"


int main (void)
  {
DDRA = 255;  //For controlling the stepper motor

 
  DDRB = 255;		// Data Direction Register for port B

  for(;;)
    {
    PORTB = 255;	
    delay_ms(500);
    PORTB = 0;
    delay_ms(500);
  }
return 0;
}
