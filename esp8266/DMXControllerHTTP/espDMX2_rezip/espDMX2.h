/*
espDMX2 v2 library
Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/espDMX2

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/

#ifndef espDMX2_h
#define espDMX2_h

#include <inttypes.h>
#include "Stream.h"

#define DMX_TX_CONF           0x3c   //SERIAL_8N2
#define DMX_TX_BAUD           250000
#define DMX_FULL_UNI_TIMING   1000   // How often to output full 512 channel universe (in milliseconds)
#define DMX_NO_LED            200
#define DMX_MIN_CHANS	      30     // Minimum channels output = this + DMX_ADD_CHANS
#define DMX_ADD_CHANS         30     // Add extra buffer to the number of channels output
#define byte uint8_t

// DMX states
enum dmx_state {
  DMX_STOP,
  DMX_START,
  DMX_FULL_UNI,
  DMX_DATA,
  DMX_NOT_INIT
};

struct dmx_;
typedef struct dmx_ dmx_t;

class espDMX2: public Stream {
    public:
        espDMX2(uint8_t dmx_nr);

        void begin() {
          begin(DMX_NO_LED);
        }
        void begin(uint8_t ledPin, uint8_t intensity) {
          begin(ledPin);
          ledIntensity(intensity);
        }
        void begin(uint8_t);
        void pause();
        void unPause();
        void end();
        void ledIntensity(uint8_t);
        
        void setChans(byte *data) {
            setChans(data, 512, 1);
        }
        void setChans(byte *data, uint16_t numChans) {
            setChans(data, numChans, 1);
        }
        void setChans(byte*, uint16_t, uint16_t);

	void clearChans();
	byte *getChans();
        uint16_t numChans();
        
        int available(void) override;
        int peek(void) override;
        int read(void) override;
        void flush(void) override;
        size_t write(uint8_t) override;
        
        operator bool() const;
        
        
    private:
        friend void dmx_interrupt_handler(dmx_t* dmx);
        void _tx_empty_irq(void);

        int _dmx_nr;
        dmx_t* _dmx = 0;
};



extern espDMX2 dmxA;
extern espDMX2 dmxB;
extern espDMX2 dmxC;

#endif

