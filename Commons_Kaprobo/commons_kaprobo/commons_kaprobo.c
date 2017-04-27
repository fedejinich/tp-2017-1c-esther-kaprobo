#include "commons_kaprobo.h"

t_log* iniciarLog(char* nombreDelLog,char* nombreDelProceso) {
	remove(nombreDelLog);
	return log_create(nombreDelLog,nombreDelProceso,0,LOG_LEVEL_INFO);
}
