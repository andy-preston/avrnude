///////////////////////////////////////////////////////////////////////////////
//
// NOTE: this code requires that SERIAL_RX_BUFFER_SIZE be set to 1024 in
// C:\Program Files\Arduino\hardware\arduino\avr\cores\arduino\HardwareSerial.h
//
///////////////////////////////////////////////////////////////////////////////

/* ASCII ICSP implementation for the Arduino NANO
 * (c) Robert Rozee  2015
 * Modified for use with avrnude
 * Andy Preston 2023
 *
 * Below is the currently implemented command set:
 *
 * 'd' : TDI = 0, TMS = 0, read_flag = 0	0x64
 * 'e' : TDI = 0, TMS = 1, read_flag = 0
 * 'f' : TDI = 1, TMS = 0, read_flag = 0
 * 'g' : TDI = 1, TMS = 1, read_flag = 0
 *
 * 'D' : TDI = 0, TMS = 0, read_flag = 1	0x44
 * 'E' : TDI = 0, TMS = 1, read_flag = 1
 * 'F' : TDI = 1, TMS = 0, read_flag = 1
 * 'G' : TDI = 1, TMS = 1, read_flag = 1
 *
 * '+' : TDI = 0, TMS = 0, accumulate PrAcc	0x2B
 *
 * (if read_flag = 1 then respond with TDO value of '0' or '1')
 *
 * '.' : no operation, used for formatting
 * '>' : request a sync response of '<'
 * '=' : retrieve accumulated PrAcc values, then set PrAcc = 1
 *
 * '0' : clock out a 0 on PGD pin
 * '1' : clock out a 1 on PGD pin
 * '-' : clock in single PGD bit	(*** for other device families)
 *
 * '2' : set MCLR low
 * '3' : set MCLR hi-Z
 *
 * '4' : turn Vcc (power to target) OFF
 * '5' : turn Vcc (power to target) ON
 *
 * '6' : turn Vpp OFF, RST ON		(*** for other device families)
 * '7' : turn RST OFF, Vpp ON		(*** for other device families)

 * '8' : insert 10mS delay
 * '@' : return A0..A5 inputs as 6 lines of text, null terminated after last line
 * '?' : return ID string, "ascii ICSP v1X"
 *
 * note 1: version number is a single numeric digit followed by single UC letter
 *         if backwards compatibility preserved then only letter needs to change
 *         if compatibility is broken then digit should increment, ie 1D -> 2A
 *
 * note 2: 2-wire, 2-phase transaction can be implemented with commands '0' and '1'
 *
 * note 3: commands '-', '6', '7' are intended to possibly allow the programming
 *         of other/older PIC families that use a different ICSP command set. these
 *         devices are likely to have much less flash storage, so any added time
 *         overhead is not of major concern. for future use
 *
 * note 4: commands '+' and '=' are to allow for accumulating the PrAcc bit when
 *         XferFastData is used. retrieving every PrAcc bit in the normal way would
 *         double the time taken to program a device. for future use
 *
 * note 5: analog inputs A0 and A1 should be reserved for reading Vcc and Vpp

 # addendum: 'i' to 'x' are used to encode a TDI data packet, 4-bits per symbol
 #           'I' to 'X' encode as above, with read_flag set - returns same
 #           'a' encodes the header sequence 'edd'
 #           'z' encodes the footer sequence 'ed'
 #           'A' encodes the header sequence 'edD'
 #           '@' returns readings in units of millivolts
 #
 # the above additions first introduced in version 1E
 # 4-bit encoding reduces the symbol stream length by around 70%


Interface pins on Arduino:
-------------------------
PGC    : (D2) open collector output, 3k3 pullup to Vcc (3v3)
PGD    : (D3) open collector output, 3k3 pullup to Vcc (3v3)
MCLR   : (D4) open collector output, pullup should be on target

Vcc (multiple pins) : fed from multiple 5v output pins via current limiting
resistors (100r, 17mA ea), with a 3v3 zener diode to ground. alternatively,
replace zener with 3v3 LDO regulator and make resistor values smaller (22r
should do)

RST    : (8) base drive for external MCLR switching transistor
Vpp    : (9) drive for external Vpp switching opto coupler

RST and Vpp are mutually exclusive. if an HV programmer is implemented it
should have it's own seperate ICSP header. Vpp should NEVER be on the same
header as MCLR to prevent the risk of damaging the 328p


2-wire, 4-phase transaction:
---------------------------
PGD := TDI
pulse PGC high
PGD := TMS
pulse PGC high
PGD := 1 (hi-Z with 3k3 pullup)
pulse PGC high
TDO := PGD
pulse PGC high


Enter ICSP mode:
---------------
MCLR := 0
PGD := 0
PGC := 0
Vcc := 1		(apply power to target, wait 50mS to stabilize)
pulse MCLR high
(pause P18)
clock out "MCHP" signature
(pause P19)
MCLR := 1
(pause P7)

command string: "5.88888.32.8.0100.1101.0100.0011.0100.1000.0101.0000.8.3.8"


Exit ICSP mode:
--------------
MCLR := 0	(hold target in reset)
Vcc := 0	(target now powered down)

command string: "88888.4"    (first wait 50mS to ensure target is no longer busy)


Using an Arduino NANO just as a USB to serial bridge:
----------------------------------------------------
if pins 28 and 29 are jumpered together (RESET and GND) then the 328p will be
held in reset with the processors TxD and RxD pins hi-Z. while in this state the
USB to serial bridge portion of the Nano can be used for communicating with a
target processor

if pins 28 and 27 are jumpered together, resetting via the USB serial port will
be disabled. if not jumpered, opening the port on some systems may cause one or
more resets, delaying the 328p being able to respond to commands. remember that
the jumper must be removed to upload new firmware, and that while fitted NEVER
press the onboard reset button
*/

