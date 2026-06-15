#include "keyboard.h"
#include "stdio.h"

#include "../include/util.h"
#include "../include/vga.h"
#include "../interrupts/idt.h"
#include "../include/stdint.h"
#include "../include/console.h"

bool capsOn;
bool capsLock;
bool ctrlPressed;
uint8_t language;

const uint32_t UNKNOWN = 0xFFFFFFFF;
const uint32_t ESC = 0xFFFFFFFF - 1;
const uint32_t CTRL = 0xFFFFFFFF - 2;
const uint32_t LSHFT = 0xFFFFFFFF - 3;
const uint32_t RSHFT = 0xFFFFFFFF - 4;
const uint32_t ALT = 0xFFFFFFFF - 5;
const uint32_t F1 = 0xFFFFFFFF - 6;
const uint32_t F2 = 0xFFFFFFFF - 7;
const uint32_t F3 = 0xFFFFFFFF - 8;
const uint32_t F4 = 0xFFFFFFFF - 9;
const uint32_t F5 = 0xFFFFFFFF - 10;
const uint32_t F6 = 0xFFFFFFFF - 11;
const uint32_t F7 = 0xFFFFFFFF - 12;
const uint32_t F8 = 0xFFFFFFFF - 13;
const uint32_t F9 = 0xFFFFFFFF - 14;
const uint32_t F10 = 0xFFFFFFFF - 15;
const uint32_t F11 = 0xFFFFFFFF - 16;
const uint32_t F12 = 0xFFFFFFFF - 17;
const uint32_t SCRLCK = 0xFFFFFFFF - 18;
const uint32_t HOME = 0xFFFFFFFF - 19;
const uint32_t UP = 0xFFFFFFFF - 20;
const uint32_t LEFT = 0xFFFFFFFF - 21;
const uint32_t RIGHT = 0xFFFFFFFF - 22;
const uint32_t DOWN = 0xFFFFFFFF - 23;
const uint32_t PGUP = 0xFFFFFFFF - 24;
const uint32_t PGDOWN = 0xFFFFFFFF - 25;
const uint32_t END = 0xFFFFFFFF - 26;
const uint32_t INS = 0xFFFFFFFF - 27;
const uint32_t DEL = 0xFFFFFFFF - 28;
const uint32_t CAPS = 0xFFFFFFFF - 29;
const uint32_t NONE = 0xFFFFFFFF - 30;
const uint32_t ALTGR = 0xFFFFFFFF - 31;
const uint32_t NUMLCK = 0xFFFFFFFF - 32;


uint8_t availableLanguagesInternal[2] = {
    0,  // QWERTY, EN-US
    1
};

const uint32_t lowercaseQWERTY[128] = {
UNKNOWN,ESC,'1','2','3','4','5','6','7','8',
'9','0','-','=','\b','\t','q','w','e','r',
't','y','u','i','o','p','[',']','\n',CTRL,
'a','s','d','f','g','h','j','k','l',';',
'\'','`',LSHFT,'\\','z','x','c','v','b','n','m',',',
'.','/',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',LEFT,UNKNOWN,RIGHT,
'+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};

const uint32_t uppercaseQWERTY[128] = {
    UNKNOWN,ESC,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R',
'T','Y','U','I','O','P','{','}','\n',CTRL,'A','S','D','F','G','H','J','K','L',':','"','~',LSHFT,'|','Z','X','C',
'V','B','N','M','<','>','?',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',
LEFT,UNKNOWN,RIGHT,'+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};

// QWERTY UPPER ROW LOWER: `1234567890-=[]'\m,./
// QWERTY UPPER ROW UPPER: `1234567890-=[]'\M,./

// AZERTY UPPER ROW LOWER: ²&é"'(-è_çà)=^$ù*,;:!
// AZERTY UPPER ROW LOWER: ²1234567890°+¨£%µ?./§

const uint32_t lowercaseAZERTY[128] = {UNKNOWN,ESC,
    '&','e','"','\'','(','-','e','_','c','a',')','=','\b','\t','a','z','e','r',
    't','y','u','i','o','p','^','$','\n',CTRL,
    'q','s','d','f','g','h','j','k','l','m',
    'u','*',LSHFT,'<','w','x','c','v','b','n',',',';',
    ':','!',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',LEFT,UNKNOWN,RIGHT, // What are those '*' and '-'?
    '+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
    };

const uint32_t uppercaseAZERTY[128] = {
    UNKNOWN,ESC,'1','2','3','4','5','6','7','8','9','0',CP437_DEGREE,'+','\b','\t','A','Z','E','R',
'T','Y','U','I','O','P','^','$','\n',CTRL,'Q','S','D','F','G','H','J','K','L','M','%','u',LSHFT,'>','W','X','C',
'V','B','N','?','.','/','$',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',
LEFT,UNKNOWN,RIGHT,'+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};


void displayLanguageName() {
    for (int i = 0; i < 5; i++) {
        putToCoords(75+i, 24, getLanguageShortName(language)[i], (color8_Bright_White << 8) | (color8_Blue << 12));
    }
}


char getCharForLanguage(uint8_t language, bool caps, char characterIdx) {
    switch(language) {
        case KB_LANGUAGE_QWERTY: 
            if (caps) return uppercaseQWERTY[characterIdx];
            return           lowercaseQWERTY[characterIdx];
            
        case KB_LANGUAGE_AZERTY: 
            if (caps) return uppercaseAZERTY[characterIdx];
            return           lowercaseAZERTY[characterIdx];
            
        default:
            return getCharForLanguage(KB_LANGUAGE_QWERTY, caps, characterIdx);
    }
}

void initKeyboard() {
    capsOn   = false;
    capsLock = false;
    ctrlPressed = false;
    language = KB_LANGUAGE_AZERTY;

    irq_install_handler(1, &keyboardHandler);
    printf("Initialized Keyboard with default layout: %s", getLanguageName(language));

    setLanguage(language);
}

void setLanguage(uint8_t languageIndex) {
    language = availableLanguagesInternal[languageIndex];


#if newConsole
    char buffer[25] = "";
    strConcat(buffer, "Language set to ", getLanguageName(language));

    sendInfo(buffer);
#else
    printf("\r\nLanguage set to %s\r\n", getLanguageName(language));
#endif


    displayLanguageName();
}

void keyboardHandler(struct InterruptRegisters* regs) {
    char scanCode = inPortB(0x60) & 0x7F; // What key has been pressed.
    char press = inPortB(0x60) & 0x80;    // Pressed down or released.

    // printf("Pressed: %i", scanCode);

    switch(scanCode){
        case 1:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
            break;

#if newConsole
        case 72:
            loadPreviousCmd();
            break;
#endif


        case 42: // Shift.
            if (press == 0) {
                capsOn = true;
            } else {
                capsOn = false;
            }
            break;

        case 58:
            if (!capsLock && press == 0) {
                capsLock = true;
            } else if (capsLock && press == 0) {
                capsLock = false;
            }

#if newConsole
            setCapsLock(capsLock);
#endif
            
            break;

        case 29:
            if (ctrlPressed && press == -128) {
                ctrlPressed = false;
            }
            else if (!ctrlPressed && press == 0) {
                ctrlPressed = true;
            }
            break;

        default:
            if (press == 0) {
                const char c = getCharForLanguage(language, capsOn || capsLock, scanCode);
                if ((c == 'l' || c == 'L') && ctrlPressed) {
                    setLanguage((language + 1) % 2);
                    return;
                }

#if newConsole
                consoleCharConsumer(c);
#else
                printf("%c", c);
#endif

                displayLanguageName();
            }
    }
}