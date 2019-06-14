/*
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
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
#ifndef _SANCUS_STREAM_H
#define _SANCUS_STREAM_H

struct sancus_stream;

/**
 *
 */
enum sancus_stream_error {
	SANCUS_STREAM_READ_ERROR,
	SANCUS_STREAM_READ_EOF,
	SANCUS_STREAM_READ_FULL,
};

/**
 *
 */
struct sancus_stream_settings {
	bool (*on_error) (struct sancus_stream *,
			  struct sancus_ev_loop *,
			  enum sancus_stream_error);

	void (*on_close) (struct sancus_stream *);

	ssize_t (*on_read) (struct sancus_stream *,
			    char *, size_t);
};

/**
 *
 */
struct sancus_stream {
	struct sancus_ev_fd read_watcher;
	struct sancus_buffer read_buffer;

	struct sancus_stream_settings *settings;
};

/**
 *
 */
void sancus_stream_start(struct sancus_stream *self, struct sancus_ev_loop *loop);

/**
 *
 */
void sancus_stream_stop(struct sancus_stream *self, struct sancus_ev_loop *loop);

/**
 *
 */
void sancus_stream_close(struct sancus_stream *self);

/**
 *
 */
int sancus_stream_init(struct sancus_stream *self,
		       struct sancus_stream_settings *settings,
		       int fd,
		       char *read_buffer, size_t read_buf_size);

/**
 * sancus_stream_fd - returns fd watched by the given stream
 */
static inline int sancus_stream_fd(struct sancus_stream *self)
{
	return self->read_watcher.fd;
}

/**
 * sancus_stream_process - try to process more of the read buffer
 *
 * @self:	stream
 *
 * Returns 0 if there is nothing to process, < 0 on error and > 0 if
 * part of the buffer was processed.
 */
ssize_t sancus_stream_process(struct sancus_stream *self);

/**
 */
#define sancus_stream_reset_readbuffer(S) sancus_buffer_reset(&(S)->read_buffer)

#endif /* !_SANCUS_STREAM_H */
