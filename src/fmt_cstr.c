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

#include <assert.h>

#include <sancus/common.h>
#include <sancus/fmt.h>

#define hexa "0123456789abcdef"

static const char ascii[128] = {
	'0',  0,  0,  0,  0,  0,  0,'a','b','t','n','v','f','r',  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
	'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
	'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
	'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',  0
};

size_t sancus_fmt_ascii_cstr(char *buf, size_t buf_len, const char *data, size_t size)
{
	size_t l, len = 0;

	assert(buf && buf_len > 0);
	assert(data && size > 0);

	while (size-- > 0) {
		unsigned char c = *data++;
		char e = (c & 0x80) ? 0 : ascii[c];

		if (!e)
			l = 4;
		else if (c < 32 || e == '"' || e == '\\')
			l = 2;
		else
			l = 1;

		if (buf_len <= l)
			break;

		buf_len -= l;
		len += l;

		switch(l) {
		case 2: *buf++ = '\\';
		case 1: *buf++ = e;
			break;
		case 4:
			/* hexa encoded */
			*buf++ = '\\';
			*buf++ = 'x';
			*buf++ = hexa[(c & 0xf0) >> 4];
			*buf++ = hexa[(c & 0x0f)];
		}
	}

	*buf = '\0';

	return len;
}
