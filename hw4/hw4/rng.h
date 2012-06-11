#ifndef _RNG_H_
#define _RNG_H_

#include <linux/ioctl.h>

#define RNG_IOC_MAGIC 'D'
#define RNG_LEVEL _IOW(RNG_IOC_MAGIC, 0, char)
#define RNG_GUESS _IOW(RNG_IOC_MAGIC, 1, char)
#define RNG_HINT _IOR(RNG_IOC_MAGIC, 2, char)

#endif /* _RNG_H_ */
