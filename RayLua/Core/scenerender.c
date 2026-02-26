//
// Created by omnis on 11/25/2025.
//

#include "scenerender.h"

#include <stdbool.h>

#include "shadowtest2.h"
#include "mapcore.h"
static bool useLights = false;

void setLightOption(bool isenabled) {
	useLights=isenabled;
}
