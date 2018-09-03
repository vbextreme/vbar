#ifndef __EF_FILE_H__
#define __EF_FILE_H__

#include <vbar/type.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define EF_PATH_MAX PATH_MAX

#ifndef EF_FILE_BUFFER
	#define EF_FILE_BUFFER 4096
#endif

typedef struct stat stat_s;
typedef struct utimbuf utimbuf_s;
typedef FILE file_t;
typedef struct dirent dirent_s;

#define __ef_fd_autoclose   __ef_cleanup(ef_fd_close_auto)
#define __ef_file_autoclose __ef_cleanup(ef_file_close_auto)


err_t ef_path_current(char* path);
err_t ef_path_home(char* path);
int path_resolve(char* path, char* res);
int file_exists(char* path);
void ef_fd_close_auto(int* fd);
void ef_file_close_auto(file_t** file);




#endif
