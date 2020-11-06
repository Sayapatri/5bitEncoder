/**********************************************************
   Sayapatri RTTY ENCODER for ARDUINO UNO
   2019.06.25 v3.1
   5BIT-BAUDOT-CODE, 45.45baud
   LCD 16x2 (SC1602BBWB-XA-GB-G) or 20x4 (UC-204)
/**********************************************************/
 
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <FlexiTimer2.h>
#include <PS2Keyboard.h>
 
LiquidCrystal lcd(4,5,6,7,8,9,10);
const boolean lcdType = 0; // ***** 16x2: 0 / 20x4: 1 *****
 
#define MARK  346   // 346 = (500000 / 1445 Hz)
#define SPACE 392   // 392 = (500000 / 1275 Hz)
 
/*
#define MARK  218   // 218 = (500000 / 2295 Hz)
#define SPACE 235   // 235 = (500000 / 2125 Hz)
*/
 
PS2Keyboard keyboard;
const int DataPin = 2;  //PS/2 DATA
const int IRQpin  = 3;  //PS/2 CLOCK
 
boolean  ddl      = 1;  //DIDDLE ENABLE
boolean  space;
boolean  crLf;
boolean  snd;
int      fig_2;
int      ti;
byte     x;
char     ch;
uint8_t  baudot;
 
void setup()
{
        if(lcdType == 0)
        {
                lcd.begin(16, 2);
        }
        else
        {
                lcd.begin(20, 4);
        }
        lcd.cursor();
        lcd.setCursor(0, 0);
        keyboard.begin(DataPin, IRQpin);
        pinMode(12, OUTPUT);  //--AFSK AUDIO OUTPUT
        pinMode(13, OUTPUT);  //--TTL LEVEL OUTPUT
        digitalWrite(13, 1);
         
        Timer1.initialize();
        Timer1.attachInterrupt(timer1_interrupt, MARK);
         
        FlexiTimer2::set(1, timer2_interrupt);
        FlexiTimer2::start();
}
 
//AFSK TONE GENERATOR
void timer1_interrupt(void)
{
        static boolean toggle;
         
        toggle = toggle ^ 1;
        digitalWrite(12, toggle);
}
 
//5BIT BAUDOT GENERATOR
void timer2_interrupt(void)
{
        static boolean bit1;
        static boolean bit2;
        static boolean bit3;
        static boolean bit4;
        static boolean bit5;
         
        if(snd == 1)
        {
                switch(ti)
                {
                        case 0:
                                digitalWrite(13, 0);
                                Timer1.setPeriod(SPACE);
                                 
                                bit1 = baudot & B00001;
                                bit2 = baudot & B00010;
                                bit3 = baudot & B00100;
                                bit4 = baudot & B01000;
                                bit5 = baudot & B10000;
                                break;
                        case 22:
                                digitalWrite(13, bit1);
                                if(bit1){Timer1.setPeriod(MARK);}
                                    else{Timer1.setPeriod(SPACE);}
                                break;
                        case 44:
                                digitalWrite(13, bit2);
                                if(bit2){Timer1.setPeriod(MARK);}
                                    else{Timer1.setPeriod(SPACE);}
                                break;
                        case 66:
                                digitalWrite(13, bit3);
                                if(bit3){Timer1.setPeriod(MARK);}
                                    else{Timer1.setPeriod(SPACE);}
                                break;
                        case 88:
                                digitalWrite(13, bit4);
                                if(bit4){Timer1.setPeriod(MARK);}
                                    else{Timer1.setPeriod(SPACE);}
                                break;
                        case 110:
                                digitalWrite(13, bit5);
                                if(bit5){Timer1.setPeriod(MARK);}
                                    else{Timer1.setPeriod(SPACE);}
                                break;
                        case 132:
                                digitalWrite(13, 1);
                                Timer1.setPeriod(MARK);
                                break;
                        case 165:
                                snd = 0;
                                break;
                }
                ti++;
        }
}
 
