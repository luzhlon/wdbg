#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _WIN64
#define WDBG_ARCH "x64"
#else
#define WDBG_ARCH "x86"
#endif // _WIN64

#define WDBG_MAKE_VERSION(A, B, C) (A "." B "." C)
#define WDBG_VERSION WDBG_MAKE_VERSION("0", "2", "0")

#endif /* __CONFIG_H__ */
