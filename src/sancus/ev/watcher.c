#include "ev/private.h"

#include <sancus/ev/fd.h>

int sancus_watcher_attach2(struct sancus_watcher *w,
			   struct sancus_ev_loop **loop_out)
{
	if (unlikely(w == NULL)) {

fail_invalid:
		return -EINVAL;
	} else if (sancus_watcher_is_active(w)) {
		return -EBUSY;
	} else {
		struct sancus_ev_loop *loop, *loop0;
		loop = loop_out ? *loop_out : NULL;
		loop0 = sancus_watcher_get_loop(w);

		if (loop0 != NULL) {
			/* already attached */

			if (loop != NULL && loop != loop0)
				goto fail_invalid;
			else if (loop_out != NULL)
				*loop_out = loop0;

		} else if (!loop_lock(loop)) {
			goto fail_invalid;
		} else {
			/* attach */

			w->loop = loop;
			sancus_list_append(&loop->watchers, &w->watcher);
			loop_unlock(loop);
		}

		return 0;
	}
}

int sancus_watcher_detach(struct sancus_watcher *w)
{
	if (w != NULL) {

		switch (w->type) {
		case SANCUS_W_FD:
			return sancus_watcher_fd_detach(sancus_watcher__to_fd(w));
		case SANCUS_W_TIMER:
		case SANCUS_W_SIGNAL:
			return -ENOSYS;

		case SANCUS_W_UNDEFINED:
		default:
			break;
		}

	}

	return -EINVAL;
}

int sancus_watcher_start(struct sancus_watcher *w,
			 struct sancus_ev_loop *loop)
{

	if (w != NULL) {

		switch (w->type) {
		case SANCUS_W_FD:
			return sancus_watcher_fd_start(sancus_watcher__to_fd(w), loop);
		case SANCUS_W_TIMER:
		case SANCUS_W_SIGNAL:
			return -ENOSYS;

		case SANCUS_W_UNDEFINED:
		default:
			;
		}
	}

	return -EINVAL;
}

int sancus_watcher_stop(struct sancus_watcher *w)
{
	if (w != NULL) {

		switch (w->type) {
		case SANCUS_W_FD:
			return sancus_watcher_fd_stop(sancus_watcher__to_fd(w));
		case SANCUS_W_TIMER:
		case SANCUS_W_SIGNAL:
			return -ENOSYS;

		case SANCUS_W_UNDEFINED:
		default:
			;
		}

	}

	return -EINVAL;
}
