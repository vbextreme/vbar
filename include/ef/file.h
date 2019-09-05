#ifndef __EF_FILE_H__
#define __EF_FILE_H__

#include <ef/type.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifndef FILE_BUFFER
	#define FILE_BUFFER 4096
#endif

typedef struct stat stat_s;
typedef struct utimbuf utimbuf_s;
typedef FILE file_t;
typedef struct dirent dirent_s;

#define __fd_autoclose   __cleanup(fd_close_auto)
#define __file_autoclose __cleanup(file_close_auto)

void path_kill_back(char* path);
err_t path_current(char* path);
err_t path_home(char* path);
char* path_resolve(const char* path);
int file_exists(char* path);
char const* file_extension(char const* name);
void fd_close_auto(int* fd);
void file_close_auto(file_t** file);
FILE* file_dup(FILE* file, char* mode);
char* file_fd_slurp(size_t* len, int fd);


#endif