//CONTROLS 16x2 or 20x4 LCD DISPLAY
void lcdOut()
{
        static byte y;
        int i;
         
        if(lcdType == 0) //16x2
        {
 
                static char c[17];
                 
                lcd.print(ch);
                c[x] = ch;
                x++;
                if(x == 16 && y == 0)
                {
                        x = 0; y = 1;
                }
                else if(x == 16 && y == 1)
                {
                        lcd.clear();
                        lcd.noCursor();
                        for(i = 0; i < 16; i++)
                        {
                                lcd.print(c[i]);
                        }
                        lcd.cursor();
                        x = 0; y = 1;
                }
                lcd.setCursor(x, y);
        }
        else //20x4
        {
                static char c[61];
                 
                lcd.print(ch);
                     if(y == 1){c[     x] = ch;}
                else if(y == 2){c[20 + x] = ch;}
                else if(y == 3){c[40 + x] = ch;}
                x++;
                     if(x == 20 && y == 0){x = 0, y = 1;}
                else if(x == 20 && y == 1){x = 0, y = 2;}
                else if(x == 20 && y == 2){x = 0, y = 3;}
                else if(x == 20 && y == 3)
                {
                        lcd.clear();
                        lcd.setCursor(0, 0); for(i = 0; i < 20; i++){lcd.print(c[     i]);}
                        lcd.setCursor(0, 1); for(i = 0; i < 20; i++){lcd.print(c[20 + i]);}
                        lcd.setCursor(0, 2); for(i = 0; i < 20; i++){lcd.print(c[40 + i]);}
                        x = 0, y = 3;
                        for(i = 0; i < 20; i++){c[     i] = c[20 + i];}
                        for(i = 0; i < 20; i++){c[20 + i] = c[40 + i];}
                }
                lcd.setCursor(x, y);
        }
}
 
void lcdCrLf()
{
        byte rest;
         
        if(lcdType == 0)
        {
                rest = (16 - x);
        }
        else
        {
                rest = (20 - x);
        }
        ch = ' ';
        for(int i = 0; i < rest; i++) //FILL THE LINE WITH SPACES, INSTEAD OF CR&LF
        {
                lcdOut();
        }
}
 
//LOOK UP TABLE
void chTable()
{
        fig_2 = -1;
        switch(ch)
        {
                case 'A': baudot = B00011; fig_2 = 0; break;
                case 'B': baudot = B11001; fig_2 = 0; break;
                case 'C': baudot = B01110; fig_2 = 0; break;
                case 'D': baudot = B01001; fig_2 = 0; break;
                case 'E': baudot = B00001; fig_2 = 0; break;
                case 'F': baudot = B01101; fig_2 = 0; break;
                case 'G': baudot = B11010; fig_2 = 0; break;
                case 'H': baudot = B10100; fig_2 = 0; break;
                case 'I': baudot = B00110; fig_2 = 0; break;
                case 'J': baudot = B01011; fig_2 = 0; break;
                case 'K': baudot = B01111; fig_2 = 0; break;
                case 'L': baudot = B10010; fig_2 = 0; break;
                case 'M': baudot = B11100; fig_2 = 0; break;
                case 'N': baudot = B01100; fig_2 = 0; break;
                case 'O': baudot = B11000; fig_2 = 0; break;
                case 'P': baudot = B10110; fig_2 = 0; break;
                case 'Q': baudot = B10111; fig_2 = 0; break;
                case 'R': baudot = B01010; fig_2 = 0; break;
                case 'S': baudot = B00101; fig_2 = 0; break;
                case 'T': baudot = B10000; fig_2 = 0; break;
                case 'U': baudot = B00111; fig_2 = 0; break;
                case 'V': baudot = B11110; fig_2 = 0; break;
                case 'W': baudot = B10011; fig_2 = 0; break;
                case 'X': baudot = B11101; fig_2 = 0; break;
                case 'Y': baudot = B10101; fig_2 = 0; break;
                case 'Z': baudot = B10001; fig_2 = 0; break;
                case '0': baudot = B10110; fig_2 = 1; break;
                case '1': baudot = B10111; fig_2 = 1; break;
                case '2': baudot = B10011; fig_2 = 1; break;
                case '3': baudot = B00001; fig_2 = 1; break;
                case '4': baudot = B01010; fig_2 = 1; break;
                case '5': baudot = B10000; fig_2 = 1; break;
                case '6': baudot = B10101; fig_2 = 1; break;
                case '7': baudot = B00111; fig_2 = 1; break;
                case '8': baudot = B00110; fig_2 = 1; break;
                case '9': baudot = B11000; fig_2 = 1; break;
                case '-': baudot = B00011; fig_2 = 1; break;
                case '?': baudot = B11001; fig_2 = 1; break;
                case ':': baudot = B01110; fig_2 = 1; break;
                case '(': baudot = B01111; fig_2 = 1; break;
                case ')': baudot = B10010; fig_2 = 1; break;
                case '.': baudot = B11100; fig_2 = 1; break;
                case ',': baudot = B01100; fig_2 = 1; break;
                case '/': baudot = B11101; fig_2 = 1; break;
                case '\r':
                        baudot = B01000; //CR
                        crLf = 1;
                        break;
                case ' ':
                        baudot = B00100; //SPACE
                        space = 1;
                        break;
                default:
                        ch = ' ';
                        baudot = B00100; //SPACE
                        space = 1;
                        break;
        }
}
 
