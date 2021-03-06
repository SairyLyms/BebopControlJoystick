/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file ihm.c
 * @brief This file contains sources about ncurses IHM used by arsdk example "BebopSample"
 * @date 15/01/2015
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdlib.h>
#include <curses.h>
#include <string.h>

#include <libARSAL/ARSAL.h>

#include "ihm.h"

#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define HEADER_X 0
#define HEADER_Y 0

#define INFO_X 0
#define INFO_Y 2

#define BATTERY_X 0
#define BATTERY_Y 4

#define JOYVALUE_X 0
#define JOYVALUE_Y 6

#undef main
/*****************************************
 *
 *             private header:
 *
 ****************************************/
void *IHM_InputProcessing(void *data);
extern input_t joyinput;
extern state_t state;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

  /*****************************************
   *
   *             implementation :
   *
   *****************************************/

  IHM_t *IHM_New (IHM_onInputEvent_t onInputEventCallback)
  {
      int failed = 0;
      IHM_t *newIHM = NULL;

      // check parameters
      if (onInputEventCallback == NULL)
      {
          failed = 1;
      }

      if (!failed)
      {
          //  Initialize IHM
          newIHM = malloc(sizeof(IHM_t));
          if (newIHM != NULL)
          {
              //  Initialize ncurses
              newIHM->mainWindow = initscr();
              newIHM->inputThread = NULL;
              newIHM->run = 1;
              newIHM->onInputEventCallback = onInputEventCallback;
              newIHM->customData = NULL;
          }
          else
          {
              failed = 1;
          }
      }

      if (!failed)
      {
          raw();                  // Line buffering disabled
          noecho();               // Don't echo() while we do getch
          refresh();
      }

      if (!failed)
      {
          //start input thread
          if(ARSAL_Thread_Create(&(newIHM->inputThread), IHM_InputProcessing, newIHM) != 0)
          {
              failed = 1;
          }
      }

      if (failed)
      {
          IHM_Delete (&newIHM);
      }

      return  newIHM;
  }

  void IHM_Delete (IHM_t **ihm)
  {
      //  Clean up

      if (ihm != NULL)
      {
          if ((*ihm) != NULL)
          {
              (*ihm)->run = 0;

              if ((*ihm)->inputThread != NULL)
              {
                  ARSAL_Thread_Join((*ihm)->inputThread, NULL);
                  ARSAL_Thread_Destroy(&((*ihm)->inputThread));
                  (*ihm)->inputThread = NULL;
              }

              delwin((*ihm)->mainWindow);
              (*ihm)->mainWindow = NULL;
              endwin();
              refresh();

              free (*ihm);
              (*ihm) = NULL;
          }
      }
  }

  void IHM_setCustomData(IHM_t *ihm, void *customData)
  {
      if (ihm != NULL)
      {
          ihm->customData = customData;
      }
  }

  void *IHM_InputProcessing(void *data)
  {
      IHM_t *ihm = (IHM_t *) data;
      int key = 0;

      if (ihm != NULL)
      {
          while (ihm->run)
          {
              if ((key == 27) || (key =='q'))
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_EXIT, ihm->customData);
                  }
              }
              if(key == 'e')
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_EMERGENCY, ihm->customData);
                  }
              }
              if(joyinput.takeoff)
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_TAKEOFF, ihm->customData);
                  }
              }
              if(joyinput.landing)
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_LAND, ihm->customData);
                  }
              }
              if(joyinput.shot)
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_CAMERA_SHOT, ihm->customData);
                  }
              }
              if((joyinput.trig && (joyinput.pitch || joyinput.roll)) || joyinput.yaw || joyinput.gaz)
              {
                  if(ihm->onInputEventCallback != NULL)
                  {
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_MOVE, ihm->customData);
                  }
              }
              else{
                      ihm->onInputEventCallback (IHM_INPUT_EVENT_NONE, ihm->customData);
              }
              if(ihm->onInputEventCallback != NULL)
              {
                  ihm->onInputEventCallback (IHM_INPUT_EVENT_CAMERA_DIR, ihm->customData);
              }
              usleep(10000);
          }
      }

      return NULL;
  }

  void IHM_PrintHeader(IHM_t *ihm, char *headerStr)
  {
      if (ihm != NULL)
      {
          move(HEADER_Y, 0);   // move to begining of line
          clrtoeol();          // clear line
          mvprintw(HEADER_Y, HEADER_X, headerStr);
      }
  }

  void IHM_PrintInfo(IHM_t *ihm, char *infoStr)
  {
      if (ihm != NULL)
      {
          move(INFO_Y, 0);    // move to begining of line
          clrtoeol();         // clear line
          mvprintw(INFO_Y, INFO_X, infoStr);
          refresh();
      }
  }

  void IHM_PrintBatteryState(IHM_t *ihm, uint8_t percent)
  {
      if (ihm != NULL)
      {
          move(BATTERY_Y, 0);     // move to begining of line
          clrtoeol();             // clear line
          mvprintw(BATTERY_Y, BATTERY_X, "Battery: %d", percent);
          refresh();
      }
  }


  void IHM_PrintStateinfo(IHM_t *ihm)
    {
        if (ihm != NULL)
        {
            move(JOYVALUE_Y, 0);     // move to begining of line
            clrtoeol();             // clear line
            mvprintw(JOYVALUE_Y, JOYVALUE_X, "Pitch: %d,Roll: %d,Yaw: %d,UP: %d,DOWN: %d, View:(x: %d,y: %d), Debug:%x", joyinput.pitch,joyinput.roll,joyinput.yaw,joyinput.up,joyinput.down,joyinput.pan,joyinput.tilt,joyinput.debug);
            move(JOYVALUE_Y+2, 0);     // move to begining of line
            clrtoeol();             // clear line
            mvprintw(JOYVALUE_Y+2, JOYVALUE_X, "Speed: %4.2f km/h,Alt: %4.2f",state.speed,state.altitude);
            refresh();
        }
    }