#define PGC PD2
#define PGD PD3
#define MCLR PD4
#define VCC1 PD5
#define VCC2 PD6
#define VCC3 PD7
#define RST = PB0
#define VPP = PB1
int SPKR = 10;
// The default hardware uses an Arduino Nano
// with a built-in LED on PB5 (SCK)
// My "universal" programmer has LEDs on
// PC3 PC4 and PC5
#define LED PC4
#define ERROR_LED PC3

int LEDxx = 0;                      // LED blink counter
int PrAcc = 1;                      // accumulated PrAcc flag

void pic32_icsp_setup() {
    // PGC, PGD & MCLR - open collector /w 3k3 pullup
    PORTD &= ~((1 << PGC) | (1 << PGD) | (1 << MCLR));
    DDRD |= (1 << PGC) | (1 << PGD) | (1 << MCLR);

    /////////////////////////////////////////////////////////////
    //
    // Our Harware doesn't use RST PB0 or VPP PB1
    // Refer back to Arduino sketch, remove these references 
    // And check everything still works
    // If possible, also clarify the function of these pins
    //
    /////////////////////////////////////////////////////////////
    PORTB |= (1 << RST);  // not MCLR (to drive base of NPN OC)
    PORTB &= ~(1 << VPP); // Vpp enable output (use optocoupler)
    PORTC &= ~(1 << LED); // status LED on arduino

    /////////////////////////////////////////////////////////////
    //
    // TODO: Setup serial using AVR registers
    //
    /////////////////////////////////////////////////////////////
    Serial.begin(115200, SERIAL_8N1);
    //  38400, 115200, 230400, 256000, 460800, 921600, 250000, 500000, 1000000
    //          (ok)   (fail)  (fail)                   (ok)    (ok)     (ok)
    //          2:34                                    1:53    1:54     1:54
}

//////////////////////////////////////////////////////////////////
//
// TODO: The default hardware doesn't use the ADC
// verify that this function is used and needed
//
//////////////////////////////////////////////////////////////////
long readVcc() { // Read 1.1V reference against AVcc
    long result;
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ///////////////////////////////////////////////////////////////
    //
    // TODO: 2millisecond pause using timer
    //
    ///////////////////////////////////////////////////////////////
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC)) {};
    result = ADCL;
    result |= ADCH<<8;
    result = 1126400L / result;       // Back-calculate AVcc in mV
    return result;
}

