/*	dsp32c.i - DSP32C board definitions for DSP32C assembler

	This file defines the DSP32C environment for my home-brew DSP32C
	board. It has:
		4 16-bit DACs on SIO output (8 output bytes).
		2 16-bit ADCs on SIO input (but 4 input words, 8 bytes).
		SIO frame rate is 32 kHz.
		Memory mode 5 hard-wired.
		4 MBytes of Dynamic RAM at 0x800000 - 0xBFFFFF.
		DRAM refresh via DSP read anywhere 0x001000 - 0x7FFFFF.
		  64 times per millisecond (addr must increment).
		  Data will be garbage (zero wait states).
		PIOP bits 0x03 enable the DAC outputs (0xFC = enable,
		  0xFF = disable).
		Interrupt 1 is before SIO frame begins (i.e. ADC/DAC 0
		  will be next). No SIO I/O will occur for at least 6
		  microsec after the interrupt occurs.


	24-bit Memory Mode 5: LOW_RAM=000000-0007FF
			      MED_RAM=000800-000FFF
			       HI_RAM=FFF800-FFFFFF

	External Dynamic RAM: refresh=001000-7FFFFF
				 DRAM=800000-BFFFFF

	r19 is dedicated to refreshing DRAM
	normally, REFRESH should replace "nop" in
	    latent instructions

	DRAM REFRESH:
	    REFRESH(rXe) must be invoked 64 times
		per millisecond	(2 per SIO frame)
		CLOBBERS rXe !!  1 instruction.
	    MASK_REFRESH is 2 instructions, and
		should be invoked every few
		thousand REFRESH(rXe) calls
		(or more often)
*/
/***************

THIS SOFTWARE IS RELEASED "AS IS", WITH NO WARRANTY EXPRESSED OR IMPLIED.
This software is copyright 1984, 1991, 1992, 1993, 1994 by Tom Roberts.
License is granted for unlimited distribution, as long as the following
conditions are met:
  A. This notice is preserved intact.
  B. No charge is made for the software (other than a reasonable
     distribution/duplication fee).

***************/


// memory organization
LOW_RAM 	equ	0x00000000
LOW_RAM_END 	equ	0x000007FF
MED_RAM		equ	0x00000800
MED_RAM_END 	equ	0x00000FFF
HI_RAM		equ	0x00FFF800
HI_RAM_END 	equ	0x00FFFFFF
DRAM		equ	0x00800000
DRAM_END 	equ	0x00BFFFFF

//
// DRAM Refresh
REFRESH_ADDR equ 0x1000	/* initial refresh address */
#define REFRESH(REGE)   /* Refresh */ REGE = *r19++;
#define MASK_REFRESH    /* Refresh-Mask */ 	\
	r19 = r19 & 0x0FFF; 			\
	r19 = r19 + REFRESH_ADDR;

//
//	DAUC bits
DAUC 	=	0x1F	// linear, truncate on float-to-int

//	PCW bits
MEM_WAIT=	0x3F	// 2-or-more wait states
PIOP	=	3	// 8 output bits
MGN	=	1	// MGN is output enable
INTER	=	0	// no interrupts enabled
PCW	=	(INTER<<10) | (MGN<<8) | (PIOP<<6) | MEM_WAIT

//	IOC bits
ASY	=	0	// external sync
BC	=	0	// ICK drives internal sync
SLEN	=	0	// not used
AIC	=	0	// ICK is external
AIL	=	0	// ILD is external
ILEN	=	2	// 16 bits
AOC	=	0	// OCK is external
AOL	=	0	// OLD is external
OLEN	=	2	// 16 bits
SAN	=	0	// Sanity bit
IN	=	1	// MSB first
OUT	=	1	// MSB first
DMA	=	0	// no DMA (yet - see SIO_DMA() below)
CKI	=	0	// internal clock = CKI/8  (not used)
O24	=	0	// not used
DSZ	=	1	// DMA size is 16 bits
IOC	=	(DSZ<<20) | (O24<<19) | (CKI<<18) | (OUT<<17) | (IN<<16)
IOC	=	IOC | (DMA<<13) | (SAN<<12) | (OLEN<<10) | (AOL<<9)
IOC	=	IOC | (ILEN<<6) | (AIL<<5) | (AIC<<4) | (SLEN<<2)
IOC	=	IOC | (BC<<1) | ASY
SANITY	=	1 << 12

