
/*
expEYES Junior (EYESJ version 1.0)
Program : eyesj.c, runs on PIC24FV32KA302 micro-controller
Listens on the RS232 port, for commands fom the PC
Author  : Ajith Kumar B.P, ( bpajith at gmail.com , ajith at iuac.res.in)
License : GNU GPL version 3
Started on  5-Mar-2012
Last edit : 8-Aug-2012 , 1-Nov-2012

 * The PC side sees 13 I/O channels (numbered 0 to 10) on the EYESJ box.
 * The actual uC pins corresponding to them are mapped using the constant
 * arrays amap, dmap and cmap. The logical channels are listed below.
 * A8 & A9 are configured as outputs permanently, and their output is looped
 * to inputs A6 and A7.  A0 is not an I/O pin.

 * A0 : Comaparator output bit, Not a real channel
 * A1 : Analog Input with level shifter AN11 / RB13 / CN13
 * A2 : Analog Input with level shifter AN12 / RB12 / CN14
 * A3 : Digital/Analog Input AN9  / RB15 / CN11
 * A4 : Digital/Analog Input AN10 / RB14 / CN12
 * A5 : D/A Input with 5K external pullup (Comp input)  AN4 / RB2 / CN6
 * A6 : Analog Input reading SQR1 AN5 / RB3 / CN7
 * A7 : Analog Input reading SQR2 AN15 / RB4 / CN1
 * A8 : SQR1 Digital Out only RB11 / CN15
 * A9 : SQR2 Digital Out only RB10 / CN16   (dummy entry for CN16)
 * A10: Digital output OD1, RB7
 * A11: CCS Control, Digital output, RB6u / shared with programming pin
 * A12: Analog Input  AN0 / RA0  , DAC readback
 */


#include <p24FV32KA302.h>
// Configuration : Primary Crystal Oscillator, No PLL, Watchdog OFF
_FICD(ICS_PGx3)
_FDS(DSWDTEN_OFF)
_FWDT(FWDTEN_OFF &WINDIS_OFF )
_FOSC( FCKSM_CSDCMD & POSCFREQ_HS & POSCMOD_HS & OSCIOFNC_ON )
_FOSCSEL( FNOSC_PRI & IESO_OFF  & SOSCSRC_DIG)

#define	GROUPSIZE		40	// Up to 40 commands in each group

// commands without any arguments (1 to 40)
#define GETVERSION		1	// Get the Eyes firmware version
#define READCMP         2       // Status of C2inD , our A4
#define READTEMP        3       // CTMU temperature
#define GETPORTB        4       // Reads port B, in digital mode

// Commands with One byte argument (41 to 80)
#define READADC		41	// Read the specified ADC
#define GETSTATE        42      // Digital Input Status
#define NANODELAY       43      // delay using CTMU
#define SETADCREF       44      // Choose Vdd (0) or External +Vref (1)
#define READADCSM	45	// Read ADC, in sleep mode
#define IRSEND1         46      // Sends one byte over IR on SQR1
#define RDEEPROM        47     // 1 byte eeprom addr

// Commands with Two bytes argument (81 to 120)
#define R2RTIME         81      //
#define R2FTIME         82      //
#define F2RTIME         83      //
#define F2FTIME         84      //
#define MULTI_R2R       85      //
#define SET2RTIME       86      // From a Dout transition to the Din transition
#define SET2FTIME       87      //
#define CLR2RTIME       88      //
#define CLR2FTIME       89      //
#define HTPUL2RTIME     90      // High True Pulse to HIGH
#define HTPUL2FTIME     91      // High True Pulse to LOW
#define LTPUL2RTIME     92      //
#define LTPUL2FTIME     93      //
#define SETPULWIDTH     94      //
#define SETSTATE        95      // Pin number, hi/lo
#define SETDAC		96	//
#define	SETCURRENT	97	// channel, Irange
#define SETACTION       98      // capture modifiers, action, target pin
#define SETTRIGVAL      99      // Analog trigger level
#define SRFECHOTIME     100     // Trigger to Echo time for SRF0x modules

// Commands with Three bytes argument (121 to 160)
#define	SETSQR1		121	// Set squarewave/level on OC2
#define	SETSQR2		122	// Set squarewave/level on OC3
#define WREEPROM        123     // 1 byte addr, 16 bit dataword

// Commands with Four bytes argument (161 to 200)
#define MEASURECV       163     // ch, irange, duration
#define SETPWM1         164     // PWM on SQR1. scale, pr, ocrs
#define SETPWM2         165     // PWM on SQR1. scale, pr, ocrs
#define IRSEND4         166      // Sends one byte over IR on SQR1

// Commands with Five bytes argument (201 to 240)
#define	CAPTURE		201	// 1 byte CH, 2 byte NS, 2 byte TG
#define	CAPTURE_HR	202	// 1 byte CH, 2 byte NS, 2 byte TG
#define SETSQRS         203     // scale, ocr, time diff

// Commands with Six bytes argument (241 to 255)
#define	CAPTURE2	241	// CH1,CH2: 1byte, NS, TG : 8 bit data
#define	CAPTURE2_HR	242	// same with 12 bit data
#define CAPTURE3        243     // 3 channels, 12 bit
#define CAPTURE4        244     // 3 channels, 12 bit

// Reply from PIC micro to the PC
#define SUCCESS		'D'	// Command executed successfully
#define	INVCMD		'C'	// Invalid Command
#define INVARG		'A'	// Invalid input data
#define INVBUFSIZE	'B'	// Resulting data exceeds buffersize
#define TIMEOUT		'T'	// Time measurement timed out

#define TRUE	1
#define FALSE	0
#define MAXWAIT	400	// .125 * 65536 * 400 = 3.2 sec, Timer3+4
#define MAXTG	1000    // Maximum timegap for CAPTURE, usec
#define MINTG	4	// Minimum timegap for CAPTURE, usec
//#define	DEADTIME	5	// for PULSE2 calls

typedef unsigned char u8;
typedef unsigned int u16;
const char version[] = "ej1.1";

