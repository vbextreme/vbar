#include <ef/type.h>
#include <ef/file.h>
#include <ef/memory.h>
#include <ef/strong.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <utime.h>
#include <pwd.h>

void path_kill_back(char* path){
	char* bs = strrchr(path, '/');
	if( bs ) *bs = 0;
}

err_t path_current(char* path){
    catch_null( getcwd(path, PATH_MAX) ){
		dbg_error("getcwd");
		dbg_errno();
		return errno;
	}
	return 0;
}

err_t path_home(char* path){
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

char* path_resolve(const char* path){
	char tmp[PATH_MAX];

	if( *path == '~' ){
		if( path_home(tmp) ){
			return NULL;
		}
		strcpy(&tmp[strlen(tmp)], &path[1]);
		return str_dup(tmp,0);
	}
	else if( *path == '.' ){
		path_current(tmp);
		if( path[1] == '.' ){
			path_kill_back(tmp);
			if( path[2] ){
				size_t l = strlen(tmp);
				if( path[2] != '/' ){
					tmp[l++] = '/';
					tmp[l] = 0;
				}
				strcpy(&tmp[strlen(tmp)], &path[2]);
				return str_dup(tmp,0);
			}
			return str_dup(tmp, 0);
		}
		if( path[1] ){
			size_t l = strlen(tmp);
			if( path[1] != '/' ){
				tmp[l++] = '/';
				tmp[l] = 0;
			}
			strcpy(&tmp[strlen(tmp)], &path[1]);
			return str_dup(tmp,0);
		}
		return str_dup(tmp, 0);
	}
	else if( *path != '/' ){
		path_current(tmp);	
		size_t l = strlen(tmp);
		tmp[l++] = '/';
		tmp[l] = 0;
		strcpy(&tmp[strlen(tmp)], &path[1]);
		return str_dup(tmp,0);
	}
	return str_dup(path,0);
}

char const* file_extension(char const* name){
	size_t p = strlen(name)-1;
	while( p > 0 && name[p] != '.' ) --p;
	return name[p] == '.' ? &name[p+1] : NULL;
}

int file_exists(char* path){
	stat_s b;
	catch_posix( stat(path, &b) ){
		dbg_warning("file %s not exist", path);
		dbg_errno();
		return 0;
	}
	return 1;
}

void fd_close_auto(int* fd){
	if( *fd == -1 ){
		return;
	}
	close(*fd);
	*fd = -1;
}

void file_close_auto(file_t** file){
	if( *file == NULL ){
		return;
	}
	fclose(*file);
	*file = NULL;
}

FILE* file_dup(FILE* file, char* mode){
	int fd = fileno(file);
	int newfd = dup(fd);
	return fdopen(newfd, mode);
}

char* file_fd_slurp(size_t* len, int fd){
	ssize_t nr;
	size_t size = FILE_BUFFER;
	size_t l = 0;
	char* buf = mem_many(char, FILE_BUFFER);
	iassert(buf);
	char* cur = buf;
	while( (nr = read(fd, cur, size)) != 0 ){
		if( nr < 0 && errno != EAGAIN ) break;
		l+=nr;
		iassert( size >= (size_t)nr );
		size -= nr;
		if( size == 0 ){
			iassert( 0 == 1 );
			char* tmp = realloc(buf, l + FILE_BUFFER);
			iassert(tmp);
			buf = tmp;
			size = FILE_BUFFER;
		}
		cur = buf+l;
	}
	char* tmp = realloc(buf, l+1);
	iassert(tmp);
	buf = tmp;
	buf[l] = 0;
	*len = l;
	return buf;
}

