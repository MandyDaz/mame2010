/***************************************************************************

    Sega Zaxxon hardware

****************************************************************************

    Sound interface is provided by an 8255. The 8255 is a parallel peripheral
    interface, used also in Scramble. It has three 8-bit outputs.
    All sounds are generated by discrete circuits. Each sound is triggered by
    an output pin of the 8255.

    Zaxxon Sound Information: (from the schematics)
    by Frank Palazzolo

    There are four registers in the 8255. they are mapped to
    (111x xxxx 0011 11pp) by Zaxxon.  Zaxxon writes to these
    at FF3C-FF3F.

    There are three modes of the 8255, but by the schematics I
    can see that Zaxxon is using "Mode 0", which is very simple.

    Important Note:
    These are all Active-Low outputs.
    A 1 De-activates the sound, while a 0 Activates/Triggers it

    Port A Output:
    FF3C bit7 Battleship
         bit6 Laser
         bit5 Base Missle
         bit4 Homing Missle
         bit3 Player Ship D
         bit2 Player Ship C
         bit1 Player Ship B
         bit0 Player Ship A

    Port B Output:
    FF3D bit7 Cannon
         bit6 N/C
         bit5 M-Exp
         bit4 S-Exp
         bit3 N/C
         bit2 N/C
         bit1 N/C
         bit0 N/C

    Port C Output:
    FF3E bit7 N/C
         bit6 N/C
         bit5 N/C
         bit4 N/C
         bit3 Alarm 3
         bit2 Alarm 2
         bit1 N/C
         bit0 Shot

    Control Byte:
    FF3F Should be written an 0x80 for Mode 0
         (Very Simple) operation of the 8255

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/zaxxon.h"



/*************************************
 *
 *  Zaxxon sound hardware description
 *
 *************************************/

static const char *const zaxxon_sample_names[] =
{
	"*zaxxon",
	"03.wav",   /* 0 - Homing Missile */
	"02.wav",   /* 1 - Base Missile */
	"01.wav",   /* 2 - Laser (force field) */
	"00.wav",   /* 3 - Battleship (end of level boss) */
	"11.wav",   /* 4 - S-Exp (enemy explosion) */
	"10.wav",   /* 5 - M-Exp (ship explosion) */
	"08.wav",   /* 6 - Cannon (ship fire) */
	"23.wav",   /* 7 - Shot (enemy fire) */
	"21.wav",   /* 8 - Alarm 2 (target lock) */
	"20.wav",   /* 9 - Alarm 3 (low fuel) */
	"05.wav",   /* 10 - initial background noise */
	"04.wav",   /* 11 - looped asteroid noise */
	0
};


static const samples_interface zaxxon_samples_interface =
{
	12,
	zaxxon_sample_names
};


MACHINE_DRIVER_START( zaxxon_samples )
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(zaxxon_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/*************************************
 *
 *  Zaxxon PPI write handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( zaxxon_sound_a_w )
{
	zaxxon_state *state = (zaxxon_state *)device->machine->driver_data;
	running_device *samples = device->machine->device("samples");
	UINT8 diff = data ^ state->sound_state[0];
	state->sound_state[0] = data;

	/* PLAYER SHIP A/B: volume */
	sample_set_volume(samples, 10, 0.5 + 0.157 * (data & 0x03));
	sample_set_volume(samples, 11, 0.5 + 0.157 * (data & 0x03));

	/* PLAYER SHIP C: channel 10 */
	if ((diff & 0x04) && !(data & 0x04)) sample_start(samples, 10, 10, TRUE);
	if ((diff & 0x04) &&  (data & 0x04)) sample_stop(samples, 10);

	/* PLAYER SHIP D: channel 11 */
	if ((diff & 0x08) && !(data & 0x08)) sample_start(samples, 11, 11, TRUE);
	if ((diff & 0x08) &&  (data & 0x08)) sample_stop(samples, 11);

	/* HOMING MISSILE: channel 0 */
	if ((diff & 0x10) && !(data & 0x10)) sample_start(samples, 0, 0, TRUE);
	if ((diff & 0x10) &&  (data & 0x10)) sample_stop(samples, 0);

	/* BASE MISSILE: channel 1 */
	if ((diff & 0x20) && !(data & 0x20)) sample_start(samples, 1, 1, FALSE);

	/* LASER: channel 2 */
	if ((diff & 0x40) && !(data & 0x40)) sample_start(samples, 2, 2, TRUE);
	if ((diff & 0x40) &&  (data & 0x40)) sample_stop(samples, 2);

	/* BATTLESHIP: channel 3 */
	if ((diff & 0x80) && !(data & 0x80)) sample_start(samples, 3, 3, TRUE);
	if ((diff & 0x80) &&  (data & 0x80)) sample_stop(samples, 3);
}


