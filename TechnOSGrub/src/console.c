#include "include/console.h"

#include "include/stdint.h"
#include "include/util.h"
#include "include/vga.h"
#include "stdlib/string.h"
#include "timer/timer.h"
#include "stdlib/stdio.h"

#include "stdlib/string.h"

#define LOG_TOP     1
#define LOG_BOTTOM  22
#define SEP_ROW     22
#define CMD_ROW     23
#define STATUS_ROW  24
#define PROMPT_LEN  2           // "> "
#define CMD_MAX     (width - PROMPT_LEN - 1)

#define localdebug 1

static const char osHeader[] =
    "  PizzaOS v0.01  ";

static const char capsLockLabel[] = "CAPS";

#define COLOR(fg, bg)   ((uint16_t)(((fg) << 8) | ((bg) << 12)))

#define COL_TITLEBAR    COLOR(color8_Bright_White, color8_Blue)
#define COL_LOG         COLOR(color8_Bright_White, color8_Black)
#define COL_SEP         COLOR(color8_Gray,         color8_Gray)
#define COL_PROMPT      COLOR(color8_Light_Cyan,   color8_Black)
#define COL_INPUT       COLOR(color8_Bright_White, color8_Black)
#define COL_STATUS_BG   COLOR(color8_Gray,         color8_Gray)
#define COL_CAPS_ON     COLOR(color8_Black,        color8_Yellow)
#define COL_CAPS_OFF    COLOR(color8_Gray,         color8_Gray)
#define COL_INFO        COLOR(color8_Light_Blue,   color8_Gray)
#define COL_WARN        COLOR(color8_Yellow,       color8_Gray)
#define COL_ERR         COLOR(color8_Light_Red,    color8_Gray)
#define COL_ERR_CONSOLE COLOR(color8_Light_Red,    color8_Black)
#define COL_LANG        COLOR(color8_Bright_White, color8_Blue)

static char  previousCmd[CMD_MAX + 1] = "Ihj? Run a command before nah?";
static char    cmdBuffer[CMD_MAX + 1];
static uint8_t cmdBufferIndex = 0;

static uint16_t logCol = 0;
static uint16_t logRow = LOG_BOTTOM;

uint16_t* const consoleVga = (uint16_t* const) 0xc00B8000;

void* commandCharConsumer;

static void fillRow(uint8_t row, uint16_t color) {
    for (int x = 0; x < width; x++) {
        consoleVga[row * width + x] = ' ' | color;
    }
}

static void clearCols(uint8_t row, uint8_t x1, uint8_t x2, uint16_t color) {
    for (int x = x1; x <= x2; x++) {
        consoleVga[row * width + x] = ' ' | color;
    }
}

static void scrollLog() {
    for (int y = LOG_TOP + 1; y <= LOG_BOTTOM; y++) {
        for (int x = 0; x < width; x++) {
            consoleVga[(y - 1) * width + x] = consoleVga[y * width + x];
        }
    }
    fillRow(LOG_BOTTOM, COL_LOG);
}

static void logWrite(const char* s, uint16_t color) {
    while (*s) {
        if (*s == '\n' || logCol >= (uint16_t)width) {
            logCol = 0;
            if (logRow < LOG_BOTTOM) {
                logRow++;
            } else {
                scrollLog();
            }
            if (*s == '\n') { s++; continue; }
        }
        if (*s == '\r') { logCol = 0; s++; continue; }

        consoleVga[logRow * width + logCol] = *s | color;
        logCol++;
        s++;
    }
    setCursor(logCol, logRow);
}

static void drawTitleBar() {
    int i;
    for (i = 0; osHeader[i]; i++) {
        putToCoords(i, 0, osHeader[i], COL_TITLEBAR);
    }
    for (; i < width; i++) {
        putToCoords(i, 0, ' ', COL_TITLEBAR);
    }
}

static void redrawCmdLine() {
    fillRow(CMD_ROW, COL_INPUT);

    putToCoords(0, CMD_ROW, '>', COL_PROMPT);
    putToCoords(1, CMD_ROW, ' ', COL_PROMPT);

    for (int i = 0; i < cmdBufferIndex; i++) {
        putToCoords(PROMPT_LEN + i, CMD_ROW, cmdBuffer[i], COL_INPUT);
    }

    setCursor(PROMPT_LEN + cmdBufferIndex, CMD_ROW);
}

static void drawStatusBar() {
    fillRow(STATUS_ROW, COL_STATUS_BG);

    putToCoords(1, STATUS_ROW, 'C', COL_CAPS_OFF);
    putToCoords(2, STATUS_ROW, 'A', COL_CAPS_OFF);
    putToCoords(3, STATUS_ROW, 'P', COL_CAPS_OFF);
    putToCoords(4, STATUS_ROW, 'S', COL_CAPS_OFF);

    putToCoords(5, STATUS_ROW, ' ', COL_STATUS_BG);
    putToCoords(6, STATUS_ROW, '|', COL_STATUS_BG);
    putToCoords(7, STATUS_ROW, ' ', COL_STATUS_BG);

    putToCoords(72, STATUS_ROW, ' ', COL_STATUS_BG);
    putToCoords(73, STATUS_ROW, '|', COL_STATUS_BG);
    putToCoords(74, STATUS_ROW, ' ', COL_STATUS_BG);
}