#define DSP_INIT	/* DSP-INIT */			\
	r1 = PCW					\
	pcw = r1					\
	r1 = 0xFF	/* disable DACs */		\
	piop = r1l					\
	ioc = IOC	/* no SIO DMA */		\
	dauc = DAUC					\
							\
	/* zero HI_RAM and Refresh the DRAM */		\
	r2e = HI_RAM					\
	r4e = 0						\
	r19e = REFRESH_ADDR				\
Zero_loop:						\
	*r2++ = r4e					\
	r2 - 0	/* HI_END+1 == 0 */			\
	if(lt) goto Zero_loop				\
	REFRESH(r1e)					\
							\
	/* wait for external sync CLEAR */		\
W_sync:	MASK_REFRESH					\
	if(sys) goto W_sync				\
	REFRESH(r1e)					\
							\
	/* wait for external sync SET */		\
W_sync2: MASK_REFRESH					\
	if(syc) goto W_sync2				\
	REFRESH(r1e)					\
							\
	/* wait for external sync CLEAR */		\
W_sync3: MASK_REFRESH					\
	if(sys) goto W_sync3				\
	REFRESH(r1e)					\
	/* end DSP-Init */

/*
	SIO I/O: There are 4 inputs and 4 outputs per frame.
	Sync identifies the next input/output as channel 0, and
	when Sync is active, no SIO DMA will occur for at least
	6 microseconds. Each SIO DMA is 16 bits (int).
	IBUF and OBUF must be appropriately aligned (multiple of
	NBUFS*8)

	NOTE: SIO_DMA() must appear before any interrupts are enabled
	(e.g. FRAME_INT()).
	Normally DAC_ENABLE immediately follows this macro.
*/
#define SIO_DMA(IBUF,OBUF,NBUFS)  			\
InputBuf	=	(IBUF)				\
OutputBuf	=	(OBUF)				\
InputMask 	=	((NBUFS)*8-1)&(~7)		\
OutputMask 	=	((NBUFS)*8-1)&(~7)		\
	r2e = OutputBuf					\
	r1 = 0						\
	do 1-1,(NBUFS)*4-1				\
		*r2++ = r1				\
W_sync4: MASK_REFRESH	  				\
	if(syc) goto W_sync4				\
	REFRESH(r2e)					\
	r2 = ibuf					\
	r20e = InputBuf					\
	obuf = r1					\
	r21e = OutputBuf				\
IOC = IOC | (4<<13)   					\
	ioc = IOC					\
	nop

/*
	DMA_DISABLE must follow SIO_DMA() in the file.
	Normally, DAC_DISABLE immediately preceeds this macro.

	Interrupts MUST be DISABLED to issue this macro.
*/
#define DMA_DISABLE					\
	nop						\
IOC = IOC & ~(7<<13)					\
	ioc = IOC					\
	nop

/*
	SIO_DMA_MASK will wrap the SIO DMA pointers within their buffers.
	It should only be used while sync is active (or during the frame
	interrupt).
*/
#define SIO_DMA_MASK			 		\
	r20e = r20e & InputMask; 			\
	r20e = r20e | InputBuf; 			\
	r21e = r21e & OutputMask; 			\
	r21e = r21e | OutputBuf


// enable/disable DAC outputs
#define DAC_ENABLE /* DAC-Enable */			\
	r1 = 0xFC;					\
	piop = r1l;

#define DAC_DISABLE /* DAC-Disable */			\
	r1 = 0xFF;					\
	piop = r1l;

/*	FRAME_INT() will enable the Frame interrupt.

	NOTE: ONLY ireq1 can be enabled. This macro points ivtp
	directly to the interrupt service routine (ISR), and enables
	ireq1. As the hardware interrupt request is asserted (LOW)
	for 125 ns, there must be at least 4 instructions in the ISR.
*/
#define FRAME_INT(ISR)	/* FRAME_INT(ISR) */		\
	r22e = ISR					\
	r1 = pcw					\
	r2e = 0x8000					\
	r1 = r1 | r2					\
	if(sys) goto $					\
	nop						\
	pcw = r1

//	INT_DISABLE will disable all interrupts
//
#define INT_DISABLE					\
	r1 = pcw					\
	r2 = 0x03FF					\
	r1 = r1 | r2					\
	pcw = r1


//	END OF dsp32c.i