// The arrays below are implemented for programming convenience
#define DE      0      // Dummy entry
const int amap[13]={DE,11, 12, 9, 10, 4, 5, 15, DE, DE, DE, DE, 0};
const int dmap[13]={DE, DE, DE, 1<<15, 1<<14, 1<<2, 1<<3, 1<<4,\
            1<<11, 1<<10, 1<<7, 1<<6, DE};    //corresponding PORTB bit
const int cnmap[13]={DE, DE, DE, 1<<11, 1<<12, 1<<6, DE, DE, DE, DE, DE, DE,DE};
            //pullup possible only on 3, 4 & 5
#define CTT     16      // ADC channel number for CTMU temperature

#define BUFSIZE         1800
u8   dbuf[2+BUFSIZE+2];       // REPLY, ADCSIZE & ISR filler margin
u16  *iptr;                     // Integer pointer
u16  buf_index;
u16  ns, tg;
u16  action;                    // Modifiers for capture, bitmask
u16  actionmask; 		// Bits set as per the action I/O pin
u16  pulse_width = 13;		// Used by PULSE2* functions
u16  triglevel = 2048;  	// Trigger around the ADC middle, 2048
u16  tmp1, tmp2, tmp3;          // For general purpose use
u8   cmd, ch;
u8   cmode;                     // Modes: LR1, HR1, LR2, HR2, LR3, LR4
u8   pulse_pol = 1;             // 1 => HIGH TRUE pulse, 0=> LOW TRUE

// Assembly language routines
extern void wait_cmp_low(void);
extern void wait_cmp_high(void);
extern void wait_for_low(int, int);
extern void wait_for_high(int, int);
extern void usleep(int);
extern int  read_adc(int ch);
extern void capture1_hr(int w0, int ns, int tg, int ch);
extern void capture2_hr(int w0, int ns, int tg, int ch1, int ch2);
extern void capture1(int w0, int ns, int tg, int ch);
extern void capture2(int w0, int ns, int tg, int ch1, int ch2);
extern void capture3(int w0, int ns, int tg, int, int, int);
extern void capture4(int w0, int ns, int tg, int, int, int, int);


u16  __attribute__ ((space(eedata))) eepstart[14];   // EEPROM beginning

void write_eeprom(u8 addr, u16 data)  // EEPROM location & word to write
{
u16 offset;
TBLPAG = __builtin_tblpage(eepstart);
offset = __builtin_tbloffset(eepstart) + addr*2;

NVMCON = 0x4058;                // Erase word mode
__builtin_tblwtl(offset, 0);   
asm volatile ("disi #5");
__builtin_write_NVM();
while(NVMCONbits.WR==1);

NVMCON = 0x4004;                    // Write mode
__builtin_tblwtl(offset, data);     // new data
asm volatile ("disi #5");
__builtin_write_NVM();
while(NVMCONbits.WR==1);
}

u16 read_eeprom(u8 addr)
    {
    u16 offset = __builtin_tbloffset(eepstart);
    TBLPAG = __builtin_tblpage(eepstart);
    offset += addr * 2;
    tmp1 =  __builtin_tblrdl(offset);          // read location
    return tmp1;
    }

#define SRFDELAY    500

void active_tim_cmp(u8 cmd, u8 src)   // Dout transition/pulse to comparator output
    {
    ANSB |= dmap[5];            // SEN input to Analog mode
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    if(cmd == SRFECHOTIME)
        {
        PR5 = 10;               // Reduce Timeout to 10 * 65536/8 usecs
        T4CONbits.TON = 1;
        LATB |= dmap[src];      // Set src pin
        usleep(pulse_width);
        LATB &= ~dmap[src];
        usleep(SRFDELAY);      // Give time for Echo out to go HIGH
        wait_cmp_low();         // then wait for it to come down
        T4CONbits.TON = 0;
        PR5 = MAXWAIT;          // Timeout back to normal
        }
    else if(cmd == SET2RTIME)
        {
        T4CONbits.TON = 1;     // Start Timer
        LATB |= dmap[src];     // Set the source Pin, SQR1, SQR2, OD1 or CCS
        wait_cmp_high();
        T4CONbits.TON = 0;
        }
    else if(cmd == SET2FTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];
        wait_cmp_low();
        T4CONbits.TON = 0;
        }
    else if(cmd == CLR2RTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];     // Clear source pin
        wait_cmp_high();
        T4CONbits.TON = 0;
        }
    else if(cmd == CLR2FTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];     // Clear src pin
        wait_cmp_low();
        T4CONbits.TON = 0;
        }
    else if(cmd == HTPUL2RTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];       // src to HI
        usleep(pulse_width);     // for the width
        LATB &= ~dmap[src];       // src lo LO
        wait_cmp_high();
        T4CONbits.TON = 0;
        }
    else if(cmd == HTPUL2FTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];      // Set src pin
        usleep(pulse_width);
        LATB &= ~dmap[src];
        wait_cmp_low();
        T4CONbits.TON = 0;
        }
    else if(cmd == LTPUL2RTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];     // Clear srcpin
        usleep(pulse_width);
        LATB |= dmap[src];       // Set srcpin
        wait_cmp_high();
        T4CONbits.TON = 0;
        }
    else if(cmd == LTPUL2FTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];      // Low True Pulse
        usleep(pulse_width);
        LATB |= dmap[src];
        wait_cmp_low();
        T4CONbits.TON = 0;
        }
    if(IFS1bits.T5IF)          // Timeout error
        {
        dbuf[0]= TIMEOUT;           // timeout error
        ANSB &= ~dmap[ch];          // Back to Analog
        return;
        }
    *++iptr = TMR4;              // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    buf_index = 4+2;             // res + oddbyte + 4 byte data
    ANSB &= ~dmap[ch];          // Back to Analog
}