void chConvt()
{
        if(ch >= 97 && ch <= 122) //CONVERT LOWER CASE TO UPPER CASE
        {
                ch = ch - 32;
        }
        else if(ch == PS2_TAB       ){ch = '\0';} //IGNORE THESE KEYS
        else if(ch == PS2_BACKSPACE ){ch = '\0';}
        else if(ch == PS2_ESC       ){ch = '\0';}
        else if(ch == PS2_DELETE    ){ch = '\0';}
        else if(ch == PS2_PAGEUP    ){ch = '\0';}
        else if(ch == PS2_PAGEDOWN  ){ch = '\0';}
        else if(ch == PS2_UPARROW   ){ch = '\0';}
        else if(ch == PS2_LEFTARROW ){ch = '\0';}
        else if(ch == PS2_DOWNARROW ){ch = '\0';}
        else if(ch == PS2_RIGHTARROW){ch = '\0';}
        else if(ch == '!'           ){ch = '\0';}
        else if(ch == '"'           ){ch = '\0';}
        else if(ch == '#'           ){ch = '\0';}
        else if(ch == '$'           ){ch = '\0';}
        else if(ch == '%'           ){ch = '\0';}
        else if(ch == '&'           ){ch = '\0';}
        else if(ch == '\''          ){ch = '\0';}
        else if(ch == '='           ){ch = '\0';}
        else if(ch == '^'           ){ch = '\0';}
        else if(ch == '~'           ){ch = '\0';}
        else if(ch == '|'           ){ch = '\0';}
        else if(ch == '@'           ){ch = '\0';}
        else if(ch == '`'           ){ch = '\0';}
        else if(ch == '['           ){ch = '\0';}
        else if(ch == '{'           ){ch = '\0';}
        else if(ch == ';'           ){ch = '\0';}
        else if(ch == '+'           ){ch = '\0';}
        else if(ch == '*'           ){ch = '\0';}
        else if(ch == ']'           ){ch = '\0';}
        else if(ch == '}'           ){ch = '\0';}
        else if(ch == '<'           ){ch = '\0';}
        else if(ch == '>'           ){ch = '\0';}
        else if(ch == '\\'          ){ch = '\0';}
        else if(ch == '_'           ){ch = '\0';}
}
 
void loop()
{
        static boolean  shift;
        static boolean  fig_1;
        static uint8_t  baudot_;
         
        if(snd == 0)
        {
                if(shift == 1) //2ND BYTE AFTER SENDING SHIFT CODE
                {
                        baudot = baudot_; //RESTORE 
                        lcdOut();
                        shift = 0;
                        ti = 0; snd = 1; //SEND(2)
                }
                else if(crLf == 1) //2ND BYTE AFTER SENDING "CR"
                {
                        baudot = B00010; //LF
                        lcdCrLf();
                        crLf = 0;
                        ti = 0; snd = 1; //SEND(2)
                }
                else //SENDING 2ND BYTE DONE
                {
                        if (keyboard.available()) //KEYBOARD IS TYPED
                        {
                                ch = keyboard.read();
                                chConvt();
                                if(ch != '\0')
                                {
                                        chTable();
                                        if(fig_1 == 0 && fig_2 == 1) //SHIFT "UP"
                                        {
                                                baudot_ = baudot; //EVACUATE
                                                baudot = B11011; //SEND FIGURE CODE FIRST
                                                shift = 1;
                                        }
                                        else if(fig_1 == 1 && fig_2 == 0) //SHIFT "DOWN"
                                        {
                                                baudot_ = baudot; //EVACUATE
                                                baudot = B11111; //SEND LETTER CODE FIRST
                                                shift = 1;
                                        }
                                        else if(space == 1 && fig_2 == 1) //FIGURE AFTER SPACE (TX_UOS)
                                        {
                                                baudot_ = baudot; //EVACUATE
                                                baudot = B11011; //SEND FIGURE CODE FIRST
                                                shift = 1;
                                        }
                                        if(shift != 1 && crLf != 1)
                                        {
                                                lcdOut();
                                        }
                                        if(fig_2 == 0 || fig_2 == 1)
                                        {
                                                space = 0;
                                                fig_1 = fig_2; //REGISTER LAST STATE (EXCEPT SPACE, CR&LF)
                                        }
                                        ti = 0; snd = 1; //SEND(1)
                                }
                        }
                        else //NOTHING TO PROCESS, DIDDLE
                        {
                                if(ddl == 1)
                                {
                                        baudot = B11111; //LTR(DIDDLE)
                                        fig_1 = 0;
                                        ti = 0; snd = 1;
                                }
                        }
                }
        }
        delay(5);
}
