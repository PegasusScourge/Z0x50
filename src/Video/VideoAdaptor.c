/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

VideoAdaptor.c : Handles the interface between the system and the graphics for the user

*/

#include "VideoAdaptor.h"
#include "../SysIO/Log.h"
#include "../Signals.h"
#include "../Util/StringUtil.h"
#include "../Memory/MemoryController.h"

// Used for timing the functions
// #define _VIDEO_DEBUG

#define MAIN_WINDOW_TITLE "Z0x50 Emulator"
#define DEFAULT_FONT_FILE "LTYPE.TTF"

sfRenderWindow* mainWindow;
sfVideoMode videoMode;
sfFont* defaultFont;
sfEvent evt;
sfColor clearColor;

sfText* text;
sfRectangleShape* rect;

sfClock* eventClock;
double eventClockResponseTime = 1000.0 / 100.0; // In ms: (1 second / frequency)

sfClock* renderClock;
double renderClockResponseTime = 1000.0 / 60.0;

DisplayInf_t displayInfo;

void (*currentScreen)() = NULL;
bool currentScreenInit = true;

const uint8_t defaultZero8 = 0;
const uint16_t defaultZero16 = 0;

#ifdef _VIDEO_DEBUG
sfClock* debugTimerClock;
#endif

/********************************************************************

    C-SFML Basic Functions

********************************************************************/

/*
Initialises the videoAdaptor system and opens SFML
*/
bool videoAdaptor_initialise() {
#ifdef _VIDEO_DEBUG
    debugTimerClock = sfClock_create();
#endif

    // Init pointers of registers
    displayInfo.AReg = &defaultZero8;
    displayInfo.BCReg = &defaultZero16;
    displayInfo.DEReg = &defaultZero16;
    displayInfo.HLReg = &defaultZero16;
    displayInfo.IX = &defaultZero16;
    displayInfo.IY = &defaultZero16;
    displayInfo.PC = &defaultZero16;
    displayInfo.SP = &defaultZero16;
    displayInfo.FReg = &defaultZero8;

    videoMode.bitsPerPixel = 8;
    videoMode.width = 800;
    videoMode.height = 600;

    // Initialise SFML
    mainWindow = sfRenderWindow_create(videoMode, MAIN_WINDOW_TITLE, sfClose, NULL);
    if (mainWindow == NULL) {
        // We failed!
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to open the C-SFML window\n");
        return false;
    }

    defaultFont = sfFont_createFromFile(DEFAULT_FONT_FILE);
    if (defaultFont == NULL) {
        // Failed to create font
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to open the C-SFML font from file %s\n", DEFAULT_FONT_FILE);
        return false;
    }

    text = sfText_create();
    if (text == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to create text object\n");
        return false;
    }

    rect = sfRectangleShape_create();
    if (rect == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to create rect object\n");
        return false;
    }

    clearColor = sfColor_fromRGB(0, 0, 255);
    videoAdaptor_pushSplash();
    sfRenderWindow_display(mainWindow);
    sfRenderWindow_setFramerateLimit(mainWindow, 100);

    // Connect our clock detection
    signals_addListener(&signal_CLCK, &videoAdaptor_onCLCK);

    eventClock = sfClock_create();
    renderClock = sfClock_create();

    return true;
}

/*
Destroy the contexts
*/
void videoAdaptor_destroy() {
    // Destroy the text
    sfText_destroy(text);

    sfRenderWindow_close(mainWindow);
    sfRenderWindow_destroy(mainWindow);

    sfFont_destroy(defaultFont);

#ifdef _VIDEO_DEBUG
    sfClock_destroy(debugTimerClock);
#endif
    sfClock_destroy(eventClock);
    sfClock_destroy(renderClock);
}

/*
Displays the videoAdaptor splash screen
*/
void videoAdaptor_pushSplash() {
    videoAdaptor_displayText("Z0x50 Splash Screen", mainWindow, 50, 50, 30, defaultFont, sfColor_fromRGB(255, 255, 255));
}

