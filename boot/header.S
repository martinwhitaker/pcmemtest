// SPDX-License-Identifier: GPL-2.0
//
// header.S supports booting directly from a UEFI BIOS or via an intermediate
// bootloader that supports the Linux boot protocol. When booted directly from
// the BIOS, it provides the MS-DOS & PE/COFF headers. When using an intermediate
// bootloader, it provides the first few bytes of the Linux boot header (at the
// end of the boot sector), with the remainder of the header being provided by
// setup.S.
//
// Copyright (C) 2020 Martin Whitaker.
//
// Derived from Linux 5.6 arch/x86/boot/header.S:
//
//	Copyright (C) 1991, 1992 Linus Torvalds
//
//	Based on bootsect.S and setup.S
//	modified by more people than can be counted
//
//	Rewritten as a common file by H. Peter Anvin (Apr 2007)

#define __ASSEMBLY__

#include "boot.h"

# The EFI loader loads the header at ImageBase, so we have to locate the main program
# after that. This means we can't load the main program at HIGH_LOAD_ADDR. Pick a load
# address well away from HIGH_LOAD_ADDR, to avoid overlap when relocating the code.

#define	IMAGE_BASE	0x200000
#define	BASE_OF_CODE	0x1000

	.section ".header", "ax", @progbits
	.code16

	.globl	boot
boot:
	# "MZ", the MS-DOS header signature.
	.byte 0x4d
	.byte 0x5a

        # In case we are booted by a legacy BIOS, print an error message.
        # Fortunately the MS-DOS header translates to harmless instructions.

	ljmp	$BOOT_SEG, $(error - boot)
error:
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	xorw	%sp, %sp
	sti
	cld

	movw	$error_msg, %si
0:	lodsb
	andb	%al, %al
	jz	wait
	movb	$0xe, %ah
	movw	$7, %bx
	int	$0x10
	jmp	0b

wait:
	# Allow the user to press a key, then reboot.
	xorw	%ax, %ax
	int	$0x16
	int	$0x19

	# int 0x19 should never return. In case it does, invoke the BIOS.
        # reset code.
	ljmp	$0xf000,$0xfff0

        # The PE header pointer.
	.org	0x3c
	.long	pe_header

error_msg:
	.ascii	"This is a UEFI bootable image\r\n"
	.ascii	"\n"
        .asciz  "Press any key to reboot\r\n"

pe_header:
	.ascii	"PE"
	.word 	0

coff_header:
#ifdef __x86_64__
	.word	0x8664				# Machine (x86-64)
#else
	.word	0x14c				# Machine (i386)
#endif
	.word	1				# NumberOfSections
	.long	0 				# TimeDateStamp
	.long	0				# PointerToSymbolTable
	.long	0				# NumberOfSymbols
	.word	section_table - optional_header	# SizeOfOptionalHeader
#ifdef __x86_64__
	.word	0x20f				# Characteristics
						# IMAGE_FILE_DEBUG_STRIPPED |
						# IMAGE_FILE_LOCAL_SYMS_STRIPPED |
						# IMAGE_FILE_LINE_NUMS_STRIPPED |
						# IMAGE_FILE_EXECUTABLE_IMAGE |
						# IMAGE_FILE_RELOCS_STRIPPED
#else
	.word	0x30f				# Characteristics.
						# IMAGE_FILE_32BIT_MACHINE |
						# IMAGE_FILE_DEBUG_STRIPPED |
						# IMAGE_FILE_LOCAL_SYMS_STRIPPED |
						# IMAGE_FILE_LINE_NUMS_STRIPPED |
						# IMAGE_FILE_EXECUTABLE_IMAGE |
						# IMAGE_FILE_RELOCS_STRIPPED
#endif

optional_header:
#ifdef __x86_64__
	.word	0x20b 				# PE32+ format
#else
	.word	0x10b				# PE32 format
#endif
	.byte	0x02				# MajorLinkerVersion
	.byte	0x14				# MinorLinkerVersion

	.long	_text_size			# SizeOfCode
	.long	0				# SizeOfInitializedData
	.long	0				# SizeOfUninitializedData

	.long	BASE_OF_CODE + 0x1e0		# AddressOfEntryPoint

	.long	BASE_OF_CODE			# BaseOfCode
#ifndef __x86_64__
	.long	0				# data
#endif

extra_header_fields:
#ifdef __x86_64__
	.quad	IMAGE_BASE			# ImageBase
#else
	.long	IMAGE_BASE			# ImageBase
#endif
	.long	4096				# SectionAlignment
	.long	512				# FileAlignment
	.word	0				# MajorOperatingSystemVersion
	.word	0				# MinorOperatingSystemVersion
	.word	0				# MajorImageVersion
	.word	0				# MinorImageVersion
	.word	0				# MajorSubsystemVersion
	.word	0				# MinorSubsystemVersion
	.long	0				# Win32VersionValue

	.long	BASE_OF_CODE + _init_size	# SizeOfImage
	.long	512				# SizeOfHeaders
	.long	0				# CheckSum
	.word	10				# Subsystem (EFI application)
	.word	0				# DllCharacteristics
#ifdef __x86_64__
	.quad	0				# SizeOfStackReserve
	.quad	0				# SizeOfStackCommit
	.quad	0				# SizeOfHeapReserve
	.quad	0				# SizeOfHeapCommit
#else
	.long	0				# SizeOfStackReserve
	.long	0				# SizeOfStackCommit
	.long	0				# SizeOfHeapReserve
	.long	0				# SizeOfHeapCommit
#endif
	.long	0				# LoaderFlags
	.long	0				# NumberOfRvaAndSizes

	# Section table
section_table:
	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0
	.long	_text_size			# VirtualSize
	.long	BASE_OF_CODE			# VirtualAddress
	.long	_text_size			# SizeOfRawData
	.long	_text_start			# PointerToRawData
	.long	0				# PointerToRelocations
	.long	0				# PointerToLineNumbers
	.word	0				# NumberOfRelocations
	.word	0				# NumberOfLineNumbers
	.long	0x60500020			# Characteristics (section flags)

# Emulate the Linux boot header, to allow loading by intermediate boot loaders.

	.org	497
setup_sects:
	.byte	SETUP_SECS
root_flags:
	.word	0
sys_size:
	.long	_sys_size
ram_size:
	.word	0
vid_mode:
	.word	0
root_dev:
	.word	0
boot_flag:
	.word	0xAA55
