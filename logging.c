#include <glib.h>
#include <stdlib.h>
#include <string.h>

void scribl_logfunc(const char *log_domain,
					GLogLevelFlags log_level,
					const char *message,
					gpointer user_data)
{
	char *base_level;

	base_level = getenv("SCRIBL_LOG");

	/* Log level 5 means log everything */
	if(!base_level)
		goto default_handler;

	if (strcmp(base_level, "5") == 0) {
		g_log_default_handler(log_domain, log_level, message, user_data);
	} else if (strcmp(base_level, "4") == 0) {
		if (log_level < G_LOG_LEVEL_DEBUG) {
			g_log_default_handler(log_domain, log_level, message, user_data);
		}
	} else {
		goto default_handler;
	}

default_handler:
	if (log_level < G_LOG_LEVEL_INFO)
		g_log_default_handler(log_domain, log_level, message, user_data);

}
