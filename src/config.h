/*
 * Title:			AGON MOS - MOS config
 * Author:			Dean Belfield
 * Created:			19/09/2022
 * Last Updated:	19/09/2022
 * 
 * Modinfo:
 */

#ifndef CONFIG_H
#define CONFIG_H

#define	enable_config	1					// 0 = disable boot config loading, 1 = enable

#define MOS_prompt '*'						// MOS prompt character
#define MOS_maxOpenFiles 8					// Maximum number of files that mos_FOPEN can open at the same time
#define MOS_defaultLoadAddress	0x040000	// Default load address for LOAD and RUN commands

#endif CONFIG_H