WRITE8_DEVICE_HANDLER( zaxxon_sound_b_w )
{
	zaxxon_state *state = (zaxxon_state *)device->machine->driver_data;
	running_device *samples = device->machine->device("samples");
	UINT8 diff = data ^ state->sound_state[1];
	state->sound_state[1] = data;

	/* S-EXP: channel 4 */
	if ((diff & 0x10) && !(data & 0x10)) sample_start(samples, 4, 4, FALSE);

	/* M-EXP: channel 5 */
	if ((diff & 0x20) && !(data & 0x20) && !sample_playing(samples, 5)) sample_start(samples, 5, 5, FALSE);

	/* CANNON: channel 6 */
	if ((diff & 0x80) && !(data & 0x80)) sample_start(samples, 6, 6, FALSE);
}


WRITE8_DEVICE_HANDLER( zaxxon_sound_c_w )
{
	zaxxon_state *state = (zaxxon_state *)device->machine->driver_data;
	running_device *samples = device->machine->device("samples");
	UINT8 diff = data ^ state->sound_state[2];
	state->sound_state[2] = data;

	/* SHOT: channel 7 */
	if ((diff & 0x01) && !(data & 0x01)) sample_start(samples, 7, 7, FALSE);

	/* ALARM2: channel 8 */
	if ((diff & 0x04) && !(data & 0x04)) sample_start(samples, 8, 8, FALSE);

	/* ALARM3: channel 9 */
	if ((diff & 0x08) && !(data & 0x08) && !sample_playing(samples, 9)) sample_start(samples, 9, 9, FALSE);
}



/*************************************
 *
 *  Congo sound hardware description
 *
 *************************************/

static const char *const congo_sample_names[] =
{
	"*congo",
	"gorilla.wav",	/* 0 */
	"bass.wav",		/* 1 */
	"congal.wav",	/* 2 */
	"congah.wav",	/* 3 */
	"rim.wav",		/* 4 */
	0
};


static const samples_interface congo_samples_interface =
{
	5,	/* 5 channels */
	congo_sample_names
};


MACHINE_DRIVER_START( congo_samples )
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(congo_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/*************************************
 *
 *  Congo PPI write handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( congo_sound_b_w )
{
	zaxxon_state *state = (zaxxon_state *)device->machine->driver_data;
	running_device *samples = device->machine->device("samples");
	UINT8 diff = data ^ state->sound_state[1];
	state->sound_state[1] = data;

	/* bit 7 = mute */

	/* GORILLA: channel 0 */
	if ((diff & 0x02) && !(data & 0x02) && !sample_playing(samples, 0)) sample_start(samples, 0, 0, FALSE);
}


WRITE8_DEVICE_HANDLER( congo_sound_c_w )
{
	zaxxon_state *state = (zaxxon_state *)device->machine->driver_data;
	running_device *samples = device->machine->device("samples");
	UINT8 diff = data ^ state->sound_state[2];
	state->sound_state[2] = data;

	/* BASS DRUM: channel 1 */
	if ((diff & 0x01) && !(data & 0x01)) sample_start(samples, 1, 1, FALSE);
	if ((diff & 0x01) &&  (data & 0x01)) sample_stop(samples, 1);

	/* CONGA (LOW): channel 2 */
	if ((diff & 0x02) && !(data & 0x02)) sample_start(samples, 2, 2, FALSE);
	if ((diff & 0x02) &&  (data & 0x02)) sample_stop(samples, 2);

	/* CONGA (HIGH): channel 3 */
	if ((diff & 0x04) && !(data & 0x04)) sample_start(samples, 3, 3, FALSE);
	if ((diff & 0x04) &&  (data & 0x04)) sample_stop(samples, 3);

	/* RIM: channel 4 */
	if ((diff & 0x08) && !(data & 0x08)) sample_start(samples, 4, 4, FALSE);
	if ((diff & 0x08) &&  (data & 0x08)) sample_stop(samples, 4);
}
