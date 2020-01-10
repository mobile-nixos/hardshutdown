/*
 * Heavily based on busybox's "hardshutdown" example.
 *  - https://busybox.net/license.html
 * Thus released as GPL v2
 *
 * Original author: Denys Vlasenko (2018)
 */
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>    /* puts */
#include <time.h>     /* nanosleep */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * Magic values required to use _reboot() system call.
 */
#define LINUX_REBOOT_MAGIC1 0xfee1dead
#define LINUX_REBOOT_MAGIC2 672274793
#define LINUX_REBOOT_MAGIC2A 85072278
#define LINUX_REBOOT_MAGIC2B 369367448
/*
 * Commands accepted by the _reboot() system call.
 *
 * RESTART     Restart system using default command and mode.
 * HALT        Stop OS and give system control to ROM monitor, if any.
 * CAD_ON      Ctrl-Alt-Del sequence causes RESTART command.
 * CAD_OFF     Ctrl-Alt-Del sequence sends SIGINT to init task.
 * POWER_OFF   Stop OS and remove all power from system, if possible.
 * RESTART2    Restart system using given command string.
 */
#define LINUX_REBOOT_CMD_RESTART    0x01234567
#define LINUX_REBOOT_CMD_HALT       0xCDEF0123
#define LINUX_REBOOT_CMD_CAD_ON     0x89ABCDEF
#define LINUX_REBOOT_CMD_CAD_OFF    0x00000000
#define LINUX_REBOOT_CMD_POWER_OFF  0x4321FEDC
#define LINUX_REBOOT_CMD_RESTART2   0xA1B2C3D4

static int my_reboot(int cmd, void *arg)
{
	return syscall(__NR_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cmd, arg);
}

static void do_reboot(void)
{
	my_reboot(LINUX_REBOOT_CMD_RESTART, "");
}
static void do_reboot2(void * arg)
{
	my_reboot(LINUX_REBOOT_CMD_RESTART2, arg);
}
static void do_poweroff(void)
{
	my_reboot(LINUX_REBOOT_CMD_POWER_OFF, "");
}
static void do_halt(void)
{
	my_reboot(LINUX_REBOOT_CMD_HALT, "");
}

static void usage(void)
{
	puts(
	    "Usage: hardshutdown -h|-r|-p [NN]\n"
	    "	NN - seconds to sleep before requested action"
	);
	exit(1);
}

enum action_t {
	SHUTDOWN,	// do nothing
	HALT,
	POWEROFF,
	REBOOT
};

int main(int argc, char *argv[])
{
	struct timespec t = {0,0};
	enum action_t action = SHUTDOWN;
	int c, i;
	char *prog, *ptr;

	//if (*argv[0] == '-') argv[0]++; /* allow shutdown as login shell */
	prog = argv[0];
	ptr = strrchr(prog,'/');
	if (ptr) {
		prog = ptr+1;
	}

	if (strcmp(prog, "reboot") == 0) {
		action = REBOOT;
	}

	if (strcmp(prog, "poweroff") == 0) {
		action = POWEROFF;
	}

	if (action == SHUTDOWN) {
		for (c=1; c < argc; c++) {
			if (argv[c][0] >= '0' && argv[c][0] <= '9') {
				t.tv_sec = strtol(argv[c], NULL, 10);
				continue;
			}
			if (argv[c][0] != '-') {
				usage();
				return 1;
			}
			for (i=1; argv[c][i]; i++) {
				switch (argv[c][i]) {
				case 'h':
					action = HALT;
					break;
				case 'p':
					action = POWEROFF;
					break;
				case 'r':
					action = REBOOT;
					argc = 1; /* Hack */
					break;
				default:
					usage();
					return 1;
				}
			}
		}
	}

	if (action == SHUTDOWN) {
		usage();
		return 1;
	}

	while (nanosleep(&t,&t) < 0) {
		if (errno != EINTR) {
			break;
		}
	}

	switch (action) {
	case HALT:
		do_halt();
		break;
	case POWEROFF:
		do_poweroff();
		break;
	case REBOOT:
		if (argc > 1) {
			do_reboot2(argv[1]);
		}
		else {
			do_reboot();
		}
		break;
	default: /* SHUTDOWN */
		break;
	}
	return 1;
}
