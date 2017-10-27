#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "interceptor.h"


static int last_child;

int vsyscall_arg(int sno, int n, ...) {

	va_list va;
	long args[6];
	int i, ret;
	
	va_start(va, n);
	for(i = 0; i < n; i++) {
		args[i] = va_arg(va, long);
	}
	va_end(va);
	
	ret = syscall(sno, args[0], args[1], args[2]);
	if(ret) ret = -errno;
	return ret;
}

#define test(s, a, t) \
({\
	int i;\
	char dummy[1024];\
	\
	sprintf(dummy, s, a);\
	printf("test: %s", dummy); \
	for(i=0; i<60-strlen(dummy); i++)\
		putchar('.');\
	if (!(t))\
		printf("failed\n");\
	else\
		printf("passed\n");\
	fflush(stdout);\
})


void clear_log() {
	system("dmesg -c &> /dev/null");
}


int do_intercept(int syscall, int status) {
	test("%d intercept", syscall, vsyscall_arg(MY_CUSTOM_SYSCALL, 3, REQUEST_SYSCALL_INTERCEPT, syscall, getpid()) == status);
	return 0;
}


int do_release(int syscall, int status) {
	test("%d release", syscall, vsyscall_arg(MY_CUSTOM_SYSCALL, 3, REQUEST_SYSCALL_RELEASE, syscall, getpid()) == status);
	return 0;
}


/** 
 * Run the tester as a non-root user, and basically run do_nonroot
 */
void do_as_guest(const char *str, int args1, int args2) {

	char cmd[1024];
	char cmd2[1024];
	char* exec[]={"bash", "-c", cmd2, NULL};

	sprintf(cmd, str, args1, args2);
	sprintf(cmd2, "su nobody -c '%s' ", cmd);
	switch ((last_child = fork()))  {
		case -1:
			assert(0);
		case 0:
			execvp("/bin/bash", exec);
			assert(0);
		default:
			waitpid(last_child, NULL, 0);
	}
}


int do_nonroot(int syscall) {
	do_intercept(syscall, -EPERM);
	do_release(syscall, -EPERM);
	return 0;
}


void test_syscall(int syscall) {
	//clear_log();
	do_intercept(syscall, 0);
	do_intercept(syscall, -EBUSY);
	do_as_guest("./test_intercept nonroot %d", syscall, 0);
	do_release(syscall, 0);
}


int main(int argc, char **argv) {

	srand(time(NULL));

	if (argc>1 && strcmp(argv[1], "intercept") == 0) 
		return do_intercept(atoi(argv[2]), atoi(argv[3]));

	if (argc>1 && strcmp(argv[1], "release") == 0)
		return do_release(atoi(argv[2]), atoi(argv[3]));

	if (argc>1 && strcmp(argv[1], "nonroot") == 0)
		return do_nonroot(atoi(argv[2]));

	test("insmod interceptor.ko %s", "", system("insmod interceptor.ko") == 0);
	test("bad MY_CUSTOM_SYSCALL args%s", "",  vsyscall_arg(MY_CUSTOM_SYSCALL, 3, 100, 0, 0) == -EINVAL);
	do_intercept(MY_CUSTOM_SYSCALL, -EINVAL);
	do_release(MY_CUSTOM_SYSCALL, -EINVAL);
	do_intercept(-1, -EINVAL);
	do_release(-1, -EINVAL);

	do_intercept(__NR_exit, 0);
	do_release(__NR_exit, 0);

	test_syscall(SYS_open);
	/* The above line of code tests SYS_open.
	   Feel free to add more tests here for other system calls, 
	   once you get everything to work; check Linux documentation
	   for other syscall number definitions.  */

	test("rmmod interceptor.ko %s", "", system("rmmod interceptor") == 0);
	return 0;
}

