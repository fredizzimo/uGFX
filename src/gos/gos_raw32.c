/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

// We need to include stdio.h below for Win32 emulation. Turn off GFILE_NEED_STDIO just for this file to prevent conflicts
#define GFILE_NEED_STDIO_MUST_BE_OFF

#include "gfx.h"

#if GFX_USE_OS_RAW32

#include <string.h>				// Prototype for memcpy()

#if GOS_RAW_HEAP_SIZE != 0
	static void _gosHeapInit(void);
#else
	#define _gosHeapInit()
#endif
static void _gosThreadsInit(void);

/*********************************************************
 * Initialise
 *********************************************************/

void _gosInit(void)
{
	/* No initialization of the operating system itself is needed as there isn't one.
	 * On the other hand the C runtime should still already be initialized before
	 * getting here!
	 */
	#warning "GOS: Raw32 - Make sure you initialize your hardware and the C runtime before calling gfxInit() in your application!"

	// Set up the heap allocator
	_gosHeapInit();

	// Start the scheduler
	_gosThreadsInit();
}

void _gosDeinit(void)
{
	/* ToDo */
}

/*********************************************************
 * For WIn32 emulation - automatically add the tick functions
 * the user would normally have to provide for bare metal.
 *********************************************************/

#if defined(WIN32)
	// Win32 nasty stuff - we have conflicting definitions for Red, Green & Blue
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501			// Windows XP and up
	#endif
	#undef Red
	#undef Green
	#undef Blue
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
	#define Blue			HTML2COLOR(0x0000FF)
	#define Red				HTML2COLOR(0xFF0000)
	#define Green			HTML2COLOR(0x008000)

	#include <stdio.h>
	systemticks_t gfxSystemTicks(void)						{ return GetTickCount(); }
	systemticks_t gfxMillisecondsToTicks(delaytime_t ms)	{ return ms; }
#endif

/*********************************************************
 * Exit everything functions
 *********************************************************/

void gfxHalt(const char *msg) {
	#if defined(WIN32)
		fprintf(stderr, "%s\n", msg);
		ExitProcess(1);
	#else
		volatile uint32_t	dummy;
		(void)				msg;

		while(1)
			dummy++;
	#endif
}

void gfxExit(void) {
	#if defined(WIN32)
		ExitProcess(0);
	#else
		volatile uint32_t	dummy;

		while(1)
			dummy++;
	#endif
}

/*********************************************************
 * Head allocation functions
 *********************************************************/

#if GOS_RAW_HEAP_SIZE == 0
	#include <stdlib.h>				// Prototype for malloc(), realloc() and free()

	void *gfxAlloc(size_t sz) {
		return malloc(sz);
	}

	void *gfxRealloc(void *ptr, size_t oldsz, size_t newsz) {
		(void) oldsz;
		return realloc(ptr, newsz);
	}

	void gfxFree(void *ptr) {
		free(ptr);
	}

