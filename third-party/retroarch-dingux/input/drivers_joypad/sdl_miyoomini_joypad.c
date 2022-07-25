/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2011-2020 - Daniel De Matteis
 *  Copyright (C) 2019-2020 - James Leaver
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdlib.h>

#include <SDL/SDL.h>

#include <libretro.h>

#include "../input_driver.h"
#include "../../config.def.h"
#include "../../tasks/tasks_internal.h"
#include "../../verbosity.h"

#include <unistd.h>
#include <fcntl.h>

/* Simple joypad driver designed to rationalise
 * the bizarre keyboard/gamepad hybrid setup
 * of OpenDingux devices, exclusive for miyoomini */

#define SDL_MIYOOMINI_JOYPAD_NAME "Dingux Gamepad"

/* Vibration time at 100% vibration strength */
#define SDL_MIYOOMINI_RUMBLE_MS   200

/* Uncomment if you want the MENU button to be used exclusively for MENU
#define SDL_MIYOOMINI_HAS_MENU_TOGGLE
*/
/* Miyoomini input map to keyboard keys */
#define SDL_MIYOOMINI_SDLK_X      SDLK_LSHIFT
#define SDL_MIYOOMINI_SDLK_A      SDLK_SPACE
#define SDL_MIYOOMINI_SDLK_B      SDLK_LCTRL
#define SDL_MIYOOMINI_SDLK_Y      SDLK_LALT
#define SDL_MIYOOMINI_SDLK_L      SDLK_e
#define SDL_MIYOOMINI_SDLK_R      SDLK_t
#define SDL_MIYOOMINI_SDLK_L2     SDLK_TAB
#define SDL_MIYOOMINI_SDLK_R2     SDLK_BACKSPACE
#define SDL_MIYOOMINI_SDLK_SELECT SDLK_RCTRL
#define SDL_MIYOOMINI_SDLK_START  SDLK_RETURN
#if (!defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE))
#define SDL_MIYOOMINI_SDLK_L3     SDLK_ESCAPE    /* MENU */
#define SDL_MIYOOMINI_SDLK_R3     SDLK_POWER     /* POWER */
#else
#define SDL_MIYOOMINI_SDLK_L3     SDLK_KP_DIVIDE /* no use */
#define SDL_MIYOOMINI_SDLK_R3     SDLK_KP_PERIOD /* no use */
#define SDL_MIYOOMINI_SDLK_MENU   SDLK_ESCAPE    /* MENU */
#endif
#define SDL_MIYOOMINI_SDLK_UP     SDLK_UP
#define SDL_MIYOOMINI_SDLK_RIGHT  SDLK_RIGHT
#define SDL_MIYOOMINI_SDLK_DOWN   SDLK_DOWN
#define SDL_MIYOOMINI_SDLK_LEFT   SDLK_LEFT

typedef struct {
   uint16_t pad_state;
   bool connected;
#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
   bool menu_toggle;
#endif
   uint32_t rumble_time;
} miyoomini_joypad_t;

static miyoomini_joypad_t miyoomini_joypad;

#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
/* TODO/FIXME - global referenced outside */
extern uint64_t lifecycle_state;
#endif

void miyoomini_rumble(uint16_t strength) {
   static char lastvalue = 0;
   const char str_export[2] = "48";
   const char str_direction[3] = "out";
   char value[1];
   int fd;

   value[0] = (strength == 0 ? 0x31 : 0x30);
   if (lastvalue != value[0]) {
      fd = open("/sys/class/gpio/export", O_WRONLY);
      if (fd > 0) { write(fd, str_export, 2); close(fd); }
      fd = open("/sys/class/gpio/gpio48/direction", O_WRONLY);
      if (fd > 0) { write(fd, str_direction, 3); close(fd); }
      fd = open("/sys/class/gpio/gpio48/value", O_WRONLY);
      if (fd > 0) { write(fd, value, 1); close(fd); }
      lastvalue = value[0];
   }
}

uint32_t miyoomini_rumble_finish(uint32_t interval) {
   miyoomini_rumble(0);
   return 0;
}

static bool sdl_miyoomini_joypad_set_rumble(unsigned pad,
      enum retro_rumble_effect effect, uint16_t strength) {
   if (pad) return false;

   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   if ( (joypad->rumble_time)&&(strength) ) {
      miyoomini_rumble(strength);
      SDL_SetTimer(joypad->rumble_time, miyoomini_rumble_finish);
   }
   return true;
}

