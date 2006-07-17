/*
 * This file contains definitions for the x86 memory management unit (MMU),
 * including paging- and segmentation-related data structures and constants,
 * the %cr0, %cr4, and %eflags registers, and traps.
 */

/*
 *	Register flags and fundamental constants.
 */

#define PGSIZE		4096		// bytes mapped by a page

// Eflags register
#define FL_CF		0x00000001	// Carry Flag
#define FL_PF		0x00000004	// Parity Flag
#define FL_AF		0x00000010	// Auxiliary carry Flag
#define FL_ZF		0x00000040	// Zero Flag
#define FL_SF		0x00000080	// Sign Flag
#define FL_TF		0x00000100	// Trap Flag
#define FL_IF		0x00000200	// Interrupt Flag
#define FL_DF		0x00000400	// Direction Flag
#define FL_OF		0x00000800	// Overflow Flag
#define FL_IOPL_MASK	0x00003000	// I/O Privilege Level bitmask
#define FL_IOPL_0	0x00000000	//   IOPL == 0
#define FL_IOPL_1	0x00001000	//   IOPL == 1
#define FL_IOPL_2	0x00002000	//   IOPL == 2
#define FL_IOPL_3	0x00003000	//   IOPL == 3
#define FL_NT		0x00004000	// Nested Task
#define FL_RF		0x00010000	// Resume Flag
#define FL_VM		0x00020000	// Virtual 8086 mode
#define FL_AC		0x00040000	// Alignment Check
#define FL_VIF		0x00080000	// Virtual Interrupt Flag
#define FL_VIP		0x00100000	// Virtual Interrupt Pending
#define FL_ID		0x00200000	// ID flag

// Page fault error codes
#define FEC_PR		0x1	// Page fault caused by protection violation
#define FEC_WR		0x2	// Page fault caused by a write
#define FEC_U		0x4	// Page fault occured while in user mode

/*
 *
 *	Segmentation data structures and constants.
 *
 */

#ifdef __ASSEMBLER__

/*
 * Macros to build GDT entries in assembly.
 */
#define SEG_NULL						\
	.word 0, 0;						\
	.byte 0, 0, 0, 0