#else

	// Slot structure - user memory follows
	typedef struct memslot {
		struct memslot *next;		// The next memslot
		size_t			sz;			// Includes the size of this memslot.
		} memslot;

	// Free Slot - immediately follows the memslot structure
	typedef struct freeslot {
		memslot *nextfree;			// The next free slot
	} freeslot;

	#define GetSlotSize(sz)		((((sz) + (sizeof(freeslot) - 1)) & ~(sizeof(freeslot) - 1)) + sizeof(memslot))
	#define NextFree(pslot)		((freeslot *)Slot2Ptr(pslot))->nextfree
	#define Ptr2Slot(p)			((memslot *)(p) - 1)
	#define Slot2Ptr(pslot)		((pslot)+1)

	static memslot *			firstSlot;
	static memslot *			lastSlot;
	static memslot *			freeSlots;
	static char					heap[GOS_RAW_HEAP_SIZE];

	static void _gosHeapInit(void) {
		lastSlot = 0;
		gfxAddHeapBlock(heap, GOS_RAW_HEAP_SIZE);
	}

	void gfxAddHeapBlock(void *ptr, size_t sz) {
		if (sz < sizeof(memslot)+sizeof(freeslot))
			return;

		if (lastSlot)
			lastSlot->next = (memslot *)ptr;
		else
			firstSlot = lastSlot = freeSlots = (memslot *)ptr;

		lastSlot->next = 0;
		lastSlot->sz = sz;
		NextFree(lastSlot) = 0;
	}

	void *gfxAlloc(size_t sz) {
		register memslot *prev, *p, *new;

		if (!sz) return 0;
		sz = GetSlotSize(sz);
		for (prev = 0, p = freeSlots; p != 0; prev = p, p = NextFree(p)) {
			// Loop till we have a block big enough
			if (p->sz < sz)
				continue;
			// Can we save some memory by splitting this block?
			if (p->sz >= sz + sizeof(memslot)+sizeof(freeslot)) {
				new = (memslot *)((char *)p + sz);
				new->next = p->next;
				p->next = new;
				new->sz = p->sz - sz;
				p->sz = sz;
				if (lastSlot == p)
					lastSlot = new;
				NextFree(new) = NextFree(p);
				NextFree(p) = new;
			}
			// Remove it from the free list
			if (prev)
				NextFree(prev) = NextFree(p);
			else
				freeSlots = NextFree(p);
			// Return the result found
			return Slot2Ptr(p);
		}
		// No slots large enough
		return 0;
	}

	void *gfxRealloc(void *ptr, size_t oldsz, size_t sz) {
		register memslot *prev, *p, *new;
		(void) oldsz;

		if (!ptr)
			return gfxAlloc(sz);
		if (!sz) {
			gfxFree(ptr);
			return 0;
		}

		p = Ptr2Slot(ptr);
		sz = GetSlotSize(sz);

		// If the next slot is free (and contiguous) merge it into this one
		if ((char *)p + p->sz == (char *)p->next) {
			for (prev = 0, new = freeSlots; new != 0; prev = new, new = NextFree(new)) {
				if (new == p->next) {
					p->next = new->next;
					p->sz += new->sz;
					if (prev)
						NextFree(prev) = NextFree(new);
					else
						freeSlots = NextFree(new);
					if (lastSlot == new)
						lastSlot = p;
					break;
				}
			}
		}

		// If this block is large enough we are nearly done
		if (sz < p->sz) {
			// Can we save some memory by splitting this block?
			if (p->sz >= sz + sizeof(memslot)+sizeof(freeslot)) {
				new = (memslot *)((char *)p + sz);
				new->next = p->next;
				p->next = new;
				new->sz = p->sz - sz;
				p->sz = sz;
				if (lastSlot == p)
					lastSlot = new;
				NextFree(new) = freeSlots;
				freeSlots = new;
			}
			return Slot2Ptr(p);
		}

		// We need to do this the hard way
		if ((new = gfxAlloc(sz)))
			return 0;
		memcpy(new, ptr, p->sz - sizeof(memslot));
		gfxFree(ptr);
		return new;
	}

	void gfxFree(void *ptr) {
		register memslot *prev, *p, *new;

		if (!ptr)
			return;

		p = Ptr2Slot(ptr);

		// If the next slot is free (and contiguous) merge it into this one
		if ((char *)p + p->sz == (char *)p->next) {
			for (prev = 0, new = freeSlots; new != 0; prev = new, new = NextFree(new)) {
				if (new == p->next) {
					p->next = new->next;
					p->sz += new->sz;
					if (prev)
						NextFree(prev) = NextFree(new);
					else
						freeSlots = NextFree(new);
					if (lastSlot == new)
						lastSlot = p;
					break;
				}
			}
		}

		// Add it into the free chain
		NextFree(p) = freeSlots;
		freeSlots = p;
	}
#endif

/*********************************************************
 * Semaphores and critical region functions
 *********************************************************/

#if !defined(INTERRUPTS_OFF) || !defined(INTERRUPTS_ON)
	#define INTERRUPTS_OFF()
	#define INTERRUPTS_ON()
#endif

void gfxSystemLock(void) {
	INTERRUPTS_OFF();
}

void gfxSystemUnlock(void) {
	INTERRUPTS_ON();
}

void gfxMutexInit(gfxMutex *pmutex) {
	pmutex[0] = 0;
}

void gfxMutexEnter(gfxMutex *pmutex) {
	INTERRUPTS_OFF();
	while (pmutex[0]) {
		INTERRUPTS_ON();
		gfxYield();
		INTERRUPTS_OFF();
	}
	pmutex[0] = 1;
	INTERRUPTS_ON();
}

void gfxMutexExit(gfxMutex *pmutex) {
	pmutex[0] = 0;
}