static bool sdl_miyoomini_joypad_set_rumble_gain(unsigned pad, unsigned gain) {
   if (pad != 0) return false;
   if (gain > 100) gain = 100;

   /* Set gain (actually rumble_time for miyoomini) */
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   joypad->rumble_time = SDL_MIYOOMINI_RUMBLE_MS * gain / 100;

   return true;
}

static const char *sdl_miyoomini_joypad_name(unsigned port) {
   if (port != 0) return NULL;
   return SDL_MIYOOMINI_JOYPAD_NAME;
}

static void sdl_miyoomini_joypad_connect(void) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;

   /* 'Register' joypad connection via autoconfig task */
   input_autoconfigure_connect(
         sdl_miyoomini_joypad_name(0), /* name */
         NULL,                         /* display_name */
         sdl_dingux_joypad.ident,      /* driver */
         0,                            /* port */
         0,                            /* vid */
         0);                           /* pid */

   joypad->connected = true;
}

static void sdl_miyoomini_joypad_disconnect(void) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;

   if (joypad->connected)
      input_autoconfigure_disconnect(0, sdl_dingux_joypad.ident);

   memset(joypad, 0, sizeof(miyoomini_joypad_t));
}

static void sdl_miyoomini_joypad_destroy(void) {
   SDL_Event event;

   /* Disconnect joypad */
   sdl_miyoomini_joypad_disconnect();

   /* Stop rumble */
   SDL_SetTimer(0, NULL);
   miyoomini_rumble(0);

   /* Flush out all pending events */
   while (SDL_PollEvent(&event));

#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
   BIT64_CLEAR(lifecycle_state, RARCH_MENU_TOGGLE);
#endif
}

static void *sdl_miyoomini_joypad_init(void *data) {
   miyoomini_joypad_t *joypad      = (miyoomini_joypad_t*)&miyoomini_joypad;

   memset(joypad, 0, sizeof(miyoomini_joypad_t));

   /* Init for rumble */
   if (!SDL_WasInit(SDL_INIT_TIMER)) SDL_InitSubSystem(SDL_INIT_TIMER);
   settings_t *settings = config_get_ptr();
   unsigned rumble_gain = settings ? settings->uints.input_rumble_gain
                                   : DEFAULT_RUMBLE_GAIN;
   sdl_miyoomini_joypad_set_rumble_gain(0, rumble_gain);

#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
   BIT64_CLEAR(lifecycle_state, RARCH_MENU_TOGGLE);
#endif
   /* Connect joypad */
   sdl_miyoomini_joypad_connect();

   return (void*)-1;
}

static bool sdl_miyoomini_joypad_query_pad(unsigned port) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   return (port == 0) && joypad->connected;
}

static int32_t sdl_miyoomini_joypad_button(unsigned port, uint16_t joykey) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   if (port != 0) return 0;
   return (joypad->pad_state & (1 << joykey));
}

static void sdl_miyoomini_joypad_get_buttons(unsigned port, input_bits_t *state) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;

   /* Macros require braces here... */
   if (port == 0) {
      BITS_COPY16_PTR(state, joypad->pad_state);
   } else {
      BIT256_CLEAR_ALL_PTR(state);
   }
}

static int16_t sdl_miyoomini_joypad_axis_state(unsigned port, uint32_t joyaxis) { return 0; }

static int16_t sdl_miyoomini_joypad_axis(unsigned port, uint32_t joyaxis) {
   if (port != 0) return 0;
   return sdl_miyoomini_joypad_axis_state(port, joyaxis);
}

static int16_t sdl_miyoomini_joypad_state(
      rarch_joypad_info_t *joypad_info,
      const struct retro_keybind *binds,
      unsigned port) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   uint16_t port_idx       = joypad_info->joy_idx;
   int16_t ret             = 0;
   size_t i;

   if (port_idx != 0) return 0;

   for (i = 0; i < RARCH_FIRST_CUSTOM_BIND; i++) {
      /* Auto-binds are per joypad, not per user. */
      const uint64_t joykey  = (binds[i].joykey != NO_BTN)
         ? binds[i].joykey  : joypad_info->auto_binds[i].joykey;

      if ((uint16_t)joykey != NO_BTN &&
            (joypad->pad_state & (1 << (uint16_t)joykey)))
         ret |= (1 << i);
   }

   return ret;
}