#define SEG(type,base,lim)					\
	.word (((lim) >> 12) & 0xffff), ((base) & 0xffff);	\
	.byte (((base) >> 16) & 0xff), (0x90 | (type)),		\
		(0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#else	// not __ASSEMBLER__

// Segment Descriptors
struct segdesc {
	uint lim_15_0 : 16;  // Low bits of segment limit
	uint base_15_0 : 16; // Low bits of segment base address
	uint base_23_16 : 8; // Middle bits of segment base address
	uint type : 4;       // Segment type (see STS_ constants)
	uint s : 1;          // 0 = system, 1 = application
	uint dpl : 2;        // Descriptor Privilege Level
	uint p : 1;          // Present
	uint lim_19_16 : 4;  // High bits of segment limit
	uint avl : 1;        // Unused (available for software use)
	uint rsv1 : 1;       // Reserved
	uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
	uint g : 1;          // Granularity: limit scaled by 4K when set
	uint base_31_24 : 8; // High bits of segment base address
};
// Null segment
#define SEG_NULL	(struct segdesc){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
// Segment that is loadable but faults when used
#define SEG_FAULT	(struct segdesc){ 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 }
// Normal segment
#define SEG(type, base, lim, dpl) (struct segdesc)			\
{ ((lim) >> 12) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,	\
    type, 1, dpl, 1, (uint) (lim) >> 28, 0, 0, 1, 1,		\
    (uint) (base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct segdesc)			\
{ (lim) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,		\
    type, 1, dpl, 1, (uint) (lim) >> 16, 0, 0, 1, 0,		\
    (uint) (base) >> 24 }

#endif /* !__ASSEMBLER__ */

// Application segment type bits
#define STA_X		0x8	    // Executable segment
#define STA_E		0x4	    // Expand down (non-executable segments)
#define STA_C		0x4	    // Conforming code segment (executable only)
#define STA_W		0x2	    // Writeable (non-executable segments)
#define STA_R		0x2	    // Readable (executable segments)
#define STA_A		0x1	    // Accessed

// System segment type bits
#define STS_T16A	0x1	    // Available 16-bit TSS
#define STS_LDT		0x2	    // Local Descriptor Table
#define STS_T16B	0x3	    // Busy 16-bit TSS
#define STS_CG16	0x4	    // 16-bit Call Gate
#define STS_TG		0x5	    // Task Gate / Coum Transmitions
#define STS_IG16	0x6	    // 16-bit Interrupt Gate
#define STS_TG16	0x7	    // 16-bit Trap Gate
#define STS_T32A	0x9	    // Available 32-bit TSS
#define STS_T32B	0xB	    // Busy 32-bit TSS
#define STS_CG32	0xC	    // 32-bit Call Gate
#define STS_IG32	0xE	    // 32-bit Interrupt Gate
#define STS_TG32	0xF	    // 32-bit Trap Gate


/*
 *
 *	Traps.
 *
 */

#ifndef __ASSEMBLER__

// Task state segment format (as described by the Pentium architecture book)
struct taskstate {
	uint32_t link;	// Old ts selector
	uintptr_t esp0;	// Stack pointers and segment selectors
	uint16_t ss0;	//   after an increase in privilege level
	uint16_t padding1;
	uintptr_t esp1;
	uint16_t ss1;
	uint16_t padding2;
	uintptr_t esp2;
	uint16_t ss2;
	uint16_t padding3;
	physaddr_t cr3;	// Page directory base
	uintptr_t eip;	// Saved state from last task switch
	uint32_t eflags;
	uint32_t eax;	// More saved state (registers)
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uintptr_t esp;
	uintptr_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;		// Even more saved state (segment selectors)
	uint16_t padding4;
	uint16_t cs;
	uint16_t padding5;
	uint16_t ss;
	uint16_t padding6;
	uint16_t ds;
	uint16_t padding7;
	uint16_t fs;
	uint16_t padding8;
	uint16_t gs;
	uint16_t padding9;
	uint16_t ldt;
	uint16_t padding10;
	uint16_t t;		// Trap on task switch
	uint16_t iomb;	// I/O map base address
};

// Gate descriptors for interrupts and traps
struct gatedesc {
	uint off_15_0 : 16;   // low 16 bits of offset in segment
	uint ss : 16;         // segment selector
	uint args : 5;        // # args, 0 for interrupt/trap gates
	uint rsv1 : 3;        // reserved(should be zero I guess)
	uint type : 4;        // type(STS_{TG,IG32,TG32})
	uint s : 1;           // must be 0 (system)
	uint dpl : 2;         // descriptor(meaning new) privilege level
	uint p : 1;           // Present
	uint off_31_16 : 16;  // high bits of offset in segment
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//	  the privilege level required for software to invoke
//	  this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)			\
{								\
	(gate).off_15_0 = (uint32_t) (off) & 0xffff;		\
	(gate).ss = (sel);					\
	(gate).args = 0;					\
	(gate).rsv1 = 0;					\
	(gate).type = (istrap) ? STS_TG32 : STS_IG32;	\
	(gate).s = 0;					\
	(gate).dpl = (d);					\
	(gate).p = 1;					\
	(gate).off_31_16 = (uint32_t) (off) >> 16;		\
}

// Set up a call gate descriptor.
#define SETCALLGATE(gate, ss, off, d)           	        \
{								\
	(gate).off_15_0 = (uint32_t) (off) & 0xffff;		\
	(gate).ss = (ss);					\
	(gate).args = 0;					\
	(gate).rsv1 = 0;					\
	(gate).type = STS_CG32;				\
	(gate).s = 0;					\
	(gate).dpl = (d);					\
	(gate).p = 1;					\
	(gate).off_31_16 = (uint32_t) (off) >> 16;		\
}

#endif /* !__ASSEMBLER__ */

