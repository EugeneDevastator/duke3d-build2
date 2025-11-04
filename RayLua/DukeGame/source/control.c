//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------


#include "control.h"

#include <stdbool.h>
#include <stdint.h>

#include "gamedefs.h"
#include "control.h"

#include <string.h>

#include "config.h"
#include "global.h"
#include "keyboard.h"
//***************************************************************************
// FIXME  These will need to be removed once the buildengine directory
//        structure is figured out and we move to unified SDL codebase
void _joystick_init(){};
void _joystick_deinit(){};
int _joystick_update() {return 1;};
int _joystick_axis(int axis){return 1;};
int _joystick_button(int button){return 1;};
int _joystick_hat(int hat){return 1;};
//#include "buildengine/display.h"
//***************************************************************************

//***************************************************************************
//
// GLOBALS
//
//***************************************************************************
#define MAXJOYHATS 2
bool  CONTROL_RudderEnabled = false;
bool  CONTROL_MousePresent = false;
bool  CONTROL_JoysPresent[ MaxJoys ] ={};
bool  CONTROL_MouseEnabled = false;
bool  CONTROL_JoystickEnabled = false;
uint8_t     CONTROL_JoystickPort;
uint32_t   CONTROL_ButtonState1;
uint32_t   CONTROL_ButtonHeldState1;
uint32_t   CONTROL_ButtonState2;
uint32_t   CONTROL_ButtonHeldState2;

uint32_t   CONTROL_JoyHatState[MAXJOYHATS];

static short mouseButtons = 0;
static short lastmousebuttons = 0;

static short joyHats[MAXJOYHATS];
static short lastjoyHats[MAXJOYHATS];

//***************************************************************************
//
// FUNCTIONS
//
//***************************************************************************

struct _KeyMapping
{
	bool key_active;
	kb_scancode key1;
	kb_scancode key2;
	
	/* other mappings go here */
} KeyMapping[MAXGAMEBUTTONS];

static int32_t MouseMapping[MAXMOUSEBUTTONS];

// Joystick/Gamepad bindings
static int32_t JoyAxisMapping[MAXJOYAXES];
static int32_t JoyHatMapping[MAXJOYHATS][8];
static int32_t JoyButtonMapping[MAXJOYBUTTONS];
static float JoyAnalogScale[MAXJOYAXES];
static int32_t JoyAnalogDeadzone[MAXJOYAXES];

static void SETBUTTON(int i)
{
	int b;
	
	if (i < 32) {
		b = 1 << i;
		
		CONTROL_ButtonState1 |= b;
	} else {
		i -= 32;
		
		b = 1 << i;
		
		CONTROL_ButtonState2 |= b;
	}
}

static void RESBUTTON(int i)
{
	int b;
	
	if (i < 32) {
		b = 1 << i;
		
		CONTROL_ButtonState1 &= ~b;
	} else {
		i -= 32;
		
		b = 1 << i;
		
		CONTROL_ButtonState2 &= ~b;
	}
}

void CONTROL_UpdateKeyboardState(int key, int pressed)
{
	int i;
	
	for (i = 0; i < MAXGAMEBUTTONS; i++) {
		if (KeyMapping[i].key_active == false) {
			continue;
		}
		
		if (KeyMapping[i].key1 == key || 
			KeyMapping[i].key2 == key) {
			
			if (pressed) {
				SETBUTTON(i);
			} else {
				RESBUTTON(i);
			}
		}
	}	
}


// added.  --ryan.
const char *CONTROL_GetMappingName(int32_t which)
{
	if (!KeyMapping[which].key_active)
        return NULL;

	return(KB_ScanCodeToString(KeyMapping[which].key1));
}


void CONTROL_MapKey( int32_t which, kb_scancode key1, kb_scancode key2 )
{
	KeyMapping[which].key_active = true;
	KeyMapping[which].key1 = key1;
	KeyMapping[which].key2 = key2;
}

void CONTROL_MapButton
        (
        int32_t whichfunction,
        int32_t whichbutton,
        bool doubleclicked
        )
{
    if(doubleclicked)
	return; // TODO

    if(whichbutton < 0 || whichbutton >= MAXMOUSEBUTTONS)
	    return;

    MouseMapping[whichbutton] = whichfunction;
}

void CONTROL_MapJoyButton(int32_t whichfunction, int32_t whichbutton, bool doubleclicked)
{
    if(whichbutton < 0 || whichbutton >= MAXJOYBUTTONS)
    {
        return;
    }

    if(doubleclicked)
       return; // TODO

    JoyButtonMapping[whichbutton] = whichfunction;
}

void CONTROL_MapJoyHat(int32_t whichfunction, int32_t whichhat, int32_t whichvalue)
{
    if(whichhat < 0 || whichhat >= MAXJOYHATS)
    {
        return;
    }

    JoyHatMapping[whichhat][whichvalue] = whichfunction;
}

