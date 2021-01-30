#ifndef __SANCUS_BIT_H__
#define __SANCUS_BIT_H__

#ifndef BIT
#define BIT(N) (1UL << (N))
#endif

/** set element on mask using clear/mask/set */
static inline unsigned long sancus_bit_cms(unsigned long old,
					   unsigned long value,
					   unsigned long mask,
					   unsigned shift)
{
	old &= ~(mask << shift);
	value = (value & mask) << shift;
	return value | old;
}

/** extract element from mask */
static inline unsigned long sancus_bit_unmask(unsigned long value,
					      unsigned mask,
					      unsigned shift)
{
	return (value >> shift) & mask;
}

static inline int sancus_bit_has_bit(unsigned long mask, unsigned bit)
{
	return !!(mask & BIT(bit));
}

static inline int sancus_bit_has_flag(unsigned long flags, unsigned flag)
{
	return (flags & flag) == flag;
}

#endif /* !__SANCUS_BIT_H__ */
