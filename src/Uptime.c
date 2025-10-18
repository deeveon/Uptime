#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "Uptime.h"
#include "uptime_rev.h"

//--------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------
STRPTR 	version = VERSTAG;			// Embed version tag into binary
OutFrmt format = FORMAT_NORMAL;
BOOL 	prefix = TRUE;

//--------------------------------------------------------------------------------
// Function prototypes
//--------------------------------------------------------------------------------
BOOL CheckRequirements(void);
BOOL SanitizeVolumeName(char* cleanName, const char* dirtyName);
struct DateStamp* GetVolumeCreationDate(STRPTR volumeName);
void PrintTimeDuration(OutFrmt format, long days, long hours, long minutes, long seconds);


//--------------------------------------------------------------------------------
// main()
//--------------------------------------------------------------------------------
int main(void)
{
	struct 	RDArgs*	rdargs;
 	long	opts[OPT_COUNT] = {0};
	struct 	DateStamp* creationDate = NULL;
	struct 	DateStamp* today = NULL;
	char	volumeName[MAX_VOL_NAME_LEN + 1] = STR_RAM_DISK;	// Default volume to check
	long	days, hours, minutes, seconds, ticks;
	int 	rc = RETURN_OK;

	// Check Kickstart and Workbench versions
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
			rc = RETURN_OK;
			goto exit;
		}

		// VOLUME argument
		if (opts[OPT_VOLUME]) {
			if (!SanitizeVolumeName(volumeName, (char*)opts[OPT_VOLUME])) {
				rc = RETURN_FAIL;
				goto exit;
			}
		}

		// Determine format based on arguments received
		if (opts[OPT_SHORT])	format = FORMAT_SHORT;		// Short output (no prefix)
		if (opts[OPT_FULL])		format = FORMAT_FULL;		// Full output (long prefix)
		if (opts[OPT_NOPREFIX])	prefix = FALSE;				// No prefix (DD:HH:MM:SS)

	} // End of argument parsing

	// Get the creation date of the RAM Disk
	creationDate = GetVolumeCreationDate(volumeName);
	if (creationDate == NULL) {
		// Printf("%s\n", STR_ERR_GET_CREATION);
		rc = RETURN_FAIL;
		goto exit;
	}

	today = malloc(sizeof(struct DateStamp));
	if (today == NULL) {
		Printf("%s %s\n", STR_ERR_ALLOC_MEM, "DateStamp");
		rc = RETURN_FAIL;
		goto exit;
	}

	// Get the current date/time
	today = DateStamp(today);

	if (today == NULL) {
		Printf("%s\n", STR_ERR_GET_CURR_TIME);
		rc = RETURN_FAIL;
		goto exit;
	}

	// Calculate the difference
	days 	= today->ds_Days - creationDate->ds_Days;
	minutes = today->ds_Minute - creationDate->ds_Minute;
	ticks 	= today->ds_Tick - creationDate->ds_Tick;

	// Adjust for negative values
	if (ticks < 0) {
		ticks += 60 * TICKS_PER_SECOND;
		minutes -= 1;
	}

	if (minutes < 0) {
		minutes += 1440;
		days -= 1;
	}

	// If the amount of time ends up negative for some reason (e.g. clock 
	// battery is dead), then return an error
	if (days < 0 || minutes < 0 || ticks < 0) {
		Printf("%s\n", STR_ERR_NEGATIVE_UPTIME);
		rc = RETURN_FAIL;
		goto exit;
	}

	// Calculate hours & seconds
	seconds = ticks / TICKS_PER_SECOND;
	hours 	= minutes / 60;
	minutes = minutes % 60;

	// Determine which prefix to print (if any)
	if (format == FORMAT_FULL)
		Printf("%s ", STR_FULL_PREFIX); // Full always has a prefix
	else if (prefix) {
		Printf("%s ", STR_NORM_PREFIX);
	}

	// Print the time duration
	PrintTimeDuration(format, days, hours, minutes, seconds);

	// Put a period at the end of the full format sentence,
	// otherwise just a newline
	if (format == FORMAT_FULL)
		Printf(".\n");
	else
		Printf("\n");

exit:
	// Free the argument list
	if (rdargs)	FreeArgs(rdargs);
	if (today)	free(today);
	if (creationDate) free(creationDate);

	return rc;
}


//--------------------------------------------------------------------------------
// Sanitizes/validates the specified volume name
// Returns TRUE if the name is valid after sanitization, FALSE otherwise.
//--------------------------------------------------------------------------------
BOOL SanitizeVolumeName(char* cleanName, const char* dirtyName)
{
	// Validate parameters
	if (dirtyName == NULL || strlen(dirtyName) == 0 || 
		strlen(dirtyName) <= 0 || strlen(dirtyName) > MAX_VOL_NAME_LEN)
	{
		Printf("%s\n", STR_ERR_INV_VOL_NAME);
		return FALSE;
	}

	if (cleanName == NULL) {
		Printf("%s: %s\n", STR_ERR_INV_POINTER, "cleanName");
		return FALSE;
	}

	// Copy the dirty name to the clean name buffer
	strncpy(cleanName, dirtyName, MAX_VOL_NAME_LEN);
	cleanName[MAX_VOL_NAME_LEN] = '\0';				// Ensure null termination

	// ReadArgs already filters out invalid characters, so we don't need to do
	// that here. Instead, we can just check for a trailing colon & remove it.
	if (cleanName[strlen(cleanName) - 1] == ':') {
		cleanName[strlen(cleanName) - 1] = '\0';
	}

	return TRUE;
}