void gfxSemInit(gfxSem *psem, semcount_t val, semcount_t limit) {
	psem->cnt = val;
	psem->limit = limit;
}

bool_t gfxSemWait(gfxSem *psem, delaytime_t ms) {
	systemticks_t	starttm, delay;

	// Convert our delay to ticks
	switch (ms) {
	case TIME_IMMEDIATE:
		delay = TIME_IMMEDIATE;
		break;
	case TIME_INFINITE:
		delay = TIME_INFINITE;
		break;
	default:
		delay = gfxMillisecondsToTicks(ms);
		if (!delay) delay = 1;
		starttm = gfxSystemTicks();
	}

	INTERRUPTS_OFF();
	while (psem->cnt <= 0) {
		INTERRUPTS_ON();
		// Check if we have exceeded the defined delay
		switch (delay) {
		case TIME_IMMEDIATE:
			return FALSE;
		case TIME_INFINITE:
			break;
		default:
			if (gfxSystemTicks() - starttm >= delay)
				return FALSE;
			break;
		}
		gfxYield();
		INTERRUPTS_OFF();
	}
	psem->cnt--;
	INTERRUPTS_ON();
	return TRUE;
}

bool_t gfxSemWaitI(gfxSem *psem) {
	if (psem->cnt <= 0)
		return FALSE;
	psem->cnt--;
	return TRUE;
}

void gfxSemSignal(gfxSem *psem) {
	INTERRUPTS_OFF();
	gfxSemSignalI(psem);
	INTERRUPTS_ON();
}

void gfxSemSignalI(gfxSem *psem) {
	if (psem->cnt < psem->limit)
		psem->cnt++;
}

/*********************************************************
 * Sleep functions
 *********************************************************/

void gfxSleepMilliseconds(delaytime_t ms) {
	systemticks_t	starttm, delay;

	// Safety first
	switch (ms) {
	case TIME_IMMEDIATE:
		return;
	case TIME_INFINITE:
		while(1)
			gfxYield();
		return;
	}

	// Convert our delay to ticks
	delay = gfxMillisecondsToTicks(ms);
	starttm = gfxSystemTicks();

	do {
		gfxYield();
	} while (gfxSystemTicks() - starttm < delay);
}

void gfxSleepMicroseconds(delaytime_t ms) {
	systemticks_t	starttm, delay;

	// Safety first
	switch (ms) {
	case TIME_IMMEDIATE:
		return;
	case TIME_INFINITE:
		while(1)
			gfxYield();
		return;
	}

	// Convert our delay to ticks
	delay = gfxMillisecondsToTicks(ms/1000);
	starttm = gfxSystemTicks();

	do {
		gfxYield();
	} while (gfxSystemTicks() - starttm < delay);
}

/*********************************************************
 * Threading functions
 *********************************************************/

#if GOS_RAW_SCHEDULER == SCHED_USE_SETJMP

	/**
	 * There are some compilers we know how they store the jmpbuf. For those
	 * we can use the constant macro definitions. For others we have to "auto-detect".
	 * Auto-detection is hairy and there is no guarantee it will work on all architectures.
	 * For those it doesn't - read the compiler manuals and the library source code to
	 * work out the correct macro values.
	 * You can use the debugger to work out the values for your compiler and put them here.
	 * Defining these macros as constant values makes the system behavior guaranteed but also
	 * makes your code compiler and cpu architecture dependent. It also saves a heap of code
	 * and a few bytes of RAM.
	 */
	#if GFX_COMPILER == GFX_COMPILER_MINGW32
		#define AUTO_DETECT_MASK	FALSE
		#define STACK_DIR_UP		FALSE
		#define MASK1				0x00000011
		#define MASK2				0x00000000
		#define STACK_BASE			12
	#else
		// Use auto-detection of the stack frame format
		// Assumes all the relevant stuff to be relocated is in the first 256 bytes of the jmpbuf.
		#define AUTO_DETECT_MASK	TRUE
		#define STACK_DIR_UP		stackdirup			// TRUE if the stack grow up instead of down
		#define MASK1				jmpmask1			// The 1st mask of jmp_buf elements that need relocation
		#define MASK2				jmpmask2			// The 2nd mask of jmp_buf elements that need relocation
		#define STACK_BASE			stackbase			// The base of the stack frame relative to the local variables
		static bool_t		stackdirup;
		static uint32_t		jmpmask1;
		static uint32_t		jmpmask2;
		static size_t		stackbase;
	#endif

	#include <setjmp.h> /* jmp_buf, setjmp(), longjmp() */

	/**
	 * Some compilers define a _setjmp() and a setjmp().
	 * The difference between them is that setjmp() saves the signal masks.
	 * That is of no use to us so prefer to use the _setjmp() methods.
	 * If they don't exist compile them to be the standard setjmp() function.
	 * Similarly for longjmp().
	 */
	#if (!defined(setjmp) && !defined(_setjmp)) || defined(__KEIL__) || defined(__C51__)
		#define _setjmp setjmp
	#endif
	#if (!defined(longjmp) && !defined(_longjmp)) || defined(__KEIL__) || defined(__C51__)
		#define _longjmp longjmp
	#endif

	typedef jmp_buf		threadcxt;

