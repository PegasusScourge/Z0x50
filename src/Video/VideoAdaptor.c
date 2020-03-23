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

#define MAIN_WINDOW_TITLE "Z0x50 Emulator"
#define DEFAULT_FONT_FILE "arial.ttf"

sfRenderWindow* mainWindow;
sfVideoMode videoMode;
sfFont* defaultFont;
sfEvent evt;

/********************************************************************

    C-SFML Basic Functions

********************************************************************/

/*
Initialises the videoAdaptor system and opens SFML
*/
bool videoAdaptor_initialise() {
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

    videoAdaptor_pushSplash();
    sfRenderWindow_display(mainWindow);

    // Connect our clock detection
    signals_addListener(&signal_CLCK, &videoAdaptor_onCLCK);

    return true;
}

/*
Destroy the contexts
*/
void videoAdaptor_destroy() {
    sfRenderWindow_close(mainWindow);
    sfRenderWindow_destroy(mainWindow);

    sfFont_destroy(defaultFont);
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
        // Check for events
        while (sfRenderWindow_pollEvent(mainWindow, &evt)) {
            if (evt.type == sfEvtClosed)
                sfRenderWindow_close(mainWindow);
        }

        // Just update for now
        videoAdaptor_pushSplash();
        sfRenderWindow_display(mainWindow);
    }
}

/********************************************************************

    C-SFML Render Functions

********************************************************************/

/*
Renders text on a window
*/
void videoAdaptor_displayText(const char* string, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c) {
    sfText* text = sfText_create();
    sfText_setFont(text, font);
    sfText_setColor(text, c);
    sfText_setString(text, string);
    sfVector2f pos = { (float)x,(float)y };
    sfText_setPosition(text, pos);
    sfText_setCharacterSize(text, size);

    // Render the text
    sfRenderWindow_drawText(window, text, NULL);

    // Destroy the text
    sfText_destroy(text);
}