/*
Activates on each clock cycle
*/
void videoAdaptor_onCLCK(bool rising) {
    if (rising) {
        // Check the log hasn't exceeded our desired length

        // Check our clock to see if we should respond to events. We only respond at a max frequency of 100Hz to make sure we don't overload the program
        uint32_t elapsedEventTime = sfTime_asMilliseconds(sfClock_getElapsedTime(eventClock));
        if (elapsedEventTime >= eventClockResponseTime) {
            sfClock_restart(eventClock);

            // Check for events
            while (sfRenderWindow_pollEvent(mainWindow, &evt)) {
                //if (evt.type == sfEvtClosed)
                //    sfRenderWindow_close(mainWindow);
            }

        }
        
        uint32_t elapsedRenderTime = sfTime_asMilliseconds(sfClock_getElapsedTime(renderClock));
        if (elapsedRenderTime >= renderClockResponseTime) {
            sfClock_restart(renderClock);
            // Just update for now
            sfRenderWindow_clear(mainWindow, clearColor);
            // Display the output of the system
            // TODO
            // Display the UI
            if (currentScreen == NULL) {
                videoAdaptor_pushSplash();
            }
            else {
                currentScreen();
            }
            
            sfRenderWindow_display(mainWindow);
        }
    }
}

/********************************************************************

    C-SFML Render Functions

********************************************************************/

/*
Renders a rectangle on a window
*/
void videoAdaptor_displayRect(sfRenderWindow* window, sfVector2f pos, sfVector2f size, sfColor fillColor, sfColor outlineColor, float outlineThickness) {
    sfRectangleShape_setFillColor(rect, fillColor);
    sfRectangleShape_setOutlineColor(rect, outlineColor);
    sfRectangleShape_setOutlineThickness(rect, outlineThickness);
    sfRectangleShape_setPosition(rect, pos);
    sfRectangleShape_setSize(rect, size);
    sfRenderWindow_drawRectangleShape(window, rect, NULL);
}

/*
Renders text on a window
*/
void videoAdaptor_displayText(const char* string, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
#ifdef _VIDEO_DEBUG
    sfClock_restart(debugTimerClock);
#endif
    
    sfText_setFont(text, font);
    sfText_setColor(text, c);
    sfText_setString(text, string);
    sfVector2f pos = { (float)x,(float)y };
    sfText_setPosition(text, pos);
    sfText_setCharacterSize(text, size);

    // Render the text
    sfRenderWindow_drawText(window, text, NULL);

#ifdef _VIDEO_DEBUG
    uint64_t time = sfTime_asMicroseconds(sfClock_getElapsedTime(debugTimerClock));
    printf(__FUNCTION__" %s %llu micro seconds\n", string, time);
#endif
}

void videoAdaptor_displayTextFromInt(int num, int radix, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    char temp[50];
    _itoa(num, temp, radix);
    videoAdaptor_displayText(temp, window, x, y, size, font, c);
}

void videoAdaptor_displayTextWithFmt(int num, const char* fmt, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    char temp[50];
    snprintf(temp, 50, fmt, num);
    videoAdaptor_displayText(temp, window, x, y, size, font, c);
}

void videoAdaptor_displayTextFromDouble(double num, unsigned int numDigits, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    if (numDigits > 49) {
        // Fail!
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to draw double with %i digits, which is more than 49\n", numDigits);
        numDigits = 49;
    }
    
    char temp[50];
    _gcvt(num, numDigits, temp);
    // snprintf(temp, 50, "%f", num);
    videoAdaptor_displayText(temp, window, x, y, size, font, c);
}

/********************************************************************

    C-SFML UI Screens

********************************************************************/

void videoAdaptor_setUIScreen(void (*newScreen)()) {
    currentScreenInit = true;
    currentScreen = newScreen;
}

