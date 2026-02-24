#pragma once
#include <stdbool.h>

void initConsole();
void display(const char* msg);
void* getCharConsumer();
void consoleCharConsumer(char c);

void sendCommand(const char* command);
void clearLogs();
void commandExecutor(const char* cmd);
void setCapsLock(bool capsLock);

void sendError  (const char* msg);
void sendWarning(const char* msg);
void sendInfo   (const char* msg);
void loadPreviousCmd();

void displayCriticalError(const char* title, const char* msg);