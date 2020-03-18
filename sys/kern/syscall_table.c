#include <syscall_table.h>

#include <clone.h>
#include <debug.h>
#include <exec.h>
#include <fs.h>
#include <futex.h>
#include <mmap.h>
#include <proc.h>
#include <sch.h>
#include <sched.h>
#include <sections.h>
#include <sig.h>
#include <signal.h>
#include <sys/prctl.h>
#include <syscall.h>
#include <syscalls.h>
#include <timer.h>
#include <unistd.h>

/*
 * System call table
 *
 * Policy for system calls:
 * - If the system call arguments contain no pointers and the system call
 *   signature matches the C library function signature, call directly through
 *   to a function of the same name. e.g SYS_dup, SYS_dup2.
 * - Otherwise, for SYS_<fn> call a wrapper with the name sc_<fn>.
 * - However, we drop 64/32 suffixes as we don't support linux legacy interfaces.
 *
 * This means that kernel code must not call any function prefixed by sc_.
 *
 * Pointers to read only memory can be verified using either:
 * - u_address -- checks that pointer points to userspace memory
 * - u_access_ok -- checks that the memory region is in userspace and accessible
 * - u_strcheck -- checks that a userspace string is valid
 *
 * Pointers to writable memory must be verified using u_access_ok.
 *
 * However, note that even if access tests succeed a subsequent access could
 * fail if we context switch after the test and another thread unmaps the
 * region. This is OK on MMU systems, but on MPU systems we can't guarantee
 * that the memory won't be remapped by another process or the kernel. So, on
 * NOMMU or MPU systems the address space for the task needs to be locked. This
 * is implemented by u_access_begin/u_access_end.
 *
 * We make use of the MMU/MPU to trap and stub bad accesses to userspace. Bad
 * writes are discarded, bad reads return 0. A fault is marked in the thread
 * state and returned to userspace on syscall return.
 */
__fast_rodata const void *const
syscall_table[SYSCALL_TABLE_SIZE] = {
	[SYS_access] = sc_access,
	[SYS_brk] = sc_brk,
	[SYS_chdir] = sc_chdir,
	[SYS_chmod] = sc_chmod,				/* stub */
	[SYS_chown32] = sc_chown,
	[SYS_clock_gettime] = sc_clock_gettime,		/* stub */
	[SYS_clock_settime] = sc_clock_settime,
	[SYS_clone] = sc_clone,
	[SYS_close] = close,
	[SYS_dup2] = dup2,
	[SYS_dup] = dup,
	[SYS_execve] = sc_execve,
	[SYS_exit] = sc_exit,
	[SYS_exit_group] = sc_exit_group,
	[SYS_faccessat] = sc_faccessat,
	[SYS_fchmod] = fchmod,				/* stub */
	[SYS_fchmodat] = sc_fchmodat,			/* stub */
	[SYS_fchown32] = fchown,			/* stub */
	[SYS_fchownat] = sc_fchownat,			/* stub */
	[SYS_fcntl64] = sc_fcntl,
	[SYS_fork] = sc_fork,
	[SYS_fstat64] = sc_fstat,
	[SYS_fstatat64] = sc_fstatat,
	[SYS_fstatfs64] = sc_fstatfs,
	[SYS_fsync] = fsync,
	[SYS_futex] = sc_futex,
	[SYS_getcwd] = sc_getcwd,
	[SYS_getdents64] = sc_getdents,
	[SYS_geteuid32] = geteuid,
	[SYS_getitimer] = sc_getitimer,
	[SYS_getpgid] = getpgid,
	[SYS_getpid] = getpid,
	[SYS_getppid] = getppid,
	[SYS_getsid] = getsid,
	[SYS_gettid] = sc_gettid,
	[SYS_getuid32] = getuid,			/* no user support */
	[SYS_ioctl] = sc_ioctl,
	[SYS_kill] = kill,
	[SYS_lchown32] = sc_lchown,
	[SYS_lstat64] = sc_lstat,
	[SYS_madvise] = sc_madvise,
	[SYS_mkdir] = sc_mkdir,
	[SYS_mkdirat] = sc_mkdirat,
	[SYS_mknod] = sc_mknod,
	[SYS_mknodat] = sc_mknodat,
	[SYS_mmap2] = sc_mmap2,
	[SYS_mount] = sc_mount,
	[SYS_mprotect] = sc_mprotect,
	[SYS_munmap] = sc_munmap,
	[SYS_nanosleep] = sc_nanosleep,
	[SYS_open] = sc_open,
	[SYS_openat] = sc_openat,
	[SYS_pipe2] = sc_pipe2,
	[SYS_pipe] = sc_pipe,
	[SYS_prctl] = prctl,
	[SYS_pread64] = sc_pread,
	[SYS_preadv] = sc_preadv,
	[SYS_pwrite64] = sc_pwrite,
	[SYS_pwritev] = sc_pwritev,
	[SYS_read] = sc_read,
	[SYS_readlink] = sc_readlink,
	[SYS_readlinkat] = sc_readlinkat,
	[SYS_readv] = sc_readv,
	[SYS_reboot] = sc_reboot,
	[SYS_rename] = sc_rename,
	[SYS_renameat] = sc_renameat,
	[SYS_rmdir] = sc_rmdir,
	[SYS_rt_sigaction] = sc_rt_sigaction,
	[SYS_rt_sigprocmask] = sc_rt_sigprocmask,
	[SYS_rt_sigreturn] = sc_rt_sigreturn,
	[SYS_sched_get_priority_max] = sched_get_priority_max,
	[SYS_sched_get_priority_min] = sched_get_priority_min,
	[SYS_sched_yield] = sch_yield,
	[SYS_set_tid_address] = sc_set_tid_address,
	[SYS_setitimer] = sc_setitimer,
	[SYS_setpgid] = setpgid,
	[SYS_setsid] = setsid,
	[SYS_sigreturn] = sc_sigreturn,
	[SYS_stat64] = sc_stat,
	[SYS_statfs64] = sc_statfs,
	[SYS_statx] = sc_statx,			    /* stub */
	[SYS_symlink] = sc_symlink,
	[SYS_symlinkat] = sc_symlinkat,
	[SYS_sync] = sc_sync,
	[SYS_syslog] = sc_syslog,
	[SYS_tgkill] = sc_tgkill,
	[SYS_tkill] = sc_tkill,
	[SYS_umask] = umask,
	[SYS_umount2] = sc_umount2,
	[SYS_uname] = sc_uname,
	[SYS_unlink] = sc_unlink,
	[SYS_unlinkat] = sc_unlinkat,
	[SYS_utimensat] = sc_utimensat,		    /* no time support in FS */
	[SYS_vfork] = sc_vfork,
	[SYS_wait4] = sc_wait4,
	[SYS_write] = sc_write,
	[SYS_writev] = sc_writev,
#if UINTPTR_MAX == 0xffffffff
	[SYS__llseek] = sc_llseek,
#else
	[SYS_lseek] = lseek,
#endif
};
