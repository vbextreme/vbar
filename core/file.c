#include <vbar/type.h>
#include <vbar/file.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <utime.h>
#include <pwd.h>

__ef_can_unused err_t ef_path_current(char* path){
    catch_null( getcwd(path, EF_PATH_MAX) ){
		dbg_error("getcwd");
		dbg_errno();
		return errno;
	}
	return 0;
}

__ef_can_unused err_t ef_path_home(char* path){
        char *hd;
        if( (hd = secure_getenv("HOME")) == NULL ){
                struct passwd* spwd = getpwuid(getuid());
                catch_null( spwd ){
                        dbg_error("no home available");
                        dbg_errno();
                        *path = 0;
                        return errno;
                }
                strcpy(path, spwd->pw_dir);
        }
        else{
                strcpy(path, hd);
        }
        return 0;
}

__ef_can_unused int path_resolve(char* path, char* res){
	if( *path == '~' ){
		if( ef_path_home(res) ){
			return -1;
		}
		strcpy(&res[strlen(res)], &path[1]);
		return 0;
	}
	strcpy(res, path);
	return 0;
}

__ef_can_unused int file_exists(char* path){
	stat_s b;
	catch_posix( stat(path, &b) ){
		dbg_warning("file %s not exist", path);
		dbg_errno();
		return 0;
	}
	return 1;
}

__ef_can_unused void ef_fd_close_auto(int* fd){
	if( *fd == -1 ){
		return;
	}
	close(*fd);
	*fd = -1;
}

__ef_can_unused void ef_file_close_auto(file_t** file){
	if( *file == NULL ){
		return;
	}
	fclose(*file);
	*file = NULL;
}


