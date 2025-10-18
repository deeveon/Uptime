#ifndef UPTIME_H
#define UPTIME_H

//--------------------------------------------------------------------------------
// Enums & typedefs
//--------------------------------------------------------------------------------

typedef enum OutFrmt {
	FORMAT_FULL,
	FORMAT_NORMAL,
	FORMAT_SHORT
} OutFrmt;

//--------------------------------------------------------------------------------
// String constants
//--------------------------------------------------------------------------------
#define STR_DAY					"day"
#define STR_HOUR				"hr"
#define STR_MINUTE				"min"
#define STR_SECOND				"sec"

#define STR_DAYS				"days"
#define STR_HOURS				"hrs"
#define STR_MINUTES				"mins"
#define STR_SECONDS				"secs"

#define STR_DAY_SHORT			"d"
#define STR_HOUR_SHORT			"h"
#define STR_MINUTE_SHORT		"m"
#define STR_SECOND_SHORT		"s"

#define STR_CONJUNCTION			"and"
#define STR_FULL_PREFIX			"The system has been running for"
#define STR_NORM_PREFIX			"Uptime:"
#define STR_RAM_DISK			"RAM Disk"

#define STR_ERR_KS_TOO_OLD		"This program requires Kickstart 2.04 or higher"	// See KICKSTART_MIN_VER below
#define STR_ERR_OS_TOO_OLD		"This program requires AmigaOS 2.04 or higher"		// See OS_MIN_VER below

#define STR_ERR_ALLOC_MEM		"Failed to allocate memory for"
#define STR_ERR_FIND_VOLUME		"Volume not found"
// #define STR_ERR_GET_CREATION	"Failed to get volume creation date"
#define STR_ERR_GET_CURR_TIME	"Failed to get current date/time"
#define STR_ERR_INV_POINTER		"Invalid pointer"
#define STR_ERR_INV_VOL_NAME	"Invalid volume name"
#define STR_ERR_LOCK_DOS		"Failed to lock DOS list"
#define STR_ERR_NEGATIVE_UPTIME	"Calculated uptime is negative. Please check that " \
								"the system clock is set correctly or use the VOLUME " \
								"option to check against a different volume."
#define STR_ERR_VOL_NAME_LEN	"Volume name is too long"

//--------------------------------------------------------------------------------
// Command line template for ReadArgs
//--------------------------------------------------------------------------------
#define TEMPLATE		"VER=VERSION/S,VOL=VOLUME/K,F=FULL/S,S=SHORT/S,NP=NOPREFIX/S"

#define OPT_VERSION			0		// Show program version and exit
#define OPT_VOLUME			1		// Volume to check (default: RAM Disk)
#define OPT_FULL			2		// Full output
#define OPT_SHORT			3		// Short output
#define OPT_NOPREFIX		4		// No prefix
#define OPT_COUNT 			5

//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------
// If you change these, be sure to change the corresponding error string above also
#define KICKSTART_MIN_VER	37		// Min Kickstart version required (37 = 2.04)
#define OS_MIN_VER			37		// Min AmigaOS version required (37 = 2.04)

#define MAX_VOL_NAME_LEN	30		// Max length of volume name


#endif // UPTIME_H
