/*      
 * The following Code is from uLaunchELF
 */

#include <tamtypes.h>
#include <kernel.h>

/* fn prototypes */
void TimerInit(void);
u64 Timer(void);
void TimerEnd(void);

/* Timer Define */
#define T0_COUNT ((volatile unsigned long*)0x10000000)
#define T0_MODE  ((volatile unsigned long*)0x10000010)

static int TimerInterruptID = -1;
static u64 TimerInterruptCount = 0;