void videoAdaptor_screenAllStats() {
    if (currentScreenInit) {
        currentScreenInit = false;
        // Initialise information about this screen
        clearColor = sfColor_fromRGB(0, 0, 0);
    }

    // Display the information
    int y = 20; int size = 15;
    videoAdaptor_displayText("Registers", mainWindow, 2, y, size, defaultFont, sfCyan); y += 20;
    videoAdaptor_displayText("A:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.AReg, "%02X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("BC:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.BCReg, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("DE:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.DEReg, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("HL:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.HLReg, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("IX:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.IX, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("IY:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.IY, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("PC:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.PC, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("SP:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayTextWithFmt(*displayInfo.SP, "%04X", mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;
    char buff[9]; sutil_byteToBinary(*displayInfo.FReg, buff, 9);
    videoAdaptor_displayText("F:", mainWindow, 2, y, size, defaultFont, sfWhite); videoAdaptor_displayText(buff, mainWindow, 40, y, size, defaultFont, sfWhite); y += 15;

    // Display signals
    y = 200; size = 10; sfVector2f rpos = { 70,240 }; sfVector2f rsize = { 80, 5 };
    videoAdaptor_displayText("Signals", mainWindow, 2, y, size, defaultFont, sfCyan); y += 16; rpos.y = (float)y + ((float)size / 2.5f);
    
    videoAdaptor_displayText("M1", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_M1) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("MREQ", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_MREQ) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("IORQ", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_IORQ) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("RD", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_RD) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("WR", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_WR) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("HALT", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_HALT) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("WAIT", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayRect(mainWindow, rpos, rsize, signals_readSignal(&signal_WAIT) ? sfGreen : sfRed, sfWhite, 0); y += 11; rpos.y = (float)y + ((float)size / 2.5f);

    videoAdaptor_displayText("Data:", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayTextWithFmt(signal_dataBus, "%02X", mainWindow, 70, y, size, defaultFont, sfWhite); y += 11;

    videoAdaptor_displayText("Address:", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayTextWithFmt(signal_addressBus, "%04X", mainWindow, 70, y, size, defaultFont, sfWhite); y += 11;

    // Display the instruction
    y = 340; size = 15;
    videoAdaptor_displayText("Current Instruction", mainWindow, 2, y, size, defaultFont, sfCyan); y += 20;
    videoAdaptor_displayText("Pre/Inst/$:", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayTextWithFmt(displayInfo.cInstr->prefix, "%04X", mainWindow, 130, y, size, defaultFont, displayInfo.cInstr->detectedPrefix ? sfBlue : sfRed);
    videoAdaptor_displayTextWithFmt(displayInfo.cInstr->opcode, "%02X", mainWindow, 170, y, size, defaultFont, sfWhite);
    videoAdaptor_displayTextWithFmt(displayInfo.cInstr->operand1, "%02X", mainWindow, 195, y, size, defaultFont, sfWhite);
    videoAdaptor_displayTextWithFmt(displayInfo.cInstr->operand0, "%02X", mainWindow, 215, y, size, defaultFont, sfWhite); y += 15;
    videoAdaptor_displayText("Instr Text:", mainWindow, 2, y, size, defaultFont, sfWhite);
    videoAdaptor_displayText(displayInfo.cInstr->string, mainWindow, 130, y, size, defaultFont, sfWhite);

    // Display memory content around PC
    y = 410; size = 10; int x = 2;
    unsigned int pc = *displayInfo.PC;
    videoAdaptor_displayText("Memory Around PC", mainWindow, 2, y, size, defaultFont, sfCyan); y += 12;
    for (int i = -8; i < 24; i++) {
        if (displayInfo.cInstr->detectedPrefix && (i == 0 || i == -1)) {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(pc + i), "%02X", mainWindow, x, y, size, defaultFont, sfBlue);
        }
        else if (i == (0 + displayInfo.cInstr->detectedPrefix)) {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(pc + i), "%02X", mainWindow, x, y, size, defaultFont, sfGreen);
        }
        else if (i == (displayInfo.cInstr->numOperands + displayInfo.cInstr->detectedPrefix) && displayInfo.cInstr->numOperands > 0) {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(pc + i), "%02X", mainWindow, x, y, size, defaultFont, sfColor_fromRGB(255, 0, 255));
        }
        else {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(pc + i), "%02X", mainWindow, x, y, size, defaultFont, sfWhite);
        }
        x += 22;
    }

    // Display memory content around SP
    y = 440; size = 10; x = 2;
    unsigned int sp = *displayInfo.SP;
    videoAdaptor_displayText("Memory Around SP", mainWindow, 2, y, size, defaultFont, sfCyan); y += 12;
    for (int i = -8; i < 24; i++) {
        if (i == 0) {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(sp + i), "%02X", mainWindow, x, y, size, defaultFont, sfGreen);
        }
        else {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(sp + i), "%02X", mainWindow, x, y, size, defaultFont, sfWhite);
        }
        x += 22;
    }

    // Display memory content around addressBus
    y = 470; size = 10; x = 2;
    videoAdaptor_displayText("Memory Around Address Bus", mainWindow, 2, y, size, defaultFont, sfCyan); y += 12;
    for (int i = -8; i < 24; i++) {
        if (i == 0) {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(signal_addressBus + i), "%02X", mainWindow, x, y, size, defaultFont, sfGreen);
        }
        else {
            videoAdaptor_displayTextWithFmt(memoryController_rawRead(signal_addressBus + i), "%02X", mainWindow, x, y, size, defaultFont, sfWhite);
        }
        x += 22;
    }
}