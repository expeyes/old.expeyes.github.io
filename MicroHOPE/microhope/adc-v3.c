// Reads ADC channel 0 and diplays the result on the LCD, as volts. Uses a crude floating point formatting 

#include "mh-lcd-float.c"
#include "mh-adc.c"


int main()
{
uint16_t       data;
float              v;

lcd_init();
adc_enable();
data = read_adc(0);
v = 5.0 / 1023 * data;
lcd_put_float(v, 3);   // 3 decimal places
lcd_put_char('V');
}