#elif GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M0 || GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M1
	typedef void *	threadcxt;
#elif GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M3 || GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M4
	typedef void *	threadcxt;
#else
	#error "GOS RAW32: Unsupported Scheduler. Try setting GOS_RAW_SCHEDULER = SCHED_USE_SETJMP"
#endif

typedef struct thread {
	struct thread *	next;					// Next thread
	int				flags;					// Flags
		#define FLG_THD_ALLOC	0x0001
		#define FLG_THD_MAIN	0x0002
		#define FLG_THD_DEAD	0x0004
		#define FLG_THD_WAIT	0x0008
	size_t			size;					// Size of the thread stack (including this structure)
	threadreturn_t	(*fn)(void *param);		// Thread function
	void *			param;					// Parameter for the thread function
	threadcxt		cxt;					// The current thread context.
} thread;

typedef struct threadQ {
	thread *head;
	thread *tail;
} threadQ;

static threadQ		readyQ;					// The list of ready threads
static threadQ		deadQ;					// Where we put threads waiting to be deallocated
static thread *		current;				// The current running thread
static thread		mainthread;				// The main thread context

static void Qinit(threadQ * q) {
	q->head = q->tail = 0;
}

static void Qadd(threadQ * q, thread *t) {
	t->next = 0;
	if (q->head) {
		q->tail->next = t;
		q->tail = t;
	} else
		q->head = q->tail = t;
}

static thread *Qpop(threadQ * q) {
	struct thread * t;

	if (!q->head)
		return 0;
	t = q->head;
	q->head = t->next;
	return t;
}

#if GOS_RAW_SCHEDULER == SCHED_USE_SETJMP && AUTO_DETECT_MASK

	// The structure for the saved stack frame information
	typedef struct saveloc {
		char *		localptr;
		jmp_buf		cxt;
	} saveloc;

	// A pointer to our auto-detection buffer.
	static saveloc	*pframeinfo;

	/* These functions are not static to prevent the compiler removing them as functions */

	void get_stack_state(void) {
		char *c;
		pframeinfo->localptr = (char *)&c;
		_setjmp(pframeinfo->cxt);
	}

	void get_stack_state_in_fn(void) {
		pframeinfo++;
		get_stack_state();
		pframeinfo--;
	}
#endif

static void _gosThreadsInit(void) {
	Qinit(&readyQ);
	current = &mainthread;
	current->next = 0;
	current->size = sizeof(thread);
	current->flags = FLG_THD_MAIN;
	current->fn = 0;
	current->param = 0;

	#if GOS_RAW_SCHEDULER == SCHED_USE_SETJMP && AUTO_DETECT_MASK
		{
			uint32_t	i;
			char **		pout;
			char **		pin;
			size_t		diff;
			char *		framebase;

			// Allocate a buffer to store our test data
			pframeinfo = gfxAlloc(sizeof(saveloc)*2);

			// Get details of the stack frame from within a function
			get_stack_state_in_fn();

			// Get details of the stack frame outside the function
			get_stack_state();

			/* Work out the frame entries to relocate by treating the jump buffer as an array of pointers */
			stackdirup =  pframeinfo[1].localptr > pframeinfo[0].localptr;
			pout = (char **)pframeinfo[0].cxt;
			pin =  (char **)pframeinfo[1].cxt;
			diff = pframeinfo[0].localptr - pframeinfo[1].localptr;
			framebase = pframeinfo[0].localptr;
			jmpmask1 = jmpmask2 = 0;
			for (i = 0; i < sizeof(jmp_buf)/sizeof(char *); i++, pout++, pin++) {
				if ((size_t)(*pout - *pin) == diff) {
					if (i < 32)
						jmpmask1 |= 1 << i;
					else
						jmpmask2 |= 1 << (i-32);

					if (stackdirup) {
						if (framebase > *pout)
							framebase = *pout;
					} else {
						if (framebase < *pout)
							framebase = *pout;
					}
				}
			}
			stackbase = stackdirup ? (pframeinfo[0].localptr - framebase) : (framebase - pframeinfo[0].localptr);

			// Clean up
			gfxFree(pframeinfo);
		}
	#endif
}