static void writeStatusMsg(const char* msg, uint16_t color) {
    clearCols(STATUS_ROW, 8, 71, COL_STATUS_BG);

    uint32_t len = strlen(msg);
    if (len > 63) len = 63;
    for (uint32_t i = 0; i < len; i++) {
        putToCoords(8 + i, STATUS_ROW, msg[i], color);
    }
}

// Public API

void initConsole() {
    cmdBufferIndex = 0;
    logCol = 0;
    logRow = LOG_BOTTOM;

    for (int y = 0; y < height; y++) fillRow(y, COL_LOG);

    drawTitleBar();
    drawStatusBar();
    redrawCmdLine();

    commandCharConsumer = &consoleCharConsumer;

    enableCursor(14, 15);
}

void* getCharConsumer() {
    return commandCharConsumer;
}


void loadPreviousCmd() {
    cmdBufferIndex = strlen(previousCmd);
    for (int i = 0; i < (CMD_MAX + 1); i++) {
        cmdBuffer[i] = previousCmd[i];
    }

    redrawCmdLine();
}


void consoleCharConsumer(char c) {
    if (c == '\b') {
        if (cmdBufferIndex > 0) {
            cmdBufferIndex--;
            cmdBuffer[cmdBufferIndex] = '\0';
            redrawCmdLine();
        }
        return;
    }

    if (c == '\n' || c == '\r') {

        if (cmdBufferIndex == 0) {
            return;
        }

        cmdBuffer[cmdBufferIndex] = '\0';

        // if (cmdBufferIndex > 0) {                                         // Uncomment to log the command written. Is it or any use though?
        //     logWrite("> ", COL_PROMPT);                                   // Uncomment to log the command written. Is it or any use though?
        //     logWrite(cmdBuffer, COLOR(color8_Light_Cyan, color8_Black));  // Uncomment to log the command written. Is it or any use though?
        //     logWrite("\n", COL_LOG);                                      // Uncomment to log the command written. Is it or any use though?
        //     // TODO: Call cmdExec here?                                   // Uncomment to log the command written. Is it or any use though?
        // }                                                                 // Uncomment to log the command written. Is it or any use though?

        cmdBufferIndex = 0;
        redrawCmdLine();
        drawStatusBar();

        commandExecutor(cmdBuffer);

        for (int i = 0; i < (CMD_MAX + 1); i++) {
            previousCmd[i] = cmdBuffer[i];
        }

        return;
    }

    if (cmdBufferIndex < CMD_MAX) {
        cmdBuffer[cmdBufferIndex] = c;
        cmdBufferIndex++;
        redrawCmdLine();
    } else {
        sendError("Command too long!");
    }
}

void display(const char* s) {
    logWrite(s, COL_LOG);
}

void sendCommand(const char* command) {
    logWrite(command, COL_LOG);
    logWrite("\n", COL_LOG);

    commandExecutor(command);
    setCursor(2, CMD_ROW - 1);
}

void sendInfo(const char* msg) {
    writeStatusMsg(msg, COL_INFO);
}

void sendWarning(const char* msg) {
    writeStatusMsg(msg, COL_WARN);
}

void sendError(const char* msg) {
    writeStatusMsg(msg, COL_ERR);
}

void setCapsLock(bool on) {
    uint16_t color = on ? COL_CAPS_ON : COL_CAPS_OFF;
    putToCoords(1, STATUS_ROW, 'C', color);
    putToCoords(2, STATUS_ROW, 'A', color);
    putToCoords(3, STATUS_ROW, 'P', color);
    putToCoords(4, STATUS_ROW, 'S', color);
}

void clearLogs() {
    for (int y = LOG_TOP; y <= LOG_BOTTOM; y++) fillRow(y, COL_LOG);
    logCol = 0;
    logRow = LOG_BOTTOM;
    setCursor(PROMPT_LEN + cmdBufferIndex, CMD_ROW);
}


// Command execution logic.

