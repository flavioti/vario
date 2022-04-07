#ifndef _COPILOTSDCARD_H_
#define _COPILOTSDCARD_H_

#include "FS.h"
#include "SD.h"
#include "SPI.h"

bool initSDCard();
void testFileIO(fs::FS &fs, const char *path);


#endif