gfxThreadHandle gfxThreadMe(void) {
	return (gfxThreadHandle)current;
}

// Check if there are dead processes to deallocate
static void cleanUpDeadThreads(void) {
	thread *p;

	while ((p = Qpop(&deadQ)))
		gfxFree(p);
}

#if GOS_RAW_SCHEDULER == SCHED_USE_SETJMP
	// Move the stack frame and relocate the context data
	void _gfxAdjustCxt(thread *t) {
		char **	s;
		char *	nf;
		int		diff;
		uint32_t	i;

		// Copy the stack frame
		#if AUTO_DETECT_MASK
			if (STACK_DIR_UP) {					// Stack grows up
				nf = (char *)(t) + sizeof(thread) + stackbase;
				memcpy(t+1, (char *)&s - stackbase, stackbase+sizeof(char *));
			} else {							// Stack grows down
				nf = (char *)(t) + t->size - (stackbase + sizeof(char *));
				memcpy(nf, &s, stackbase+sizeof(char *));
			}
		#elif STACK_DIR_UP
			// Stack grows up
			nf = (char *)(t) + sizeof(thread) + stackbase;
			memcpy(t+1, (char *)&s - stackbase, stackbase+sizeof(char *));
		#else
			// Stack grows down
			nf = (char *)(t) + t->size - (stackbase + sizeof(char *));
			memcpy(nf, &s, stackbase+sizeof(char *));
		#endif

		// Relocate the context data
		s = (char **)(t->cxt);
		diff = nf - (char *)&s;

		// Relocate the elements we know need to be relocated
		for (i = 1; i && i < MASK1; i <<= 1, s++) {
			if ((MASK1 & i))
				*s += diff;
		}
		#ifdef MASK2
			for (i = 1; i && i < MASK2; i <<= 1, s++) {
				if ((MASK1 & i))
					*s += diff;
			}
		#endif
	}
	#define CXT_SET(t) {                                                         \
		if (!_setjmp(t->cxt)) {													\
			_gfxAdjustCxt(t);													\
			current = t;														\
			_longjmp(current->cxt, 1);											\
		}																		\
	}
	#define CXT_SAVE() 			if (_setjmp(current->cxt)) return
	#define CXT_RESTORE() 		_longjmp(current->cxt, 1)

#elif GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M0 || GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M1

	// Use the EABI calling standard (ARM's AAPCS) - Save r4 - r11

	#define CXT_SET(t) {														\
		register void *r13 asm ("r13");											\
		current = t;															\
		r13 = (char *)current + current->size;									\
	}

	/**
	 * Save the current thread context.
	 *
	 * Automatically returns the calling function when this thread gets restarted
	 * with the thread handle as the return value
	 */
	//
	#define CXT_SAVE() {															\
		register void *r13 asm ("r13");												\
		asm volatile (	"push    {r4, r5, r6, r7, lr}                   \n\t"		\
						"mov     r4, r8                                 \n\t"		\
						"mov     r5, r9                                 \n\t"		\
						"mov     r6, r10                                \n\t"		\
						"mov     r7, r11                                \n\t"		\
						"push    {r4, r5, r6, r7}" : : : "memory");					\
		current->ctx = r13;															\
	}
	#define CXT_RESTORE() {															\
		register void *		r13	asm ("r13");										\
		r13 = current->ctx;															\
		asm volatile (	"pop     {r4, r5, r6, r7}                       \n\t"		\
						"mov     r8, r4                                 \n\t"		\
						"mov     r9, r5                                 \n\t"		\
						"mov     r10, r6                                \n\t"		\
						"mov     r11, r7                                \n\t"		\
						"pop     {r4, r5, r6, r7, pc}" : : "r" (r13) : "memory");	\
	}

