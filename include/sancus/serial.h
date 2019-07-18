/*
 * Copyright (c) 2012, Alejandro Mery <amery@geeks.cl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _SANCUS_SERIAL_H
#define _SANCUS_SERIAL_H

#include <termios.h>

struct sancus_serial {
	int fd;
	struct termios oldtio;
};

/**
 * open @pathname as a serial device into @self, optionally
 * returning it's current termios as @copy
 */
int sancus_serial_open(struct sancus_serial *self, const char *pathname,
		       int cloexec, struct termios *copy);

/**
 * close serial port @self and restore the original termios data
 */
int sancus_serial_close(struct sancus_serial *self);

/**
 * apply a given termios to an open serial port
 */
int sancus_serial_apply(struct sancus_serial *, struct termios *);

static inline void sancus_serial_setup_raw(struct termios *tio)
{
	cfmakeraw(tio);
}

static inline void sancus_serial_setup_8N1(struct termios *tio,
					   speed_t baudrate)
{
	tio->c_cflag |= (CLOCAL|CREAD);

	tio->c_cflag &= ~(CSIZE|PARENB|CSTOPB);
	tio->c_cflag |= CS8;

	if (baudrate) {
		cfsetispeed(tio, baudrate);
		cfsetospeed(tio, baudrate);
	}
}

#endif
