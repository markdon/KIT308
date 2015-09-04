// YOU SHOULD _NOT_ NEED TO MODIFY THIS FILE

// simple timer
// system/OS/core specific functions required for timing
// to use this file you _MUST_ define either TARGET_PPU, TARGET_SPU, or TARGET_WINDOWS

#ifndef __TIMER_H
#define __TIMER_H

#if defined(TARGET_PPU)
	#include <ppu_intrinsics.h>
#elif defined(TARGET_SPU)
	#include <spu_mfcio.h>
#elif defined(TARGET_WINDOWS)
	#define NOMINMAX			// undefine stupid windows macros that break STL
	#include <windows.h>
#else
	#error Must define one of TARGET_PPU, TARGET_SPU, or TARGET_WINDOWS
#endif

class Timer
{
	// timing variables
	#if defined(TARGET_PPU)
		unsigned long long startTicks, finishTicks, usedTicks;
	#elif defined(TARGET_SPU)
		static const unsigned int startTicks = 0xFFFFFFFF;
		unsigned int finishTicks, usedTicks;
	#elif defined(TARGET_WINDOWS)
		unsigned int startTicks, finishTicks, usedTicks;
	#endif

public:
	Timer()
	{
		// automatically start the timer on creation (can still restart at any time)
		start();
	}

	// store start time
	inline void start()
	{
		#if defined(TARGET_PPU)
			startTicks = __mftb();								
		#elif defined(TARGET_SPU)
			spu_write_decrementer(startTicks);
		#elif defined(TARGET_WINDOWS)
			startTicks = GetTickCount();
		#endif
	}

	// calculate total time taken
	inline void end()
	{
		#if defined(TARGET_PPU)
			finishTicks = __mftb();								
			usedTicks = finishTicks - startTicks;
		#elif defined(TARGET_SPU)
			finishTicks = spu_read_decrementer();
			usedTicks = startTicks - finishTicks;
		#elif defined(TARGET_WINDOWS)
			finishTicks = GetTickCount();						
			usedTicks = finishTicks - startTicks;
		#endif
	}

	// get raw tick count (different per system/OS/core/whatever)
	inline unsigned long long getTicks()
	{
		return (unsigned long long) usedTicks;
	}

	// get time in milliseconds
	inline unsigned int getMilliseconds()
	{
		#if defined(TARGET_PPU)
			return (usedTicks * 1000) / 79800000;
		#elif defined(TARGET_SPU)
			return usedTicks / 80000;
		#elif defined(TARGET_WINDOWS)
			return usedTicks;
		#endif
	}
};

#endif //__TIMER_H
