/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gfile/options.h
 * @brief   GFILE - File IO options header file.
 *
 * @addtogroup GFILE
 * @{
 */

#ifndef _GFILE_OPTIONS_H
#define _GFILE_OPTIONS_H

/**
 * @name    GFILE Functionality to be included
 * @{
 */
	/**
	 * @brief   Include printg, fprintg, sprintg etc functions
	 * @details	Defaults to FALSE
	 */
	#ifndef GFILE_NEED_PRINTG
		#define GFILE_NEED_PRINTG		FALSE
	#endif
	/**
	 * @brief   Include scang, fscang, sscang etc functions
	 * @details	Defaults to FALSE
	 */
	#ifndef GFILE_NEED_SCANG
		#define GFILE_NEED_SCANG		FALSE
	#endif
	/**
	 * @brief   Map all the stdio functions to their GFILE equivalent
	 * @details	Defaults to FALSE
	 * @note	This replaces the functions in stdio.h with equivalents
	 * 			- Do not include stdio.h as it has different conflicting definitions.
	 */
	#ifndef GFILE_NEED_STDIO
		#define GFILE_NEED_STDIO		FALSE
	#endif
	/**
	 * @brief   Include the ROM file system
	 * @details	Defaults to FALSE
	 * @note	To ensure that you are opening a file on the ROM file system, prefix
	 * 			its name with "S|" (the letter 'S', followed by a vertical bar).
	 * @note	This requires a file called romfs_files.h to be included in the
	 * 			users project. This file includes all the files converted to .h files
	 * 			using the file2c utility using the "-r" flag.
	 */
	#ifndef GFILE_NEED_ROMFS
		#define GFILE_NEED_ROMFS		FALSE
	#endif
	/**
	 * @brief   Include the RAM file system
	 * @details	Defaults to FALSE
	 * @note	To ensure that you are opening a file on the RAM file system, prefix
	 * 			its name with "R|" (the letter 'R', followed by a vertical bar).
	 * @note	You must also define GFILE_RAMFS_SIZE with the size of the file system
	 * 			to be allocated in RAM.
	 */
	#ifndef GFILE_NEED_RAMFS
		#define GFILE_NEED_RAMFS		FALSE
	#endif
	/**
	 * @brief   Include the FAT file system driver
	 * @details	Defaults to FALSE
	 * @note	To ensure that you are opening a file on the FAT file system, prefix
	 * 			its name with "F|" (the letter 'F', followed by a vertical bar).
	 * @note	You must separately include the FATFS library and code.
	 */
	#ifndef GFILE_NEED_FATFS
		#define GFILE_NEED_FATFS		FALSE
	#endif
	/**
	 * @brief   Include the operating system's native file system
	 * @details	Defaults to FALSE
	 * @note	To ensure that you are opening a file on the native file system, prefix
	 * 			its name with "N|" (the letter 'N', followed by a vertical bar).
	 * @note	If defined then the gfileStdOut and gfileStdErr handles
	 * 			use the operating system equivalent stdio and stderr.
	 * 			If it is not defined the gfileStdOut and gfileStdErr io is discarded.
	 */
	#ifndef GFILE_NEED_NATIVEFS
		#define GFILE_NEED_NATIVEFS		FALSE
	#endif
/**
 * @}
 *
 * @name    GFILE Optional Parameters
 * @{
 */
	/**
	 * @brief   The maximum number of open files
	 * @note	This count excludes gfileStdIn, gfileStdOut and gfileStdErr
	 * 			(if open by default).
	 */
	#ifndef GFILE_MAX_GFILES
		#define GFILE_MAX_GFILES		3
	#endif
	/**
	 * @brief   The size in bytes of the RAM file system
	 */
	#ifndef GFILE_RAMFS_SIZE
		#define GFILE_RAMFS_SIZE		0
	#endif
/** @} */

#endif /* _GFILE_OPTIONS_H */
/** @} */