static void sdl_miyoomini_joypad_poll(void) {
   miyoomini_joypad_t *joypad = (miyoomini_joypad_t*)&miyoomini_joypad;
   SDL_Event event;

#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
   /* Note: The menu toggle key is an awkward special
    * case - the press/release events happen almost
    * instantaneously, and since we only sample once
    * per frame the input is often 'missed'.
    * If the toggle key gets pressed, we therefore have
    * to wait until the *next* frame to release it */
   if (joypad->menu_toggle) {
      BIT64_CLEAR(lifecycle_state, RARCH_MENU_TOGGLE);
      joypad->menu_toggle = false;
   }
#endif

   /* All digital inputs map to keyboard keys */
   while (SDL_PollEvent(&event))
   {
      switch (event.type)
      {
         case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
               case SDL_MIYOOMINI_SDLK_X:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_X);
                  break;
               case SDL_MIYOOMINI_SDLK_A:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_A);
                  break;
               case SDL_MIYOOMINI_SDLK_B:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_B);
                  break;
               case SDL_MIYOOMINI_SDLK_Y:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_Y);
                  break;
               case SDL_MIYOOMINI_SDLK_L:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L);
                  break;
               case SDL_MIYOOMINI_SDLK_R:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R);
                  break;
               case SDL_MIYOOMINI_SDLK_L2:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L2);
                  break;
               case SDL_MIYOOMINI_SDLK_R2:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R2);
                  break;
               case SDL_MIYOOMINI_SDLK_SELECT:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_SELECT);
                  break;
               case SDL_MIYOOMINI_SDLK_START:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_START);
                  break;
               case SDL_MIYOOMINI_SDLK_L3:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L3);
                  break;
               case SDL_MIYOOMINI_SDLK_R3:
               case SDLK_UNKNOWN:	// for stockSDL POWER button
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R3);
                  break;
               case SDL_MIYOOMINI_SDLK_UP:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_UP);
                  break;
               case SDL_MIYOOMINI_SDLK_RIGHT:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_RIGHT);
                  break;
               case SDL_MIYOOMINI_SDLK_DOWN:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_DOWN);
                  break;
               case SDL_MIYOOMINI_SDLK_LEFT:
                  BIT16_SET(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_LEFT);
                  break;
#if defined(SDL_MIYOOMINI_HAS_MENU_TOGGLE)
               case SDL_MIYOOMINI_SDLK_MENU:
                  BIT64_SET(lifecycle_state, RARCH_MENU_TOGGLE);
                  joypad->menu_toggle = true;
                  break;
#endif
               default:
                  break;
            }
            break;
         case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
               case SDL_MIYOOMINI_SDLK_X:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_X);
                  break;
               case SDL_MIYOOMINI_SDLK_A:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_A);
                  break;
               case SDL_MIYOOMINI_SDLK_B:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_B);
                  break;
               case SDL_MIYOOMINI_SDLK_Y:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_Y);
                  break;
               case SDL_MIYOOMINI_SDLK_L:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L);
                  break;
               case SDL_MIYOOMINI_SDLK_R:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R);
                  break;
               case SDL_MIYOOMINI_SDLK_L2:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L2);
                  break;
               case SDL_MIYOOMINI_SDLK_R2:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R2);
                  break;
               case SDL_MIYOOMINI_SDLK_SELECT:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_SELECT);
                  break;
               case SDL_MIYOOMINI_SDLK_START:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_START);
                  break;
               case SDL_MIYOOMINI_SDLK_L3:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_L3);
                  break;
               case SDL_MIYOOMINI_SDLK_R3:
               case SDLK_UNKNOWN:	// for stockSDL POWER button
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_R3);
                  break;
               case SDL_MIYOOMINI_SDLK_UP:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_UP);
                  break;
               case SDL_MIYOOMINI_SDLK_RIGHT:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_RIGHT);
                  break;
               case SDL_MIYOOMINI_SDLK_DOWN:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_DOWN);
                  break;
               case SDL_MIYOOMINI_SDLK_LEFT:
                  BIT16_CLEAR(joypad->pad_state, RETRO_DEVICE_ID_JOYPAD_LEFT);
                  break;
               default:
                  break;
            }
            break;
         default:
            break;
      }
   }

}

input_device_driver_t sdl_dingux_joypad = {
   sdl_miyoomini_joypad_init,
   sdl_miyoomini_joypad_query_pad,
   sdl_miyoomini_joypad_destroy,
   sdl_miyoomini_joypad_button,
   sdl_miyoomini_joypad_state,
   sdl_miyoomini_joypad_get_buttons,
   sdl_miyoomini_joypad_axis,
   sdl_miyoomini_joypad_poll,
   sdl_miyoomini_joypad_set_rumble,
   sdl_miyoomini_joypad_set_rumble_gain,
   sdl_miyoomini_joypad_name,
   "sdl_dingux",
};
