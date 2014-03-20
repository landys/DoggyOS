
#ifndef	_DOGGY_KTOOLLIB_H_
#define	_DOGGY_KTOOLLIB_H_

#define ROUND_DOWN(num, aligned_size) ((unsigned)(num) & ~((aligned_size) - 1))
#define ROUND_UP(num, aligned_size) (((unsigned)(num) + ((aligned_size) - 1)) & \
											~((aligned_size) - 1))
#define IS_ALIGNED(num, aligned_size) (!((unsigned)(num) & ((aligned_size) - 1)))

#define OFFSET(field, structure) ((unsigned)&(((structure*)0)->field))
#endif
