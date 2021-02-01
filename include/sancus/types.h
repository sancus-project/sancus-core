#ifndef __SANCUS_TYPES_H__
#define __SANCUS_TYPES_H__

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef BIT
#define BIT(N) (1UL << N)
#endif

#define MASK8  0xff
#define MASK16 0xffff
#define MASK32 0xffffffff

/*
 * safe numeric conversions
 */
#define DECL_SANCUS_GET_CONVERTED(N,T0,T1,V,TEST,FN,...) \
static inline T1 sancus_ ##N(T0 V, ##__VA_ARGS__) \
{ \
	assert(TEST); \
	return FN; \
} \
static inline bool sancus_get_ ##N(T0 V, ##__VA_ARGS__, T1  *out) \
{ \
	if (TEST) { \
		if (out != NULL) \
			*out = FN; \
		return true; \
	} \
	return false; \
}

#define DECL_SANCUS_NUMERIC_CMP1_CAST(N,T0,T1,CA) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,v CA a,(T1)v,T0 a)
#define DECL_SANCUS_NUMERIC_CMP1_MASK(N,T0,T1,CA,M) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,v CA a,(T1)(v & M),T0 a)
#define DECL_SANCUS_NUMERIC_CMP1_F(N,T0,T1,CA,F) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,v CA a,F(v),T0 a)

#define DECL_SANCUS_NUMERIC_CMP1_CAST_K(N,T0,T1,CA,A) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,v CA A,(T1)v)

#define DECL_SANCUS_NUMERIC_CMP2_CAST(N,T0,T1,CA,CB) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,(v CA a) && (v CB b),(T1)v,T0 a, T0 b)
#define DECL_SANCUS_NUMERIC_CMP2_MASK(N,T0,T1,CA,CB,M) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,(v CA a) && (v CB b),(T1)(v & M),T0 a, T0 b)
#define DECL_SANCUS_NUMERIC_CMP2_F(N,T0,T1,CA,CB,F) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,(v CA a) && (v CB b),F(v),T0 a, T0 b)

#define DECL_SANCUS_NUMERIC_CMP2_CAST_K(N,T0,T1,CA,A,CB,B) \
	DECL_SANCUS_GET_CONVERTED(N,T0,T1,v,(v CA A) && (v CB B),(T1)v)

#define DECL_SANCUS_NUMERIC_GELE_CAST(N,T0,T1) \
	DECL_SANCUS_NUMERIC_CMP2_CAST(N,T0,T1,>=,<=)

#define DECL_SANCUS_NUMERIC_GELE_CAST_K(N,T0,T1,A,B) \
	DECL_SANCUS_NUMERIC_CMP2_CAST_K(N,T0,T1,>=,A,<=,B)

/* string to numeric */
#define DECL_SANCUS_STRTO(N,F,T,...) \
static inline bool sancus_get_##N(const char *s, T *out) \
{ \
	T v = 0; \
	bool ret = false; \
\
	if (s != NULL && *s != '\0') { \
		char *end = NULL; \
		\
		v = F(s, &end, ##__VA_ARGS__); \
		if (end != NULL && *end == '\0') \
			ret = true; \
	} \
\
	if (out != NULL) \
		*out = v; \
	return ret; \
}

DECL_SANCUS_STRTO(ls,   strtol,   long, 10)
DECL_SANCUS_STRTO(lls,  strtoll,  long long, 10)
DECL_SANCUS_STRTO(uls,  strtoul,  unsigned long, 10)
DECL_SANCUS_STRTO(ulls, strtoull, unsigned long long, 10)
DECL_SANCUS_STRTO(fs,   strtod,   double)
DECL_SANCUS_STRTO(ffs,  strtold,  long double)

#define DECL_SANCUS_GELE_STRTO(N,F0,F1,T0,T1) \
static inline bool sancus_get_##N(const char *s, T0 a, T0 b, T1 *out) \
{ \
	T0 v; \
	bool ret = false; \
	if (sancus_get_##F0(s, &v)) \
		ret = sancus_get_##F1(v,a,b,out); \
	return ret; \
}

/*
 * value within range as unsigned
 */
DECL_SANCUS_NUMERIC_GELE_CAST(urll, long long, unsigned)
DECL_SANCUS_NUMERIC_GELE_CAST(urzu, size_t,    unsigned)
DECL_SANCUS_NUMERIC_GELE_CAST(urzd, ssize_t,   unsigned)

DECL_SANCUS_GELE_STRTO(urlls, lls, urll, long long, unsigned)

/*
 * value within range as int
 */
DECL_SANCUS_NUMERIC_GELE_CAST(drl,  long,      int)
DECL_SANCUS_NUMERIC_GELE_CAST(drll, long long, int)
DECL_SANCUS_NUMERIC_GELE_CAST(drzu, size_t,    int)
DECL_SANCUS_NUMERIC_GELE_CAST(drzd, ssize_t,   int)

DECL_SANCUS_GELE_STRTO(drls,  ls, drl, long, int)
DECL_SANCUS_GELE_STRTO(drlls, lls, drll, long long, int)

/*
 * value as uintN_t
 */
#define DECL_SANCUS_uint(N,T0,B)  DECL_SANCUS_NUMERIC_GELE_CAST_K(N, T0, uint##B##_t, 0, MASK##B)
#define DECL_SANCUS_uintu(N,T0,B) DECL_SANCUS_NUMERIC_CMP1_CAST_K(N, T0, uint##B##_t, <=, MASK##B)

DECL_SANCUS_uint(u32ll, long long, 32)
DECL_SANCUS_uint(u16ll, long long, 16)
DECL_SANCUS_uint(u8ll,  long long,  8)

DECL_SANCUS_uint(u32l, long, 32)
DECL_SANCUS_uint(u16l, long, 16)
DECL_SANCUS_uint(u8l,  long,  8)

DECL_SANCUS_uint(u32zd, ssize_t, 32)
DECL_SANCUS_uint(u16zd, ssize_t, 16)
DECL_SANCUS_uint(u8zd,  ssize_t,  8)

DECL_SANCUS_uintu(u32zu, size_t, 32)
DECL_SANCUS_uintu(u16zu, size_t, 16)
DECL_SANCUS_uintu(u8zu,  size_t,  8)

#endif /* !__SANCUS_TYPES_H__ */
