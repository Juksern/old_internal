#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <Psapi.h>

//ours
#include "vmt.hh"
#include "hooks.hh"
#include "render.hh"
#include "gui.hh"

#define INRANGE(x, a, b) (x >= a && x <= b)
#define GETBITS(x) (INRANGE(x, '0', '9') ? (x - '0') : ((x & (~0x20)) - 'A' + 0xa))
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

namespace utils {
    inline bool isMatch(const PBYTE addr, const PBYTE pat, const PBYTE msk) {
		size_t n = 0;
		while (addr[n] == pat[n] || msk[n] == (BYTE)'?') {
			if (!msk[++n]) {
				return true;
			}
		}
		return false;
	}

	inline PBYTE find_pattern(const PBYTE rangeStart, DWORD len,
		const char* pattern) {
		size_t l = strlen(pattern);
		PBYTE patt_base = static_cast<PBYTE>(malloc(l >> 1));
		PBYTE msk_base = static_cast<PBYTE>(malloc(l >> 1));
		PBYTE pat = patt_base;
		PBYTE msk = msk_base;
		l = 0;
		while (*pattern) {
			if (*pattern == ' ')
				pattern++;
			if (!*pattern)
				break;
			if (*(PBYTE)pattern == (BYTE)'\?') {
				*pat++ = 0;
				*msk++ = '?';
				pattern += ((*(PWORD)pattern == (WORD)'\?\?') ? 2 : 1);
			}
			else {
				*pat++ = GETBYTE(pattern);
				*msk++ = 'x';
				pattern += 2;
			}
			l++;
		}
		*msk = 0;
		pat = patt_base;
		msk = msk_base;
		for (DWORD n = 0; n < (len - l); ++n) {
			if (isMatch(rangeStart + n, patt_base, msk_base)) {
				return rangeStart + n;
			}
		}
		return NULL;
	}
}