#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <pthread.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#define  DEFAULT_CHUNK  262144  /* 256k */
static const char *dirpath = "/home/mungkin";
char playlist[1000][1000];
int idx=0;

bool ext_match(const char *name, const char *ext){
	size_t nl = strlen(name), el = strlen(ext);
	return nl >= el && !strcmp(name + nl - el, ext);
}

int copy_file(const char *target, const char *source, const size_t chunk){
    const size_t size = (chunk > 0) ? chunk : DEFAULT_CHUNK;
    char        *data, *ptr, *end;
    ssize_t      bytes;
    int          ifd, ofd, err;

    /* NULL and empty file names are invalid. */
    if (!target || !*target || !source || !*source)
        return EINVAL;

    ifd = open(source, O_RDONLY);
    if (ifd == -1){
        return errno;}

    /* Create output file; fail if it exists (O_EXCL): */
    ofd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (ofd == -1) {
        err = errno;
        close(ifd);
        return err;
    }

    /* Allocate temporary data buffer. */
    data = malloc(size);
    if (!data) {
        close(ifd);
        close(ofd);
        /* Remove output file. */
        unlink(target);
        return ENOMEM;
    }

    /* Copy loop. */
    while (1) {

        /* Read a new chunk. */
        bytes = read(ifd, data, size);
        if (bytes < 0) {
            if (bytes == -1)
                err = errno;
            else
                err = EIO;
            free(data);
            close(ifd);
            close(ofd);
            unlink(target);
            return err;
        } else
        if (bytes == 0)
            break;

        /* Write that same chunk. */
        ptr = data;
        end = data + bytes;
        while (ptr < end) {

            bytes = write(ofd, ptr, (size_t)(end - ptr));
            if (bytes <= 0) {
                if (bytes == -1)
                    err = errno;
                else
                    err = EIO;
                free(data);
                close(ifd);
                close(ofd);
                unlink(target);
                return err;
            } else
                ptr += bytes;
        }
    }

    free(data);

    err = 0;
    if (close(ifd))
        err = EIO;
    if (close(ofd))
        err = EIO;
    if (err) {
        unlink(target);
        return err;
    }

    return 0;
}

void move_mp3_to(char* path){
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    if (d){
        struct dirent *p;

        while (p=readdir(d)){
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..") || p->d_name[0]=='.'){
                continue;
            }

            len = path_len + strlen(p->d_name) + 2; 
            buf = malloc(len);

            if (buf){
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)){
                    if (S_ISDIR(statbuf.st_mode)){
                        move_mp3_to(buf);
                    }else{
                        if(ext_match(buf, ".mp3")){
                            //copy to root
                            printf("%s\n", buf);
							if(!strcmp(path, dirpath)){
								strcpy(playlist[idx++], "p->d_nameALIAS_NAMAYANGSALAH");
							}else{
                            	strcpy(playlist[idx++], p->d_name);
							}
                            char tpath[1000];
                            strcpy(tpath, dirpath);
                            strcat(tpath, "/");
                            strcat(tpath, p->d_name);
                            if(copy_file(tpath, buf, 0) == 17){
								strcpy(playlist[idx-1], "p->d_nameALIAS_NAMAYANGSALAH");
							}
                        }
                    }
                }
                free(buf);
            }
        }
        closedir(d);
    }
}

void xmp_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    // char fpath[1000];
    // strcpy(fpath, dirpath);
    // move_mp3_to(fpath);
}

void xmp_destroy(void *private_data){
    if(chdir(dirpath)<0){
        exit(0);
    }
    int j;
    for(j=0;j<idx;j++){
        remove(playlist[j]);
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = readlink(fpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
        
        if(S_ISDIR(st.st_mode) || !ext_match(de->d_name, ".mp3")){
            continue;
        }
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	/* On Linux this could just be 'mknod(fpath, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;
	char fpath[1000], tpath[1000];
	sprintf(fpath,"%s%s",dirpath,from);
	sprintf(tpath,"%s%s",dirpath,to);

	res = symlink(fpath, tpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
	char fpath[1000], tpath[1000];
	sprintf(fpath,"%s%s",dirpath,from);
	sprintf(tpath,"%s%s",dirpath,to);

	res = rename(fpath, tpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;
	char fpath[1000], tpath[1000];
	sprintf(fpath,"%s%s",dirpath,from);
	sprintf(tpath,"%s%s",dirpath,to);

	res = link(fpath, tpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.init       = xmp_init,
    .destroy    = xmp_destroy,
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create         = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
    char fpath[1000];
    strcpy(fpath, dirpath);
    move_mp3_to(fpath);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}