#include <stdio.h>
#include <stdlib.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "uptime_rev.h"

//--------------------------------------------------------------------------------
// String constants
//--------------------------------------------------------------------------------
#define DAY		"day"
#define HOUR	"hr"
#define MINUTE	"min"
#define SECOND	"sec"

#define DAYS	"days"
#define HOURS	"hrs"
#define MINUTES	"mins"
#define SECONDS	"secs"

#define CONJUNCTION		"and"
#define RAM_DISK		"RAM Disk"
#define STR_PREFIX		"Uptime:"

#define STR_ERR_LOCK_DOS		"Failed to lock DOS list"
#define STR_ERR_FIND_RAM_DISK	"Failed to find RAM Disk"
#define STR_KS_TOO_OLD			"This program requires Kickstart 2.04 or higher"
#define STR_OS_TOO_OLD			"This program requires AmigaOS 2.04 or higher"

//--------------------------------------------------------------------------------
// Command line template for ReadArgs
//--------------------------------------------------------------------------------
#define TEMPLATE		"VER=VERSION/S,S=SHORT/S"

#define OPT_VERSION			0		// Show program version and exit
#define OPT_SHORT			1		// Short output (i.e. no prefix)
#define OPT_COUNT 			2

//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------
#define KICKSTART_MIN_VER	37		// Min Kickstart version required (37 = 2.04)
#define OS_MIN_VER			37		// Min AmigaOS version required (37 = 2.04)

//--------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------
// Embed version tag into binary
const char* version = VERSTAG;

// Function prototypes
BOOL CheckRequirements(void);


//--------------------------------------------------------------------------------
// main()
//--------------------------------------------------------------------------------
int main(void)
{
	struct 	RDArgs*	rdargs;
 	long	opts[OPT_COUNT] = {0};
	struct 	DosList* doslist;
	struct 	DateStamp ds, today;
	BOOL	verbose = TRUE;
	long	days, hours, minutes, seconds, ticks;
	int 	count = 0;
	int 	rc = RETURN_OK;

	if (!CheckRequirements())
		return RETURN_FAIL;

	// Parse command line arguments
	rdargs = ReadArgs(TEMPLATE, opts, NULL);
	{
		if (rdargs == NULL) {
			PrintFault(IoErr(), NULL);
			rc = RETURN_FAIL;
			goto exit;
		}

		// VERSION argument
		if (opts[OPT_VERSION]) {
			Printf("%s", VSTRING);
			FreeArgs(rdargs);
			// Terminate the program immediately after showing version info
			exit(RETURN_OK);  
		}

		// Determine mode based on arguments received
		if (opts[OPT_SHORT])	verbose = FALSE;	// Short output (no prefix)

	} // End of ReadArgs section

	// Lock the DOS list for reading
	doslist = LockDosList(LDF_VOLUMES | LDF_READ);
	if (doslist == NULL) {
		Printf("%s\n", STR_ERR_LOCK_DOS);
		rc = RETURN_FAIL;
		goto exit;
	}

	// Find the RAM Disk entry
	doslist = FindDosEntry(doslist, RAM_DISK, LDF_VOLUMES | LDF_READ);
	if (doslist == NULL) {
		Printf("%s\n", STR_ERR_FIND_RAM_DISK);
		rc = RETURN_FAIL;
		goto exit;
	}

	// Get the creation date/time of the RAM disk
	ds = doslist->dol_misc.dol_volume.dol_VolumeDate;

	// Get the current date/time
	DateStamp(&today);

	// Calculate the difference
	days 	= today.ds_Days - ds.ds_Days;
	minutes = today.ds_Minute - ds.ds_Minute;
	ticks 	= today.ds_Tick - ds.ds_Tick;

	// Adjust for negative values
	if (ticks < 0) {
		ticks += 60 * TICKS_PER_SECOND;
		minutes -= 1;
	}

	if (minutes < 0) {
		minutes += 1440;
		days -= 1;
	}

	// Calculate seconds and hours
	seconds = ticks / TICKS_PER_SECOND;
	hours = (minutes / 60);
	minutes = minutes % 60;

	// Count how many time periods we have to print
	if (days) count++;
	if (hours) count++;
	if (minutes) count++;
	if (seconds) count++;

	// Print the prefix
	if (verbose) 
		Printf("%s ", STR_PREFIX);

	// Print days
	if (days) {
		Printf("%ld %s", days, (days == 1) ? DAY : DAYS);
		count--;
	}

	// Print hours
	if (hours) {
		// Do we first need to print a comma or a conjunction?
		if (days && count > 1)
			Printf(", ");
		else if (days && count == 1)
			Printf(" %s ", CONJUNCTION);

		Printf("%ld %s", hours, (hours == 1) ? HOUR : HOURS);
		count--;
	}

	// Print minutes
	if (minutes) {
		// Do we first need to print a comma or a conjunction?
		if (days || hours) {
			if (count > 1)
				Printf(", ");
			else if (count == 1)
				Printf(" %s ", CONJUNCTION);
		}
		Printf("%ld %s", minutes, (minutes == 1) ? MINUTE : MINUTES);
	}

	// Print seconds
	if (seconds) {
		// Do we first need to print a conjunction?
		if (days || hours || minutes)
			Printf(" %s ", CONJUNCTION);
		Printf("%ld %s", seconds, (seconds == 1) ? SECOND : SECONDS);
	}

	if (!(days || hours || minutes || seconds)) {
		// If we have nothing to print, print 0 seconds
		Printf("0 %s", SECONDS);
	}

	// New line at the end
	Printf("\n");

exit:
	// Unlock the DOS list if it's still locked
	if (doslist) UnLockDosList(LDF_VOLUMES | LDF_READ);

	// Free the argument list
	if (rdargs)	FreeArgs(rdargs);

	return rc;
}


//--------------------------------------------------------------------------------
//	Checks that the Kickstart and Workbench versions meet the minimum requirements.
//	Returns TRUE if requirements are met, FALSE otherwise.
//--------------------------------------------------------------------------------
BOOL CheckRequirements(void)
{
	// Check Kickstart version
	if (SysBase->LibNode.lib_Version < KICKSTART_MIN_VER) {
		Printf("%s\n", STR_KS_TOO_OLD);
		return FALSE;
	}

	// Check Workbench version
	if (WorkbenchBase->lib_Version < OS_MIN_VER) {
		Printf("%s\n", STR_OS_TOO_OLD);
		return FALSE;
	}

	return TRUE;
}