#elif GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M3 || GOS_RAW_SCHEDULER == SCHED_USE_CORTEX_M4

		// Use the EABI calling standard (ARM's AAPCS) - Save r4 - r11 and floating point if needed

		#define CXT_SET(t) {														\
			register void *r13 asm ("r13");											\
			current = t;															\
			r13 = (char *)current + current->size;									\
		}

		#if CORTEX_USE_FPU
			#define CXT_SAVE() {															\
				register void *r13 asm ("r13");												\
				asm volatile ("push {r4, r5, r6, r7, r8, r9, r10, r11, lr}" : : : "memory");\
				asm volatile ("vpush {s16-s31}" : : : "memory");							\
				current->ctx = r13;															\
			}
			#define CXT_RESTORE() {															\
				register void *		r13	asm ("r13");										\
				r13 = current->ctx;															\
				asm volatile ("vpop {s16-s31}" : : : "memory");							\
				asm volatile ("pop {r4, r5, r6, r7, r8, r9, r10, r11, pc}" : : : "memory");	\
			}
		#else
			#define CXT_SAVE() {															\
				register void *r13 asm ("r13");												\
				asm volatile ("push {r4, r5, r6, r7, r8, r9, r10, r11, lr}" : : : "memory");\
				current->ctx = r13;															\
			}
			#define CXT_RESTORE() {															\
				register void *		r13	asm ("r13");										\
				r13 = current->ctx;															\
				asm volatile ("pop {r4, r5, r6, r7, r8, r9, r10, r11, pc}" : : : "memory");	\
			}
		#endif
#endif

void gfxYield(void) {

	// Clean up zombies
	cleanUpDeadThreads();

	// Is there another thread to run?
	if (!readyQ.head)
		return;

	// Save the current thread (automatically returns when this thread gets re-executed)
	CXT_SAVE();

	// Add us back to the Queue
	Qadd(&readyQ, current);

	// Run the next process
	current = Qpop(&readyQ);
	CXT_RESTORE();
}

// This routine is not currently public - but it could be.
void gfxThreadExit(threadreturn_t ret) {
	// Save the results in case someone is waiting
	current->param = (void *)ret;
	current->flags |= FLG_THD_DEAD;

	// Add us to the dead list if we need deallocation as we can't free ourselves.
	// If someone is waiting on the thread they will do the cleanup.
	if ((current->flags & (FLG_THD_ALLOC|FLG_THD_WAIT)) == FLG_THD_ALLOC)
		Qadd(&deadQ, current);

	// Set the next thread
	current = Qpop(&readyQ);

	// Was that the last thread? If so exit
	if (!current)
		gfxExit();

	// Switch to the new thread
	CXT_RESTORE();
}

void _gfxStartThread(thread *t) {

	// Save the current thread (automatically returns when this thread gets re-executed)
	CXT_SAVE();

	// Add the current thread to the queue because we are starting a new thread.
	Qadd(&readyQ, current);

	// Change to the new thread and the new stack
	CXT_SET(t);

	// Run the users function
	gfxThreadExit(current->fn(current->param));

	// We never get here!
}

gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param) {
	thread *	t;
	(void)		prio;

	// Ensure we have a minimum stack size
	if (stacksz < sizeof(thread)+64) {
		stacksz = sizeof(thread)+64;
		stackarea = 0;
	}

	if (stackarea) {
		t = (thread *)stackarea;
		t->flags = 0;
	} else {
		t = (thread *)gfxAlloc(stacksz);
		if (!t)
			return 0;
		t->flags = FLG_THD_ALLOC;
	}
	t->size = stacksz;
	t->fn = fn;
	t->param = param;

	_gfxStartThread(t);

	// Return the new thread handle
	return t;
}

threadreturn_t gfxThreadWait(gfxThreadHandle th) {
	thread *		t;

	t = th;
	if (t == current)
		return -1;

	// Mark that we are waiting
	t->flags |= FLG_THD_WAIT;

	// Wait for the thread to die
	while(!(t->flags & FLG_THD_DEAD))
		gfxYield();

	// Unmark
	t->flags &= ~FLG_THD_WAIT;

	// Clean up resources if needed
	if (t->flags & FLG_THD_ALLOC)
		gfxFree(t);

	// Return the status left by the dead process
	return (threadreturn_t)t->param;
}

#endif /* GFX_USE_OS_RAW32 */