////////////////////////////////////////////////////////////////
//
// TODO: give symbolic values to these binary constants
//
///////////////////////////////////////////////////////////////
int clock1(int D) {
    if (D) {
        DDRD &= B11110111;                     // PGD = hi-Z
    } else {
        DDRD |= B00001000;                     // PGD = 0
    }
    ////////////////////////////////////////////////////////////
    //
    // TODO: delay function using timer (or NOPS??)
    //
    ////////////////////////////////////////////////////////////
    delayMicroseconds(1);
    DDRD &= B11111011;                            // HIGH (via 3k3 pullup)
    delayMicroseconds(1);
    DDRD |= B00000100;                            // LOW
    delayMicroseconds(1);
    int B = ((PIND & B00001000) >> 3);
    return B;
}

int clock4(int TDI, int TMS) {
    // phase 1
    if (TDI) {
        DDRD &= B11110111;                   // PGD = hi-Z
    } else {
        DDRD |= B00001000;                   // PGD = 0
    }
    delayMicroseconds(1);
    DDRD &= B11111011;                            // HIGH (via 3k3 pullup)
    delayMicroseconds(1);
    DDRD |= B00000100;                            // LOW
    delayMicroseconds(1);
    // phase 2
    if (TMS) {
        DDRD &= B11110111;                   // PGD = hi-Z
    } else {
        DDRD |= B00001000;                   // PGD = 0
    }
    delayMicroseconds(1);
    DDRD &= B11111011;                            // HIGH (via 3k3 pullup)
    delayMicroseconds(1);
    DDRD |= B00000100;                            // LOW
    delayMicroseconds(1);
    // phase 3
    DDRD &= B11110111;                            // PGD = hi-Z (input)
    delayMicroseconds(1);
    DDRD &= B11111011;                            // HIGH (via 3k3 pullup)
    delayMicroseconds(1);
    DDRD |= B00000100;                            // LOW
    delayMicroseconds(1);
    // read TDO
    int B = ((PIND & B00001000) >> 3);
    // phase 4
    DDRD &= B11111011;                            // HIGH (via 3k3 pullup)
    delayMicroseconds(1);
    DDRD |= B00000100;                            // LOW
    return B;
}

