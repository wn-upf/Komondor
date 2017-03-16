#ifndef _AUX_LOGGER_
#define _AUX_LOGGER_

// Logger: struct containing the attributes needed for writting logs in a file
struct Logger
{
	int save_logs;		// Flag for activating the log writting
	FILE *file;			// File for writting logs
	char *head_string;	// Header string (to be passed as argument when it is needed to write info from other class or component)

	// TODO: create 'getter' methods
};

#endif
