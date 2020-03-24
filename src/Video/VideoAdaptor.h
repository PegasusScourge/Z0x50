#pragma once

/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

VideoAdaptor.h : Handles the interface between the system and the graphics for the user

*/

#include <stdbool.h>
#include <stdint.h>

#include "SFML/Graphics.h"

extern sfRenderWindow* mainWindow;
extern sfFont* defaultFont;

/********************************************************************

    C-SFML Basic Functions

********************************************************************/

bool videoAdaptor_initialise();
void videoAdaptor_destroy();
void videoAdaptor_pushSplash();
void videoAdaptor_onCLCK(bool rising);

/********************************************************************

    C-SFML Render Functions

********************************************************************/

void videoAdaptor_displayText(const char* string, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c);
void videoAdaptor_displayTextFromInt(int num, int radix, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c);
void videoAdaptor_displayTextFromDouble(double num, unsigned int numDigits, sfRenderWindow* window, unsigned int x, unsigned int y, unsigned int size, sfFont* font, sfColor c);