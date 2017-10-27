#ifndef _COMMON_H
#define _COMMON_H

#define REQUEST_SYSCALL_INTERCEPT       1
#define REQUEST_SYSCALL_RELEASE         2
#define REQUEST_START_MONITORING        3
#define REQUEST_STOP_MONITORING         4

#define MY_CUSTOM_SYSCALL               0

#ifdef __KERNEL__

asmlinkage long my_syscall(int cmd, int syscall, int pid);

#define log_message(pid, syscall, arg1, arg2, arg3, arg4, arg5, arg6) \
	printk(KERN_DEBUG "[%x]%lx(%lx,%lx,%lx,%lx,%lx,%lx)\n", pid, \
		syscall, \
		arg1, arg2, arg3, arg4, arg5, arg6 \
	);
#endif


#endif /* _COMMON_H */
