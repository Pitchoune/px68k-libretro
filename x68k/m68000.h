#ifndef _WINX68K_M68000_H
#define _WINX68K_M68000_H

#include "common.h"

extern	void	Exception(int nr, uint32_t oldpc);
extern	int	m68000_ICount;
extern	int	m68000_ICountBk;
extern	int	ICount;
extern	void	M68KRUN(void);
extern	void	M68KRESET(void);

typedef struct
{
	uint32_t d[8];
	uint32_t a[8];

	uint32_t isp;

	uint32_t sr_high;
	uint32_t ccr;
	uint32_t x_carry;

	uint32_t pc;
	uint32_t IRQ_level;

	uint32_t sr;

	void *irq_callback;

	uint32_t ppc;
	uint32_t (*reset_callback)(void);

	uint32_t sfc;
	uint32_t dfc;
	uint32_t usp;
	uint32_t vbr;

	uint32_t bank;

	uint32_t memmin;
	uint32_t memmax;

	uint32_t cputype;
} m68k_regs;

extern m68k_regs regs;

#endif /* _WINX68K_M68000_H */