void active_tim(u8 cmd, u8 src, u8 dst) // TIM, Time Interval Measurement
    {
    tmp1 = ANSB;
    ANSB &= ~dmap[dst];    // Destination Pin to digital mode
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    if(cmd == SRFECHOTIME)
        {
        PR5 = 10;               // Reduce Timeout to 10 * 65536/8 usecs
        T4CONbits.TON = 1;
        LATB |= dmap[src];
        usleep(pulse_width);
        LATB &= ~dmap[src];
        usleep(SRFDELAY);      // Give time for Echo out to go HIGH
        wait_for_low(0, dmap[dst]);  // then for it to come LOW
        T4CONbits.TON = 0;
        PR5 = MAXWAIT;          // Timeout back to normal
        }
    else if(cmd == SET2RTIME)
        {
        T4CONbits.TON = 1;              // Start Timer
        LATB |= dmap[src];              // SET Source pin
        wait_for_high(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == SET2FTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];
        wait_for_low(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == CLR2RTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];
        wait_for_high(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == CLR2FTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];
        wait_for_low(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == HTPUL2RTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];   // HIGH True Pulse
        usleep(pulse_width);
        LATB &= ~dmap[src];
        wait_for_high(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == HTPUL2FTIME)
        {
        T4CONbits.TON = 1;
        LATB |= dmap[src];
        usleep(pulse_width);
        LATB &= ~dmap[src];
        wait_for_low(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == LTPUL2RTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];     // LOW True Pulse
        usleep(pulse_width);
        LATB |= dmap[src];
        wait_for_high(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    else if(cmd == LTPUL2FTIME)
        {
        T4CONbits.TON = 1;
        LATB &= ~dmap[src];
        usleep(pulse_width);
        LATB |= dmap[src];
        wait_for_low(0, dmap[dst]);
        T4CONbits.TON = 0;
        }
    if(IFS1bits.T5IF)          // Timeout error
        {
        dbuf[0]= TIMEOUT;
        ANSB = tmp1;
        return;
        }
    *++iptr = TMR4;              // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    buf_index = 4+2;             // res + oddbyte + 4 byte data
    ANSB = tmp1;                // Back to Analog
}


void set_sqrs(u8 scale, u16 ocrs, u16 tg)   // OC2 & OC3 driven by T2 & T3
{
    T2CON = 0;                      // STOP Timer
    OC2CON1 = 0;                    // STOP Compare output
    T3CON = 0;                      // STOP Timer
    OC3CON1 = 0;                    // Stop Compare output

    TMR2 = ocrs-1;                  // Start without delay
    TMR3 = ocrs-1-tg;                 // Start after some delay
    OC2R =  ocrs-1;
    OC3R =  ocrs-1;
    OC2RS = ocrs/2-1;
    OC3RS = ocrs/2-1;
    PR2  = ocrs-1;
    PR3  = ocrs-1;
    OC2CON1 = 5;                // OCM mode CC Toggle
    OC3CON1 = 5;                // OCM mode CC Toggle
    OC2CON1bits.OCTSEL = 0;     // 4 = T1, 3 = T5, 2 = T4, 1 = T3, 0 = T2
    OC3CON1bits.OCTSEL = 1;     // 4 = T1, 3 = T5, 2 = T4, 1 = T3, 0 = T2
    OC2CON2 = 12;               //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
    OC3CON2 = 13;               //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
    T2CONbits.TCKPS = scale;    // Timer2 Scale factor
    T3CONbits.TCKPS = scale;    // Timer3 Scale factor
    T2CONbits.TON= 1;           //TON = 1;
    T3CONbits.TON= 1;           //TON = 1;
}

void set_sqr1(u8 scale, u16 ocrs)   // OC2 driven by Timer2, SQR1
{
    T2CON = 0;                      // STOP Timer
    OC2CON1 = 0;                    // Compare output
    if (scale == 255)
        {asm("bset LATB, #11");}     // Set OC2 HIGH
    else if (scale == 254)
        {asm("bclr LATB, #11");}     // Set OC2 LOW
    else
        {
        TMR2 = ocrs-1;
        OC2R =  ocrs-1;
        OC2RS = ocrs/2-1;
        PR2  = ocrs-1;
        OC2CON1 = 5;                // OCM mode CC Toggle
        OC2CON1bits.OCTSEL = 0;     // 4 = T1, 3 = T5, 2 = T4, 1 = T3, 0 = T2
        OC2CON2 = 12;               //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
        T2CONbits.TCKPS = scale;    // Timer2 Scale factor
        T2CONbits.TON= 1;           //TON = 1;
        }
}

void set_sqr2(u8 scale, u16 ocrs)   // OC3 driven by Timer3, SQR2
{
    T3CON = 0;                      // Stop timer
    OC3CON1 = 0;                    // Compare output
    if (scale == 255)
        {asm("bset LATB, #10");}     // Set OC3 HIGH
    else if (scale == 254)
        {asm("bclr LATB, #10");}     // Set OC3 LOW
    else
        {
        TMR3 = ocrs-1;
        OC3R =  ocrs-1;
        OC3RS = ocrs/2-1;
        PR3  = ocrs-1;
        OC3CON1 = 5;                // OCM mode CC Toggle
        OC3CON1bits.OCTSEL = 1;     // 4 = T1, 3 = T5, 2 = T4, 1 = T3, 0 = T2
        OC3CON2 = 13;               //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
        T3CONbits.TCKPS = scale;    // Timer2 Scale factor
        T3CONbits.TON= 1;           //TON = 1;
        }
}


void r2rtime(int src, int dst)
{
    dbuf[0]= TIMEOUT;    // Assume a timeout error
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    T4CONbits.TON = 1;

    if(src == 0)
        wait_cmp_low();
    else
        wait_for_low(0, src);    // First argument is to preserve w0
    if(IFS1bits.T5IF) return;    // Timeout error

    if(src == 0)
        wait_cmp_high();
    else
        wait_for_high(0, src);   // First argument is to preserve w0
    asm("clr TMR5HLD");
    asm("clr TMR4");
    if(IFS1bits.T5IF) return;    // Timeout error

    if(dst == 0)
        wait_cmp_high();
    else
        wait_for_high(0, dst);    // First argument is to preserve w0
    asm("bclr T4CON,#15");
    if(IFS1bits.T5IF) return;    // Timeout error

    *++iptr = TMR4;              // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    buf_index = 4+2;             // res + oddbyte + 4 byte data
    dbuf[0]= SUCCESS;            // Change response
}

void r2ftime(int src, int dst)
{
    dbuf[0]= TIMEOUT;    // Assume a timeout error
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    T4CONbits.TON = 1;

    if(src == 0)
        wait_cmp_low();
    else
        wait_for_low(0, src);    // First argument is to preserve w0
    if(IFS1bits.T5IF) return;    // Timeout error

    if(src == 0)
        wait_cmp_high();
    else
        wait_for_high(0, src);   // First argument is to preserve w0
    asm("clr TMR5HLD");
    asm("clr TMR4");
    if(IFS1bits.T5IF) return;    // Timeout error

    if(dst == 0)
        wait_cmp_low();
    else
        wait_for_low(0, dst);    // First argument is to preserve w0
    asm("bclr T4CON,#15");
    if(IFS1bits.T5IF) return;    // Timeout error

    *++iptr = TMR4;              // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    buf_index = 4+2;             // res + oddbyte + 4 byte data
    dbuf[0]= SUCCESS;            // Change response
}

void f2rtime(int src, int dst)
{
    dbuf[0]= TIMEOUT;    // Assume a timeout error
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    T4CONbits.TON = 1;

    if(src == 0)
        wait_cmp_high();
    else
        wait_for_high(0, src);       // First argument is to preserve w0
    if(IFS1bits.T5IF) return;        // Timeout error

    if(src == 0)
        wait_cmp_low();
    else
        wait_for_low(0, src);      // // First argument is to preserve w0
    asm("clr TMR5HLD");
    asm("clr TMR4");
    if(IFS1bits.T5IF) return;    // Timeout error

    if(src == 0)
        wait_cmp_high();
    else
        wait_for_high(0, dst);     // // First argument is to preserve w0
    asm("bclr T4CON,#15");
    if(IFS1bits.T5IF) return;    // Timeout error

    *++iptr = TMR4;            // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    if(IFS1bits.T5IF) return;    // Timeout error
    buf_index = 4+2;            // res + oddbyte + 4 byte data
    dbuf[0]= SUCCESS;           // Change response
}

void f2ftime(int src, int dst)
{
    dbuf[0]= TIMEOUT;    // Assume a timeout error
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    T4CONbits.TON = 1;

    if(src == 0)
        wait_cmp_high();
    else
        wait_for_high(0, src);       // First argument is to preserve w0
    if(IFS1bits.T5IF) return;        // Timeout error

    if(src == 0)
        wait_cmp_low();
    else
        wait_for_low(0, src);      // // First argument is to preserve w0
    asm("clr TMR5HLD");
    asm("clr TMR4");
    if(IFS1bits.T5IF) return;    // Timeout error

    if(src == 0)
        wait_cmp_low();
    else
        wait_for_low(0, dst);     // // First argument is to preserve w0
    asm("bclr T4CON,#15");
    if(IFS1bits.T5IF) return;    // Timeout error

    *++iptr = TMR4;            // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    if(IFS1bits.T5IF) return;    // Timeout error
    buf_index = 4+2;            // res + oddbyte + 4 byte data
    dbuf[0]= SUCCESS;           // Change response
}


void multi_r2rtime(int pin, int nskip)
{
    dbuf[0]= TIMEOUT;    // Assume a timeout error
    iptr = (u16*)dbuf;
    IFS1bits.T5IF = 0;
    TMR4 = 0;
    TMR5 = 0;
    T4CONbits.TON = 1;

    if(pin == 0)
        wait_cmp_low();
    else
        wait_for_low(0, pin);       // First argument is to preserve w0

    if(pin == 0)
        wait_cmp_high();
    else
        wait_for_high(0, pin);      // // First argument is to preserve w0
    asm("clr TMR5HLD");
    asm("clr TMR4");
    if(IFS1bits.T5IF) return;    // Timeout error

rep:
    if(pin == 0)
        wait_cmp_low();
    else
        wait_for_low(0, pin);    // First argument is to preserve w0
    if(pin == 0)
        wait_cmp_high();
    else
        wait_for_high(0, pin);   // First argument is to preserve w0
    if(nskip--) goto rep;
    asm("bclr T4CON,#15");
    if(IFS1bits.T5IF) return;    // Timeout error

    *++iptr = TMR4;              // Collect 32bit value to buffer
    *++iptr = TMR5HLD;
    buf_index = 4+2;             // res + oddbyte + 4 byte data
    dbuf[0]= SUCCESS;            // Change response
}

#define I2CADDR   0          // DAC address is zero
void write_dac(int dat)
{
    I2C1CONbits.SEN = 1;        // Start condition
    while(I2C1CONbits.SEN) ;
    usleep(500);
    I2C1TRN = 0xC0 | (I2CADDR << 1);  // 0xC0 + (device addr << 1)
    while (I2C1STATbits.TRSTAT) ;

    dat &= 0x0fff;              // Consider only 12 bits
    I2C1TRN = dat>>8;             // DAC write command + 4MSB
    while (I2C1STATbits.TRSTAT) ;
    I2C1TRN = dat&255;             // 8 LSB
    while (I2C1STATbits.TRSTAT) ;

    I2C1CONbits.PEN = 1;        // Stop condition
    usleep(1500);
    }

// Slow ADC read, average of 16 reading taken in sleep mode.
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void)
{
    IFS0bits.AD1IF = 0;
    tmp2 += ADC1BUF0;
}

u16 read_adc_sleepavg(u8 ch) // averaging , convert in sleep mode
{
AD1CHS = ch;
AD1CON3bits.ADRC = 1;       //
IFS0bits.AD1IF = 0;
IEC0bits.AD1IE = 1;
tmp2 = 0;
for(tmp1=0; tmp1 < 16; ++tmp1)
    {
    AD1CON1bits.SAMP = 1;
    usleep(100);
    AD1CON1bits.SAMP = 0;
    asm("pwrsav #0");           // MCU to sleep mode
    asm("nop");
    asm("nop");
    asm("nop");
    while(!AD1CON1bits.DONE) ;
    }
AD1CON3bits.ADRC = 0;
IEC0bits.AD1IE = 0;
return tmp2 >> 4;
}

u16 read_adc_avg(u8 ch)  // Averaging, no sleep mode
{
AD1CHS = ch;
IFS0bits.AD1IF = 0;
tmp2 = 0;
for(tmp1=0; tmp1 < 16; ++tmp1)
    {
    AD1CON1bits.SAMP = 1;
    usleep(100);
    AD1CON1bits.SAMP = 0;
    asm("nop");
    while(!AD1CON1bits.DONE) ;
    tmp2 += ADC1BUF0;
    }
return tmp2 >> 4;
}


void init(void)     // Initialize I/O ports, UART, ADC , SPI etc.
{
    ANSA = 1;       // ADC Channel 0 as Analog
    TRISA = 0xffff; // PORTA must be all input, unused pins are grounded.

    ANSB = 0xffff;  // All portB to analog mode
    TRISBbits.TRISB7  = 0;          // OD1 output
    TRISBbits.TRISB11 = 0;          // OC2 - SQR1
    TRISBbits.TRISB10 = 0;          // OC3 - SQR2

//CNPU1bits.CN11PUE = 1;          // pullup on CN11 (RB15)
//CNPU1bits.CN12PUE = 1;          // pullup on CN12 (RB14)

    // UART Initialization
    TRISBbits.TRISB1 = 1;   // U2Rx as Input, else it won't work
    ANSBbits.ANSB1 = 0;     // This should be set as Digital Input
    U2MODE = 0x8008;        // enable UART, BRGH, 1 stop bit, No parity
    U2STA = 0x0400;         // Enable Tx
    U2BRG = 16;             // for 16Mhz clock:(38400,51), (115200,16)

    // ADC Initialization
    AD1CON1 = 0x8402;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CON5 = 0;
    AD1CSSL = 0;

    // I2C Initialization
    I2C1CONbits.I2CEN = 1;      // Enable I2C unit
    I2C1BRG = 18;               // Baud rate 100kHz . 18
    usleep(500);                // Delay needed

    // Initialize T4+T5 as a 32 bit timer
    T4CONbits.T32 = 1;      // 32 bit mode T4 and T5
    PR4  = 0;               // PR4 & 5 decides the limit
    PR5  = MAXWAIT;         // 3.2 seconds limit
    T4CONbits.TCKPS = 0;    // pre-scaling bits
    IFS1bits.T5IF = 0;

    // Comparator2 initialization
    CM2CONbits.CREF = 1;            //  +IN is CVref
    CM2CONbits.CCH =  2;            // -IN is C2IND pin
    CM2CONbits.CPOL =  1;           // Inverted output
    CM2CONbits.CON =  1;            // Enable Comparator 2
    //CM2CONbits.COE =  1;            // Enable Output pin (RB7)
    CVRCONbits.CVREN = 1;           // Enable CVref
    //CVRCONbits.CVROE = 1;         // should NOT do this
    CVRCONbits.CVR = 8;             // 5V * 8/32 is +Vin

    // CTMU
    CTMUCON1 = 0;
    CTMUICON = 0;
    }

int measure_cv(u8 chan, u8 rng, int timer )
{
    AD1CHS = chan;
    //AD1CTMUENL = (1 << ch);   // ADC channel to CTMU
    AD1CON1bits.SAMP = 1;       // ADC to Sample mode
    CTMUCON1bits.CTMUEN = 1;    // Start CTMU
    CTMUICONbits.IRNG = rng;
    usleep(5);                  // Time to stabilize
    CTMUCON1bits.IDISSEN = 1;       // Ground CS output
    for(tmp1=0; tmp1 < 5; ++tmp1)   // to remove stored charge
        usleep(tg);
    CTMUCON1bits.IDISSEN = 0;

    CTMUCON2bits.EDG2 = 0;
    CTMUCON2bits.EDG1 = 1;      // Enable current source
    usleep(timer);              // for the given duration
    CTMUCON2bits.EDG1 = 0;
    AD1CON1bits.SAMP = 0;       // Start convert
    while(!AD1CON1bits.DONE) ;  // wait for ADC
    AD1CON1bits.SAMP = 1;       // ADC back to Sample mode

    CTMUCON1bits.CTMUEN = 0;    // Stop CTMU
    return ADC1BUF0;
}

int nano_delay(u8 ir) // EDGE13 to EDGE5, cap of IN1
{
    // NOT working. Will fix this later    BPA, 18-May-12
    ch = amap[3];
    AD1CHS = ch;         // input capacitance of IN1 is used
    AD1CTMUENL = (1 << ch);   // ADC channel to CTMU
    AD1CON1bits.SAMP = 1;       // ADC to Sample mode
    CTMUCON1bits.CTMUEN = 1;    // Start CTMU
    CTMUICONbits.IRNG = ir;       // 5.5 uA
    usleep(5);                  // Time to stabilize
    CTMUCON1bits.IDISSEN = 1;       // Ground CS output
    for(tmp1=0; tmp1 < 5; ++tmp1)   // to remove stored charge
        usleep(tg);
    CTMUCON1bits.IDISSEN = 0;

    // Start process here
    //CTMUCON1bits.EDGSEQEN = 1;

    AD1CON1bits.SAMP = 1;       // ADC in sampling mode
    AD1CON1bits.SSRC = 4;       // Start convert by CTMU event

    CTMUCON1bits.EDGEN = 1;
    CTMUCON2bits.EDG1 = 0;
    CTMUCON2bits.EDG2 = 0;
    CTMUCON2bits.EDG1EDGE = 1;
    CTMUCON2bits.EDG2EDGE = 1;
    CTMUCON2bits.EDG1POL = 1;
    CTMUCON2bits.EDG2POL = 1;
    CTMUCON2bits.EDG1SEL = 5;
    CTMUCON2bits.EDG2SEL = 13;
    asm("nop");
    usleep(10); //    while(!AD1CON1bits.DONE) ;  // wait for ADC
    AD1CON1bits.SAMP = 1;       // ADC back to Sample mode
    AD1CON1bits.SSRC = 0;       // Start mode back to software
    AD1CON1;
    CTMUCON2;
    CTMUCON1;

    CTMUCON1 = 0;    // Stop CTMU
    CTMUCON2 = 0;
    return ADC1BUF0;
}

void set_pwm1(u16 resol, u16 dcyc) // Resolution & duty cycle
{
    T2CON = 0;                // STOP Timer
    OC2CON1 = 0;              // Compare output
    OC2CON1bits.OCTSEL = 7;   // source is clock
    OC2RS  = resol;
    OC2R = dcyc;
    OC2CON2 = 0x001F;         // sync on this itself
    OC2CON1bits.OCM = 6;      // Simple PWM mode
}

void set_pwm2(u16 resol, u16 dcyc)   // Resolution & duty cycle
{
    T3CON = 0;                // STOP Timer
    OC3CON1 = 0;              // Compare output
    OC3CON1bits.OCTSEL = 7;   // source is system clock
    OC3RS  = resol;
    OC3R = dcyc;
    OC3CON2 = 0x001F;         // sync on this itself
    OC3CON1bits.OCM = 6;      // Simple PWM mode
}


int main()
{
iptr = (u16*)dbuf;
CLKDIV = 0;                 // No clock division, no DOZE
init();


buf_index = 0;
while(1)
    {
    while(!U2STAbits.URXDA);      // Wait for data from PC
    dbuf[buf_index++] = U2RXREG;
    if( (buf_index==1) && dbuf[0] == 0xf8 )// work around for the Rpi AMA bug
        {
        buf_index = 0;
        continue;
        }

    if(buf_index*GROUPSIZE > dbuf[0])
        {
        cmd = dbuf[0];
        dbuf[0]= SUCCESS;    // Assume success
        buf_index = 1;
        #define SPACE 680
        switch(cmd)         // Command processing starts here
            {
            case WREEPROM:
	  	tmp1 = dbuf[2] | (dbuf[3] << 8);  // data
	  	write_eeprom(dbuf[1], tmp1);
                break;

            case RDEEPROM:
	  	tmp1 = read_eeprom(dbuf[1]);    // give location
                dbuf[buf_index++] = tmp1 & 255;
      		dbuf[buf_index++] = tmp1 >> 8;
                break;

            case IRSEND1:   // Transmits 1 byte via IR LED on SQR1
                // Sets SQR1 in CTC mode, as per the byte to send
                TMR2 = OC2R = PR2 = 210;
                OC2RS = 105;
                OC2CON1bits.OCTSEL = 0;  // 4=T1,3=T5, 2 = T4, 1 = T3, 0 = T2
                OC2CON2 = 12;       //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
                T2CONbits.TCKPS = 0;    // Timer2 Scale factor
                T2CONbits.TON= 1;      //TON = 1;
                OC2CON1 = 5;            // OCM mode CC Toggle
                usleep(2000);usleep(2000);usleep(2000);usleep(2000);
                usleep(1000);
                OC2CON1 = 0;            // OCM mode CC Toggle
                usleep(2000);usleep(2000);
                usleep(400);

        	ch = dbuf[1];			// byte to transmit
		for(tmp1=0; tmp1 < 8; ++tmp1)
                    {
                    OC2CON1 = 5; usleep(SPACE);  // pulse ON
                    OC2CON1 = 0; usleep(440);
                    if(ch & 128) usleep(1120);
                    ch <<= 1;
                    }
                OC2CON1 = 5; usleep(SPACE/2);  // pulse ON
                OC2CON1 = 0;
                break;

            case IRSEND4:   // Transmits 1 byte via IR LED on SQR1
                // Sets SQR1 in CTC mode, as per the byte to send
                TMR2 = OC2R = PR2 = 210;
                OC2RS = 105;
                OC2CON1bits.OCTSEL = 0;  // 4=T1,3=T5, 2 = T4, 1 = T3, 0 = T2
                OC2CON2 = 12;       //SYNC, 15=T5, 14=T4, 13=T3, 12=T2, 11=T1
                T2CONbits.TCKPS = 0;    // Timer2 Scale factor
                T2CONbits.TON= 1;      //TON = 1;
                OC2CON1 = 5;            // OCM mode CC Toggle
                usleep(2000);usleep(2000);usleep(2000);usleep(2000);
                usleep(1000);
                OC2CON1 = 0;            // OCM mode CC Toggle
                usleep(2000);usleep(2000);
                usleep(400);
		for(tmp2=0; tmp2 < 4; ++tmp2)
                {
        	ch = dbuf[tmp2+1];	// bytes in dbuf[1] to [4]
		for(tmp1=0; tmp1 < 8; ++tmp1)
                    {
                    OC2CON1 = 5; usleep(SPACE);  // pulse ON
                    OC2CON1 = 0; usleep(440);
                    if(ch & 128) usleep(1120);
                    ch <<= 1;
                    }
                }
                OC2CON1 = 5; usleep(SPACE);  // pulse ON
                OC2CON1 = 0;
                break;

            case SETPWM1:
	  	tmp1 = dbuf[1] | (dbuf[2] << 8);  // PR2 value
	  	tmp2 = dbuf[3] | (dbuf[4] << 8);  // OC2RS value
                set_pwm1(tmp1, tmp2);
                break;

            case SETPWM2:
	  	tmp1 = dbuf[1] | (dbuf[2] << 8);  // PR2 value
	  	tmp2 = dbuf[3] | (dbuf[4] << 8);  // OC2RS value
                set_pwm2(tmp1, tmp2);
                break;

            case SETADCREF:     // ADCref:  Vdd  or external +Vref
                if(dbuf[1])     // External +Vref
                    AD1CON2bits.PVCFG = 1;      // Choose external +Vref
                else
                    AD1CON2bits.PVCFG = 0;      // +Vref = Vdd
                break;

            case READADCSM:
                ANSB |= dmap[dbuf[1]];     // Make sure pin is Analog
                ns = read_adc_sleepavg(amap[dbuf[1]]);
                dbuf[buf_index++] = ns & 255;
      		dbuf[buf_index++] = ns >> 8;
      		break;

            case READADC:             // Read without sleep & averaging
                ANSB |= dmap[dbuf[1]];     // Make sure pin is Analog
                ns = read_adc_avg(amap[dbuf[1]]);
                dbuf[buf_index++] = ns & 255;
      		dbuf[buf_index++] = ns >> 8;
      		break;

            case NANODELAY:      // NOT working yet
                nano_delay(dbuf[1]);
                dbuf[buf_index++] = tmp1 & 255;
      		dbuf[buf_index++] = tmp1 >> 8;
                break;

            case MEASURECV:        // ch, range, time interval
                tg = dbuf[3] | (dbuf[4] << 8);
                tmp1 = measure_cv(amap[dbuf[1]], dbuf[2], tg);
                dbuf[buf_index++] = tmp1 & 255;
      		dbuf[buf_index++] = tmp1 >> 8;
                break;

            case SETCURRENT:        // channel & irange
                if(dbuf[1] == 0)
                    {
                    CTMUCON1 = 0;              // Stop CTMU
                    dbuf[buf_index++] = 0;
                    dbuf[buf_index++] = 0;
                    break;
                    }
                CTMUCON1bits.CTMUEN = 1;    // Start CTMU
                CTMUICONbits.IRNG = dbuf[2];
                usleep(5);
                ANSB |= dmap[dbuf[1]];      // Pin Analog mode
                AD1CHS = amap[dbuf[1]];
                AD1CON1bits.SAMP = 1;       // ADC back to Sample mode
                CTMUCON2bits.EDG1 = 1;
                CTMUCON2bits.EDG2 = 0;
                AD1CON1bits.SAMP = 0;       // Start convert
                while(!AD1CON1bits.DONE) ;  // wait for ADC
                AD1CON1bits.SAMP = 1;       // ADC back to Sample mode
                dbuf[buf_index++] = ADC1BUF0 & 255;
      		dbuf[buf_index++] = ADC1BUF0 >> 8;
      		break;

            case READTEMP:      // Internal Diode . What current ???
                CTMUCON1bits.CTMUEN = 1;    // Start CTMU
                AD1CTMUENHbits.CTMEN16 = 1;
                CTMUICONbits.IRNG = 3;
                CTMUCON2bits.EDG1 = 1;
                CTMUCON2bits.EDG2 = 0;
                usleep(10);
                ns = read_adc_avg(CTT);
                CTMUCON1bits.CTMUEN = 0;    // Stop CTMU
                dbuf[buf_index++] = ns & 255;
      		dbuf[buf_index++] = ns >> 8;
      		break;

            case SETTRIGVAL:        // Sets trigger level
                tg = dbuf[1] | (dbuf[2] << 8);
                if( (tg >= 100) & (tg <= 4000))
                    triglevel = tg;
                break;

            case SETSTATE:
                // CCS shares the PGEC bit, kept as input during programming

                ch = dbuf[1];
                if(ch == 11) TRISBbits.TRISB6  = 0;    // CCS output
                if( (ch < 8) || (ch > 11) )  // SQR1,SQR2,OD & CCS
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                if(dbuf[2])                     // any non-zero value
                    LATB |= dmap[ch];          // Set the bit HI
                else
                    LATB &= ~dmap[ch];          // Set the bit LO
      		break;

            case SETPULWIDTH:
                pulse_width = dbuf[1] | (dbuf[2] << 8);
                break;

            case SETACTION:
                action = 1 << dbuf[1];   // Assembly  routine Uses Bitmask
                if(action == 1)          // Analog trigger
                    {
                    actionmask = amap[dbuf[2]];
                    ANSB |= dmap[dbuf[2]];  // Keep input in Analog mode
                    }
                else
                    actionmask = dmap[dbuf[2]];
                break;

            case SET2RTIME:       // SET to HIGH
            case SET2FTIME:       // SET to LOW
            case CLR2RTIME:       // CLR to HIGH
            case CLR2FTIME:       // CLR to LOW
            case HTPUL2RTIME:     // HIGH True Pulse to HIGH on pin
            case HTPUL2FTIME:     // HTGH True Pulse to LOW on  pin
            case LTPUL2RTIME:     // LOW True Pulse  to HIGH on pin
            case LTPUL2FTIME:     // LOW True Pulse  to LOW on  pin
            case SRFECHOTIME:
                if(dbuf[2] == 0)  // Destination is Comparator
                    active_tim_cmp(cmd, dbuf[1]);
                else
                    active_tim(cmd, dbuf[1], dbuf[2]);  // cmd, src & dst
                break;

            case GETPORTB:
                tmp1 = ANSB;
                ANSB = 0;       // PortB all bits digital
                //asm("nop");
                //writeEE(10);
                //tmp2 = NVMCON; // PORTB;
                dbuf[buf_index++] = tmp2 & 255;      // Low byte
                dbuf[buf_index++] = tmp2 >> 8;
                ANSB = tmp1;    // Back to initial state
                break;

            case GETSTATE:
                ch = dbuf[1];
                if(ch == 0)  // Special Case, Comparator output status
                    {
                    ANSB |= dmap[5];     // SEN to Analog mode
                    usleep(5);                  // Settling time
                    dbuf[buf_index++] = CM2CONbits.COUT;
                    }
                else        // Other  Inputs
                    {
                    ANSB &= ~dmap[ch];          // Pin to digital
                    if(PORTB & dmap[ch])
                        dbuf[buf_index++] = 1;
                    else
                        dbuf[buf_index++] = 0;
                    ANSB |= dmap[ch];          // Back to Analog
                    }
      		break;

            case SETSQRS:
		ch = dbuf[1];              // TCKPS, pre-scaling
	  	tmp1 = dbuf[2] | (dbuf[3] << 8);  // OCRS value
	  	tmp2 = dbuf[4] | (dbuf[5] << 8);  // OCRS value
                set_sqrs(ch, tmp1, tmp2);
                break;

            case SETSQR1:
	  	tg = dbuf[2] | (dbuf[3] << 8);  // OCRS value
                set_sqr1(dbuf[1], tg);
                break;

            case SETSQR2:
	  	tg = dbuf[2] | (dbuf[3] << 8);  // OCRS value
                set_sqr2(dbuf[1], tg);
                break;

            case R2RTIME:
                tmp1 = ANSB;
                ANSB &= ~dmap[dbuf[1]];     // Go digital
                ANSB &= ~dmap[dbuf[2]];     // Go digital
                r2rtime(dmap[dbuf[1]], dmap[dbuf[2]]);
                ANSB = tmp1;
                break;

            case R2FTIME:
                tmp1 = ANSB;
                ANSB &= ~dmap[dbuf[1]];     // Go digital
                ANSB &= ~dmap[dbuf[2]];     // Go digital
                r2ftime(dmap[dbuf[1]], dmap[dbuf[2]]);
                ANSB = tmp1;
                break;

            case F2RTIME:
                tmp1 = ANSB;
                ANSB &= ~dmap[dbuf[1]];     // Go digital
                ANSB &= ~dmap[dbuf[2]];     // Go digital
                f2rtime(dmap[dbuf[1]], dmap[dbuf[2]]);
                ANSB = tmp1;
                break;

            case F2FTIME:
                tmp1 = ANSB;
                ANSB &= ~dmap[dbuf[1]];     // Go digital
                ANSB &= ~dmap[dbuf[2]];     // Go digital
                f2ftime(dmap[dbuf[1]], dmap[dbuf[2]]);
                ANSB = tmp1;
                break;

            case MULTI_R2R:
                tmp1 = ANSB;
                ANSB &= ~dmap[dbuf[1]];     // Go digital
                multi_r2rtime(dmap[dbuf[1]], dbuf[2]);
                ANSB = tmp1;
                break;

            case SETDAC:
	  	tg = dbuf[1] | (dbuf[2] << 8);  // DAC value
                write_dac(tg);
                break;

            case CAPTURE4:
	  	ns = dbuf[3] | (dbuf[4] << 8);  // Number of samples
		tg = dbuf[5] | (dbuf[6] << 8) ; // Time gap
		if(4*ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                tmp1 = dbuf[1] & 15;        // 1st channel
                ANSB |= dmap[tmp1];
                tmp2 = dbuf[1] >>4;         // 2nd channel
                ANSB |= dmap[tmp2];
                tmp3 = dbuf[2] & 15;        // 3rd
                ANSB |= dmap[tmp3];
                ch = dbuf[2] >>4;           // 4th channel
                ANSB |= dmap[ch];
		capture4(0, ns*4, tg, amap[tmp1], amap[tmp2], \
                                      amap[tmp3], amap[ch]);
                dbuf[0] = SUCCESS;
                dbuf[1] = 2;             // Data Size
                buf_index = 4*ns+2;         // 2 byte ADC data
		break;

            case CAPTURE3:
	  	ns = dbuf[3] | (dbuf[4] << 8);  // Number of samples
		tg = dbuf[5] | (dbuf[6] << 8) ; // Time gap
		if(3*ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                tmp1 = dbuf[1] & 15;        // 1st channel
                ANSB |= dmap[tmp1];
                tmp2 = dbuf[1] >>4;         // 2nd channel
                ANSB |= dmap[tmp2];
                tmp3 = dbuf[2];
                ANSB |= dmap[tmp3];
		capture3(0, ns*3, tg, amap[tmp1], amap[tmp2], amap[tmp3]);
                dbuf[0] = SUCCESS;
                dbuf[1] = 2;             // Data Size
                buf_index = 3*ns+2;         // 2 byte ADC data
		break;

            case CAPTURE2:
	  	ns = dbuf[3] | (dbuf[4] << 8);  // Number of samples
		tg = dbuf[5] | (dbuf[6] << 8) ; // Time gap
		if(2*ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                ANSB |= dmap[dbuf[1]];
                ANSB |= dmap[dbuf[2]];
		capture2(0, ns*2, tg, amap[dbuf[1]], amap[dbuf[2]]);
                dbuf[0] = SUCCESS;
                dbuf[1] = 1;             // Data Size
                buf_index = 2*ns+2;      // 1 byte ADC data
		break;

            case CAPTURE2_HR:
	  	ns = dbuf[3] | (dbuf[4] << 8);  // Number of samples
		tg = dbuf[5] | (dbuf[6] << 8) ; // Time gap
		if(4*ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                ANSB |= dmap[dbuf[1]];
                ANSB |= dmap[dbuf[2]];
		capture2_hr(0, ns*2, tg, amap[dbuf[1]], amap[dbuf[2]]);
                dbuf[0] = SUCCESS;
                dbuf[1] = 2;             // Data Size
                buf_index = 4*ns+2;         // 2 byte ADC data
		break;

            case CAPTURE:       // 8 bit adc data
	  	ns = dbuf[2] | (dbuf[3] << 8);  // Number of samples
		tg = dbuf[4] | (dbuf[5] << 8) ; // Time gap
		if(ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                ANSB |= dmap[dbuf[1]];
		capture1(0, ns, tg, amap[dbuf[1]]);   // dbuf[1] is ch#
                dbuf[0] = SUCCESS;
                dbuf[1] = 1;             // Data Size
                buf_index = ns+2;          // 1 byte ADC data
		break;

            case CAPTURE_HR:        // 12 bit adc data
	  	ns = dbuf[2] | (dbuf[3] << 8);  // Number of samples
		tg = dbuf[4] | (dbuf[5] << 8) ; // Time gap
		if(2*ns > BUFSIZE)
                    {
                    dbuf[0] = INVARG;
                    break;
                    }
                ANSB |= dmap[dbuf[1]];
		capture1_hr(0, ns, tg,amap[dbuf[1]]);  // dbuf[1] is ch#
                dbuf[0] = SUCCESS;
                dbuf[1] = 2;
                buf_index = 2*ns + 2;       // 2 bytes per ADC data
		break;


            case GETVERSION:
      		for(ns=0 ; ns < 5; ++ns)
                    dbuf[buf_index++] = version[ns];
      		break;

            default:
        	dbuf[0] = INVCMD;		// Invalid Command
      		break;
            }

        while(!U2STAbits.TRMT){};   // Wait for Transmit buffer
        U2TXREG = dbuf[0];       // Send the response byte in all cases
	if(dbuf[0] == SUCCESS)   // If no error, send the data bytes
	    for(ns=1; ns < buf_index; ++ns)
                {
    		while(!U2STAbits.TRMT){};   // Wait for Transmit Buffer
                U2TXREG= dbuf[ns];
		}
        buf_index = 0;
        }
    }
}