void CONTROL_DefineFlag( int32_t which, bool toggle )
{
	// STUBBED("CONTROL_DefineFlag");
}

bool CONTROL_FlagActive( int32_t which )
{
	STUBBED("CONTROL_FlagActive");
	return false;
}

void CONTROL_ClearAssignments()
{
	STUBBED("CONTROL_ClearAssignments");
}

void CONTROL_GetUserInput( UserInput *info )
{
	STUBBED("CONTROL_GetUserInput");
}

void CONTROL_GetInput( ControlInfo *info )
{
    int32_t sens = CONTROL_GetMouseSensitivity() >> 9;
    int32_t mx, my;
    int i, j;
    memset(info, '\0', sizeof (ControlInfo));

    mx = my = 0;

    MOUSE_GetDelta(&mx,&my);

    info->dyaw = mx * sens;

       switch(ControllerType)
       {
       case controltype_keyboardandjoystick:
               {
               }
               break;
       case controltype_joystickandmouse:
               {
                       // Mouse should use pitch instead of forward movement.
                       info->dpitch = my * sens*2;
               }
               break;
       default:
               {
            // If mouse aim is active
                       if( myaimmode )
                       {  
                               info->dpitch = my * sens*2;
            }
            else
            {
    info->dz = my * sens*2;
            }
               }
               break;
       }

    // TODO: releasing the mouse button does not honor if a keyboard key with
    // the same function is still pressed. how should it?
    for(i=0; i<MAXMOUSEBUTTONS;++i)
    {
	if( MouseMapping[i] != -1 )
	{
	    if(!(lastmousebuttons & (1<<i)) && mouseButtons & (1<<i))
		SETBUTTON(MouseMapping[i]);
	    else if(lastmousebuttons & (1<<i) && !(mouseButtons & (1<<i)))
		RESBUTTON(MouseMapping[i]);
	}
    }
    lastmousebuttons = mouseButtons;

    // update stick state.
    if ((CONTROL_JoystickEnabled) && (_joystick_update()))
    {
               // Check the hats
               JOYSTICK_UpdateHats();

               // TODO: make this NOT use the BUTTON() system for storing the hat input. (requires much game code changing)
               for(i=0; i<MAXJOYHATS; i++)
               {

                       for(j=0; j<8; j++)
                       {
                               if(!(lastjoyHats[i] & (1<<j)) && (joyHats[i] & (1<<j)))
                               {
                                       SETBUTTON(JoyHatMapping[i][j]);
                               }
                               else if((lastjoyHats[i] & (1<<j)) && !(joyHats[i] & (1<<j)))
                               {
                                       RESBUTTON(JoyHatMapping[i][j]);
                               }
                       }

                       lastjoyHats[i] = joyHats[i];
               }
               

        for(i=0; i<MAXJOYAXES;i++)
        {
            switch(JoyAxisMapping[i])
            {
                case analog_turning:
                    {
                     info->dyaw +=  (int32_t)((float)CONTROL_FilterDeadzone
                                    (
                                        _joystick_axis(i),
                                        JoyAnalogDeadzone[i]
                                    ) 
                                        * JoyAnalogScale[i]
                                    );
                    }
                   break;
                case analog_strafing:
                    {
                        info->dx += (int32_t)((float)CONTROL_FilterDeadzone
                                    (
                                        _joystick_axis(i), 
                                        JoyAnalogDeadzone[i]
                                    )
                                        * JoyAnalogScale[i]
                                    );
                                    //printf("Joy %d = %d\n", i, info->dx);
                    }
                   break;
                case analog_lookingupanddown:
                        info->dpitch += (int32_t)((float)CONTROL_FilterDeadzone
                                    (
                                        _joystick_axis(i), 
                                        JoyAnalogDeadzone[i]
                                    )
                                        * JoyAnalogScale[i]
                                    );
                   break;
                case analog_elevation: //STUB
                   break;
                case analog_rolling: //STUB
                   break;
                case analog_moving:
                    {
                        info->dz += (int32_t)((float)CONTROL_FilterDeadzone
                                    (
                                        _joystick_axis(i), 
                                        JoyAnalogDeadzone[i]
                                    )
                                        * JoyAnalogScale[i]
                                    );
                    }
                   break;
                default:
                    break;
            }
        }
        for(i=0; i<MAXJOYBUTTONS;++i)
        {
            if(_joystick_button(i))
            {
                SETBUTTON(JoyButtonMapping[i]);
            }
            else
            {
                RESBUTTON(JoyButtonMapping[i]);
            }
        }
    }
}

void CONTROL_ClearButton( int32_t whichbutton )
{
	RESBUTTON(whichbutton);
}