//--------------------------------------------------------------------------------
// Outputs the time duration in the specified format to stdout.
//--------------------------------------------------------------------------------
void PrintTimeDuration(OutFrmt format, long days, long hours, long minutes, long seconds)
{
	int count = 0;

	// Count how many time periods to print
	if (days) count++;
	if (hours) count++;
	if (minutes) count++;
	if (seconds) count++;

	// Print the time duration based on the selected format & prefix options
	switch (format)
	{
		case FORMAT_FULL:
		case FORMAT_NORMAL:
			// Print days
			if (days) {
				Printf("%ld %s", days, (days == 1) ? STR_DAY : STR_DAYS);
				count--;
			}

			// Print hours
			if (hours) {
				// Do we first need to print a comma or a conjunction?
				if (days && count > 1)
					Printf(", ");
				else if (days && count == 1)
					Printf(" %s ", STR_CONJUNCTION);

				Printf("%ld %s", hours, (hours == 1) ? STR_HOUR : STR_HOURS);
				count--;
			}

			// Print minutes
			if (minutes) {
				// Do we first need to print a comma or a conjunction?
				if (days || hours) {
					if (count > 1)
						Printf(", ");
					else if (count == 1)
						Printf(" %s ", STR_CONJUNCTION);
				}
				Printf("%ld %s", minutes, (minutes == 1) ? STR_MINUTE : STR_MINUTES);
			}

			// Print seconds
			if (seconds) {
				// Do we first need to print a conjunction?
				if (days || hours || minutes)
					Printf(" %s ", STR_CONJUNCTION);
				Printf("%ld %s", seconds, (seconds == 1) ? STR_SECOND : STR_SECONDS);
			}

			// If we have nothing else to print, print 0 seconds
			if (!(days || hours || minutes || seconds)) {
				Printf("0 %s", STR_SECONDS);
			}

			break;

		case FORMAT_SHORT:
			// Printf("%02ld:%02ld:%02ld:%02ld", days, hours, minutes, seconds);
			// Printf("%ldd %ldh %ldm %lds", days, hours, minutes, seconds);
			if (days)
				Printf("%ld%s ", days, STR_DAY_SHORT);
			if (hours)
				Printf("%ld%s ", hours, STR_HOUR_SHORT);
			if (minutes)
				Printf("%ld%s ", minutes, STR_MINUTE_SHORT);
			if (seconds)
				Printf("%ld%s", seconds, STR_SECOND_SHORT);

			// If we have nothing else to print, print 0seconds
			if (!(days || hours || minutes || seconds)) {
				Printf("0%s", STR_SECOND_SHORT);
			}
			break;

		default:
			// Should never reach here
			Printf("Invalid format\n");
			break;

	} // End of switch
}


//--------------------------------------------------------------------------------
// Gets the creation date of the specified volume name (e.g. "Workbench").
// Returns a pointer to a DateStamp if successful, otherwise NULL.
//--------------------------------------------------------------------------------
struct DateStamp* GetVolumeCreationDate(STRPTR volumeName)
{
	struct 	DosList*	dl;
	struct 	DateStamp* 	ds = NULL;
	ULONG 	flags = LDF_VOLUMES | LDF_READ;

	// Validate parameters
	// TODO: Are there any other checks we should do here on the volume name?
	if (volumeName == NULL) {
		Printf("%s\n", STR_ERR_INV_VOL_NAME);
		return NULL;
	}

	// Lock the DOS list for reading
	dl = LockDosList(flags);
	if (dl == NULL) {
		Printf("%s\n", STR_ERR_LOCK_DOS);
		goto error;
	}

	// Find the specified volume entry
	dl = FindDosEntry(dl, volumeName, flags);
	if (dl == NULL) {
		Printf("%s: %s\n", STR_ERR_FIND_VOLUME, volumeName);
		goto error;
	}

	// Allocate memory for the DateStamp structure
	ds = malloc(sizeof(struct DateStamp));
	if (ds == NULL) {
		Printf("%s %s\n", STR_ERR_ALLOC_MEM, "DateStamp");
		goto error;
	}

	// Get the creation date/time of the specified volume
	*ds = dl->dol_misc.dol_volume.dol_VolumeDate;

error:

	// Unlock the DOS list
	UnLockDosList(flags);

	// Return the DateStamp structure or NULL if there was an error
	if (ds)
		return ds;
	else
		return NULL;
}


//--------------------------------------------------------------------------------
//	Checks that the Kickstart and Workbench versions meet the minimum requirements.
//	Returns TRUE if requirements are met, FALSE otherwise.
//--------------------------------------------------------------------------------
BOOL CheckRequirements(void)
{
	// Check Kickstart version
	if (SysBase->LibNode.lib_Version < KICKSTART_MIN_VER) {
		Printf("%s\n", STR_ERR_KS_TOO_OLD);
		return FALSE;
	}

	// Check Workbench version
	if (WorkbenchBase->lib_Version < OS_MIN_VER) {
		Printf("%s\n", STR_ERR_OS_TOO_OLD);
		return FALSE;
	}

	return TRUE;
}
