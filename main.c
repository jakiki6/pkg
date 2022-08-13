#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <ftw.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>

void *safe_malloc(size_t size) {
	void *ptr = malloc(size);

	if (ptr == NULL) {
		exit(2);
	}

	memset(ptr, size, 0);

	return ptr;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	remove(fpath);
}

void usage() {
	struct utsname *buf = safe_malloc(sizeof(struct utsname));
	uname(buf);

	printf("pkg 0.0.1 (%s) running on %s\n", buf->machine, buf->sysname);
	printf("usage: pkg command [arguments]\n");
	printf("commands:\n");
	printf("  add <path> : add package to system\n");
	printf("  del <name> : remove package from system\n");
	exit(1);
}

void add(char *path) {
	printf("[+] Installing file '%s'\n", path);

	char *tempdir = safe_malloc(256);
	path = realpath(path, NULL);

	snprintf(tempdir, 255, "/tmp/pkg_%u", random());
	if (mkdir(tempdir, 0700)) {
		printf("[+] Couldn't create temporary directory\n");
		exit(2);
	}

	chdir(tempdir);

	int pid;
	if (pid = fork()) {
		int status;
		waitpid(pid, &status, 0);

		if (status) {
			printf("[+] Untar failed\n");
			exit(2);
		}
	} else {
		char *argv[] = {"/bin/tar", "fx", path, 0};

		execvp("/bin/tar", argv);
	}

	if (pid = fork()) {
		int status;
		waitpid(pid, &status, 0);

		if (status) {
			printf("[+] Installation failed with error code %i\n", status);
		}
	} else {
		setenv("UTILS", "/usr/share/pkg/utils.sh", 0);
		setenv("PKG_PREFIX", "", 0);
		setenv("PKG_LIB", "/usr/var/pkg", 0);

		char *argv[] = {"./pkg_install", 0};

		if (execvp("./pkg_install", argv)) {
			exit(1);
		};
	}

	nftw(tempdir, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
	rmdir(tempdir);
	free(path);
}

void del(char *name) {
	printf("[+] Uninstalling '%s'\n", name);

	char *path = safe_malloc(256);
	snprintf(path, 255, "/usr/var/pkg/%s", name);

	chdir(path);

	int pid;
	if (pid = fork()) {
		int status;
		waitpid(pid, &status, 0);

		if (status) {
			printf("[+] Uninstallation failed with error code %i\n", status);
		}
	} else {
		setenv("UTILS", "/usr/share/pkg/utils.sh", 0);
		setenv("PKG_PREFIX", "", 0);
		setenv("PKG_LIB", "/usr/var/pkg", 0);

		char *argv[] = {"./pkg_uninstall", 0};

		if (execvp("./pkg_uninstall", argv)) {
			exit(1);
		};
        }
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		usage();
	}

	srandom(time(NULL));

	if (geteuid() != 0) {
		printf("[+] I need superuser privileges\n");
		exit(3);
	}

	if (strcmp(argv[1], "add") == 0) {
		if (argc < 3) {
			printf("[+] Path is missing\n");
			return 1;
		}

		add(argv[2]);
	} else if (strcmp(argv[1], "del") == 0) {
		if (argc < 3) {
			printf("[+] Name is missing\n");
			return 1;
		}

		del(argv[2]);
	} else {
		usage();
	}
}