void CONTROL_ClearUserInput( UserInput *info )
{
	STUBBED("CONTROL_ClearUserInput");
}

void CONTROL_WaitRelease()
{
	STUBBED("CONTROL_WaitRelease");
}

void CONTROL_Ack()
{
	STUBBED("CONTROL_Ack");
}

void CONTROL_CenterJoystick
   (
   void ( *CenterCenter )(),
   void ( *UpperLeft )(),
   void ( *LowerRight )(),
   void ( *CenterThrottle )(),
   void ( *CenterRudder )()
   )
{
	STUBBED("CONTROL_CenterJoystick");
}

static int32_t mouseSensitivity = 0;
int32_t CONTROL_GetMouseSensitivity()
{
    return mouseSensitivity;
}

void CONTROL_SetMouseSensitivity( int32_t newsensitivity )
{
	mouseSensitivity = newsensitivity;
}

void CONTROL_Startup
   (
   controltype which,
   int32_t ( *TimeFunction )(),
   int32_t ticspersecond
   )
{
    int i;

    _joystick_init();

    for (i=0; i < MAXJOYHATS; i++)
    {
	    joyHats[i] = 0;
	    lastjoyHats[i] = 0;
    }
}

void CONTROL_Shutdown()
{
    _joystick_deinit();
}


void CONTROL_MapAnalogAxis
   (
   int32_t whichaxis,
   int32_t whichanalog
   )
{
    if(whichaxis < MAXJOYAXES)
    {
        JoyAxisMapping[whichaxis] = whichanalog;
    }
}


void CONTROL_MapDigitalAxis
   (
   int32_t whichaxis,
   int32_t whichfunction,
   int32_t direction
   )
{
	STUBBED("CONTROL_MapDigitalAxis");
}

void CONTROL_SetAnalogAxisScale
   (
   int32_t whichaxis,
   float axisscale
   )
{
    if(whichaxis < MAXJOYAXES)
    {
        // Set it... make sure we don't let them set it to 0.. div by 0 is bad.
        JoyAnalogScale[whichaxis] = (axisscale == 0) ? 1.0f : axisscale;
    }
}

void CONTROL_SetAnalogAxisDeadzone
   (
   int32_t whichaxis,
   int32_t axisdeadzone
   )
{
    if(whichaxis < MAXJOYAXES)
    {
        // Set it... 
        JoyAnalogDeadzone[whichaxis] = axisdeadzone;
    }
}

int32_t CONTROL_FilterDeadzone
   (
   int32_t axisvalue,
   int32_t axisdeadzone
   )
{
    if((axisvalue < axisdeadzone) && (axisvalue > -axisdeadzone))
    {
        return 0;
    }

    return axisvalue;
}

int32_t CONTROL_GetFilteredAxisValue(int32_t axis)
{
return (int32_t)((float)CONTROL_FilterDeadzone
                                    (
                                        _joystick_axis(axis), 
                                        JoyAnalogDeadzone[axis]
                                    )
                                        * JoyAnalogScale[axis]
                                    );
}

void CONTROL_PrintAxes()
{
	STUBBED("CONTROL_PrintAxes");
}

bool MOUSE_Init()
{
	memset(MouseMapping,-1,sizeof(MouseMapping));
	return true;
}

void    MOUSE_Shutdown()
{
	STUBBED("MOUSE_Shutdown");
}

void    MOUSE_ShowCursor()
{
	STUBBED("MOUSE_ShowCursor");
}

void    MOUSE_HideCursor()
{
	STUBBED("MOUSE_HideCursor");
}

static int32_t mousePositionX = 0;
static int32_t mousePositionY = 0;
static int32_t mouseRelativeX = 0;
static int32_t mouseRelativeY = 0;

static void updateMouse()
{
    // this is in buildengine.
    short x, y;
    getmousevalues(&x, &y, &mouseButtons);

    mouseRelativeX += x;
    mouseRelativeY += y;
    mousePositionX += x;
    mousePositionY += y;
}

int32_t   MOUSE_GetButtons()
{
    updateMouse();
    return ((int32_t) mouseButtons);
}

void    MOUSE_GetPosition( int32_t*x, int32_t*y  )
{
    if (x) *x = mousePositionX;
    if (y) *y = mousePositionY;
}

void    MOUSE_GetDelta( int32_t* x, int32_t* y  )
{
    updateMouse();

    if (x) *x = mouseRelativeX;
    if (y) *y = mouseRelativeY;

    mouseRelativeX = mouseRelativeY = 0;
}

void JOYSTICK_UpdateHats()
{
       int i;

       for(i=0; i<MAXJOYHATS; i++)
       {
               joyHats[i] = _joystick_hat(i);
       }
}