void handle_command(int I) {
    if (((I >= 'i') && (I <= 'x')) || ((I >= 'I') && (I <= 'X'))) {
        // 4-bit encoding of TDI, TMS = 0
        int J = tolower(I) - 'i';
        int B = 0;
        if (clock4(J & 1, 0)) B |= 1;
        if (clock4(J & 2, 0)) B |= 2;
        if (clock4(J & 4, 0)) B |= 4;
        if (clock4(J & 8, 0)) B |= 8;
        ch = 'I' + B;
        if (isupper(I)) Serial.print(ch);
        return;
    }
 
    switch (char(I)) {
        // 'd','e','f','g': write TDI and TMS, no read back
        case 'd':
            // TDI = 0, TMS = 0, read_flag = 0
            clock4(0, 0);
            return;
        case 'e':
            // TDI = 0, TMS = 1, read_flag = 0
            clock4(0, 1);
            return;
        case 'f':
            // TDI = 1, TMS = 0, read_flag = 0
            clock4(1, 0);
            return;
        case 'g':
            // TDI = 1, TMS = 1, read_flag = 0
            clock4(1, 1);
            return;
        case 'a':
            // TDI = 0, TMS = 1-0-0, read_flag = 0
            // (data header)
            clock4(0, 1);
            clock4(0, 0);
            clock4(0, 0);
            return;
        case 'z':
            // TDI = 0, TMS = 1-0, read_flag = 0
            // (data footer)
            clock4(0, 1);
            clock4(0, 0);
            return;
        // 'D','E','F','G', '+': write TDI and TMS, read back TDO
        case 'D':
            // TDI = 0, TMS = 0, read_flag = 1
            ch = '0' + clock4(0, 0);
            Serial.print(ch);
            return;
        case 'E':
            // TDI = 0, TMS = 1, read_flag = 1
            ch = '0' + clock4(0, 1);
            Serial.print(ch);
            return;
        case 'F':
            // TDI = 1, TMS = 0, read_flag = 1
            ch = '0' + clock4(1, 0);
            Serial.print(ch);
            return;
        case 'G':
            // TDI = 1, TMS = 1, read_flag = 1
            ch = '0' + clock4(1, 1);
            Serial.print(ch);
            return;
        case 'A':
            // TDI = 0, TMS = 1-0-0, read_flag = 1
            clock4(0, 1);
            clock4(0, 0);
            ch = '0' + clock4(0, 0);
            Serial.print(ch);
            return;
        case '+':
            // TDI = 0, TMS = 0, accumulate PrAcc
            if (!clock4(0, 0)) {
                // remember if any error ('0')
                PrAcc = 0;
            }
            return;
        // '>', '.', '=': handshake and formatting commands, placed here for possible speed
        case '>':
            // request a sync response of '<'
            Serial.print('<');
            return;
        case '=':
            // retrieve value of PrAcc
            Serial.print(PrAcc ? '1' : '0');
            // reset to default
            PrAcc = 1; 
            return;
        case '.':
            // no operation, used for formatting
            return;
        // '0','1': used to clock out "MCHP" signature for ICSP entry
        case '0':
            // clock out a 0 bit on PGD pin
            clock1(0);
            return;
        case '1':
            // clock out a 1 bit on PGD pin
            clock1(1);
            return;
        case '-':
            // clock in single PGD bit
            ch = '0' + clock1(1);
            Serial.print(ch);
            return;
        // '2','3': pulse MCLR high, clock out signature, set MCLR high
        case '2': 
            // set MCLR low
            DDRD |= (1 << MCLR);
            return;
        case '3':
            // set MCLR high
            DDRD &= ~(1 << MCLR);
            return;
        // '4','5': control power supply to target
        case '4':
            // Turn power to target OFF
            // PGC = 0, PGD = 0, Hold target in reset
            DDRD |= (1 << PGC) | (1 << PGD) | (1 << MCLR);
            // hi-Z
            DDRD &= ~(1 << VCC1) | (1 << VCC2) | (1 << VCC3);
            return;
        case '5':
             // turn power to target ON
             // PGC = 0, PGD = 0, Hold target in reset
             DDRD |= (1 << PGC) | (1 << PGD) | (1 << MCLR);
             // reset to +5v
             PORTD |= (1 << VCC1) | (1 << VCC2) | (1 << VCC3);
             DDRD |= (1 << VCC1) | (1 << VCC2) | (1 << VCC3);
             return;
        // HV programming commands, for older device families that require Vpp
        case '6':
            // turn OFF Vpp, hold in reset
            digitalWrite(Vpp, LOW);			            // Vpp = 0 (Vpp OFF)
            delay (1);                              // 1mS delay
            digitalWrite(RST, HIGH);                // RST = 1 (hold in reset)
            return;
        case '7':
            // release reset, turn ON Vpp
            digitalWrite(RST, LOW);                 // RST = 0 (release reset)
            delay (1);                              // 1mS delay
            digitalWrite(Vpp, HIGH);                // Vpp = 1 (Vpp ON)
            return;
        // miscellaneous other commands
        case '8':
            // insert 10mS delay
            delay(10);
            return;
        case '@':
            // output analog values
            long Vusb;
            Vusb = readVcc();
            Serial.println(analogRead(A0) * Vusb / 1024);
            Serial.println(analogRead(A1) * Vusb / 1024);
            Serial.println(analogRead(A2) * Vusb / 1024);
            Serial.println(analogRead(A3) * Vusb / 1024);
            Serial.println(analogRead(A4) * Vusb / 1024);
            Serial.println(analogRead(A5) * Vusb / 1024);
            Serial.print((char)0x00);               // null terminated
            return;
        case '?':
            // return ID string, "ascii ICSP v1X"
            Serial.print("ascii ICSP v1X");
            return;
        default:
            // invalid input
            PORTC |= (1 << LED_ERROR);
            return;
    }
}

void pic32_icsp_loop() {
    if (LEDxx == 0) PORTB |= B00100000;           // turn status LED ON
    if (LEDxx == 3) PORTB &= B11011111;           // turn status LED OFF
    LEDxx = ++LEDxx & 0x001F;
    char ch;
    //////////////////////////////////////////////////////////////////
    //
    // TODO: ATMega serial routies
    //
    /////////////////////////////////////////////////////////////////
    while (Serial.available()) {  // loop while data in buffer (buffer size is 64 bytes)
        int I = Serial.read();
        handle_command(I);
    }
}

//  pinMode(pin, OUTPUT);                       // drive pin to set value
//  pinMode(pin, INPUT);                        // hi-Z state
