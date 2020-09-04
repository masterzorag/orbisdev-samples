#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>

/// timing, SDL_GetTicks() equivalent
unsigned int get_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
