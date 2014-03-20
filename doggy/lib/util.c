/*======================================================================*
                              milli_delay
 *======================================================================*/
#include "type.h"
#include "const.h"
#include "util.h"

PUBLIC void milli_delay(int milli_sec)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}
