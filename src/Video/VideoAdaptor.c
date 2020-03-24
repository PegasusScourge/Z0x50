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

// Used for timing the functions
// #define _VIDEO_DEBUG

#define MAIN_WINDOW_TITLE "Z0x50 Emulator"
#define DEFAULT_FONT_FILE "arial.ttf"

sfRenderWindow* mainWindow;
sfVideoMode videoMode;
sfFont* defaultFont;
sfEvent evt;

sfText* text;

sfClock* eventClock;
double eventClockResponseTime = 1000.0 / 100.0; // In ms: (1 second / frequency)

sfClock* renderClock;
double renderClockResponseTime = 1000.0 / 60.0;

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
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to create a text object\n");
        return false;
    }

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
    sfRenderWindow_clear(mainWindow, sfColor_fromRGB(0,0,255));
    videoAdaptor_displayText("Z0x50 Splash Screen", mainWindow, 50, 50, 30, defaultFont, sfColor_fromRGB(255, 255, 255));
}

/*
Activates on each clock cycle
*/
void videoAdaptor_onCLCK(bool rising) {
    if (rising) {
        // Check our clock to see if we should respond to events. We only respond at a max frequency of 100Hz to make sure we don't overload the program
        uint32_t elapsedEventTime = sfTime_asMilliseconds(sfClock_getElapsedTime(eventClock));
        if (elapsedEventTime >= eventClockResponseTime) {
            sfClock_restart(eventClock);

            // Check for events
            while (sfRenderWindow_pollEvent(mainWindow, &evt)) {
                if (evt.type == sfEvtClosed)
                    sfRenderWindow_close(mainWindow);
            }

        }
        
        uint32_t elapsedRenderTime = sfTime_asMilliseconds(sfClock_getElapsedTime(renderClock));
        if (elapsedRenderTime >= renderClockResponseTime) {
            sfClock_restart(renderClock);
            // Just update for now
            videoAdaptor_pushSplash();
            videoAdaptor_displayTextFromInt(elapsedEventTime, 10, mainWindow, 50, 100, 24, defaultFont, sfColor_fromRGB(255, 255, 255));
            videoAdaptor_displayTextFromInt(elapsedRenderTime, 10, mainWindow, 50, 130, 24, defaultFont, sfColor_fromRGB(255, 255, 255));
            videoAdaptor_displayTextFromDouble(72.0/0.33, 6, mainWindow, 50, 160, 24, defaultFont, sfColor_fromRGB(255, 255, 255));
            sfRenderWindow_display(mainWindow);
        }
    }
}

/********************************************************************

    C-SFML Render Functions

********************************************************************/

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
    formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" %s %llu micro seconds\n", string, time);
#endif
}

void videoAdaptor_displayTextFromInt(int num, int radix, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    char temp[50];
    _itoa(num, temp, radix);
    videoAdaptor_displayText(temp, window, x, y, size, font, c);
}

void videoAdaptor_displayTextFromDouble(double num, unsigned int numDigits, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    char temp[50];
    _gcvt(num, numDigits, temp);
    // snprintf(temp, 50, "%f", num);
    videoAdaptor_displayText(temp, window, x, y, size, font, c);
}