void commandExecutor(const char* command) {

    if (command == 0) {
        logWrite("No command specified..?\r\n", COL_ERR_CONSOLE);
        return;
    }

    unsigned int len = strlen(command);
    char buffer[len + 1];

    // buffer = abcdefg0
    // *p          ^


    for (unsigned int i = 0; i <= len; i++)
        buffer[i] = command[i];

    // char *argv[len / 2 + 2];
    char *argv[len + 2];
    unsigned int argc = 0;

    char *p = buffer;

    while (*p != '\0') {

        while (*p == ' ') {
            p++;
        }

        if (*p == '\0') {
            break;
        }

        argv[argc++] = p;

        while (*p != '\0' && *p != ' ') p++;

        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    if (argv[0] == 0) {
        logWrite("No command specified..?\r\n", COL_ERR_CONSOLE);
        return;
    }

    argv[argc] = 0;

    if      (strcmp(argv[0], "clear")) clearLogs();

    else if (strcmp(argv[0], "echo")) {
        logWrite(command+5, COL_LOG);
        if (command[len] != '\n') logWrite("\n", COL_LOG);
    }

    else if (strcmp(argv[0], "uptime")) {

        char bfr[64] = "";
        char bfr2[96] = "";
        char bfr3[96] = "";
        utoa64(getTicks() / getFreq(), bfr);

        strConcat(bfr2, "Current uptime: ", bfr);
        strConcat(bfr3, bfr2, " seconds\n");

        logWrite(bfr3, COL_LOG);
        logWrite("\n", COL_LOG);
    }

    else if (strcmp(argv[0], "help")) {
        logWrite("Available commands:\r\n", COL_PROMPT);
        logWrite("clear   - Clears the console log\r\n", COL_PROMPT);
        logWrite("echo    - Prints a message to the console log\r\n", COL_PROMPT);
        logWrite("help    - Prints this message\r\n", COL_PROMPT);
        logWrite("uptime  - Returns the current uptime (in seconds) or the machine.\r\n", COL_PROMPT);
        logWrite("crashtest - Tries to perform an illegal operation to see how the system reacts.", COL_ERR_CONSOLE);
    }

    else if (strcmp(argv[0], "make")) {
        logWrite("Aw man.. If you are really writing make commands in your own OS,\nit means you are getting tired..\nJust go to sleep. Sh- just- just do it.\r\n", COL_PROMPT);
    }

    else if (strcmp(argv[0], "daniewl")) {
        for (uint32_t i = 0; i<0xFFF; i++) {
            printf("Daniewl? %li\n\r", i);
        }

        logWrite("Daniel is a good boy.\r\n", COL_PROMPT);
        redrawCmdLine();
        drawStatusBar();
        drawTitleBar();
    }

    else if (strcmp(argv[0], "crashtest")) {
        if (!strcmp(argv[1], "--confirm")) {
            logWrite("Please use --confirm to confirm your evil intents.\r\n", COL_ERR_CONSOLE);
            return;
        }

        if (strcmp(argv[2], "--ud2")) {
            sendError("Crashing system with undefined opcode..");
            logWrite("Crashing system with undefined opcode...", COL_ERR_CONSOLE);
            logWrite("\n", COL_ERR_CONSOLE);

            __asm__ volatile("ud2");
        }

        else if (strcmp(argv[2], "--div0")) {
            sendError("Crashing system with division by 0..");
            logWrite("Crashing system with division by 0...", COL_ERR_CONSOLE);
            logWrite("\n", COL_ERR_CONSOLE);

            int x = 1 / 0;
            asm volatile ("div %0" :: "r"(x));
        }

        else {
            sendError("Invalid argument for crashtest command.");
            logWrite("Invalid argument for crashtest command.", COL_ERR_CONSOLE);
        }
    }


    else {
        char b[256];
        strConcat(b, "Unknown command: ", command);

        sendWarning(b);
        logWrite(b, COL_ERR_CONSOLE);
        logWrite("\n", COL_ERR_CONSOLE);
    }

    logWrite("\n", COL_LOG);
}

void displayCriticalError(const char* title, const char* msg)
{
    uint16_t titleLen = strlen(title);
    uint16_t msgLen   = strlen(msg);

    if (titleLen > 58)
        titleLen = 58;

    uint16_t windowWidth = titleLen;
    if (windowWidth < 40)
        windowWidth = 40;

    if (msgLen > windowWidth * 5)
        msgLen = windowWidth * 5;

    uint16_t titleY = 10;
    uint16_t msgStartY = 11;

    uint16_t titleStartX  = (width - titleLen) / 2;
    uint16_t windowStartX = (width - windowWidth) / 2;

    uint16_t msgLines = (msgLen + windowWidth - 1) / windowWidth;

    uint16_t textLeft   = windowStartX;
    uint16_t textRight  = windowStartX + windowWidth - 1;
    uint16_t textTop    = titleY;
    uint16_t textBottom = msgStartY + msgLines - 1;

    uint16_t boxLeft   = textLeft   - 1;
    uint16_t boxRight  = textRight  + 1;
    uint16_t boxTop    = textTop    - 1;
    uint16_t boxBottom = textBottom + 1;

    for (uint16_t y = boxTop; y <= boxBottom; y++) {
        for (uint16_t x = boxLeft; x <= boxRight; x++) {
            putToCoords(x, y, ' ', COL_ERR);
        }
    }

    for (uint16_t x = 0; x < titleLen; x++) {
        putToCoords(titleStartX + x, titleY, title[x], COL_ERR);
    }

    for (uint16_t i = 0; i < msgLen; i++) {
        uint16_t line = msgStartY + (i / windowWidth);
        uint16_t col  = windowStartX + (i % windowWidth);
        putToCoords(col, line, msg[i], COL_ERR);
    }
}