#pragma once

#include "../include/util.h"
#include "../include/stdint.h"
#include "stdbool.h"

#define CP437_DEGREE ((char)0xF8)

void initKeyboard();
void keyboardHandler( struct InterruptRegisters* regs);
char getCharForLanguage(uint8_t language, bool caps, char characterIdx);
void setLanguage(uint8_t languageIndex);

#define KB_LANGUAGE_QWERTY 0
#define KB_LANGUAGE_AZERTY 1
#define KB_LANGUAGE_UNDEF  256

inline const char* getLanguageName(uint8_t language) {
    switch(language) {
        case KB_LANGUAGE_QWERTY: return "QWERTY: EN-US";
        case KB_LANGUAGE_AZERTY: return "AZERTY: FR-FR";
        default: return "Unknown, defaulting to QWERTY: ?" "?-US";
    }
}

inline const char* getLanguageShortName(uint8_t language) {
    switch(language) {
        case KB_LANGUAGE_QWERTY: return "EN-US";
        case KB_LANGUAGE_AZERTY: return "FR-FR";
        default: return "?" "?-EN";
    }
}