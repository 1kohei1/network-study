#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include "sock.h"
#include "ether.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "param.h"
#include "cmd.h"

int EndFlag = 0;
int DeviceSoc;
PARAM Param;

// Handle ping request to the program
void *MyEthThread(void *arg) {
	int nready;
	struct pollfd targets[1];
	u_int8_t buf[2048];
	int len;

	// Which file descriptor we would like to watch
	targets[0].fd = DeviceSoc;
	// What event to watch defined in bits/poll.h
	// POLLIN: there was data to read
	// POLLERR: error happened
	targets[0].events = POLLIN | POLLERR;

	// While the program is not finished,
	while (EndFlag == 0) {
		// poll: it waits for one of a set of file descriptors to become
		// ready to perform I/O
		// targets: an array of struct pollfd defined in sys/poll.h
		// 1: the number of elements in targets
		// 1000: timeout in miliseconds
		// http://man7.org/linux/man-pages/man2/poll.2.html
		switch ((nready = poll(targets, 1, 1000))) {
			case -1: // Error happened
				// EINTR: Interrupted system call
				// https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html
				// Where errno defined?
				// errno is defined by including errno.h
				// http://man7.org/linux/man-pages/man3/errno.3.html
				if (errno != EINTR) {
					if (errno & EFAULT) {
						printf("The array given as argument was not contained in the "
						 "calling program's address space.");
					} else if (errno & EINVAL) {
						printf("The nfds value exceeds the RLIMIT_NOFILE value");
					} else if (errno & ENOMEM) {
						printf("There was no space to allocate file descriptor tables");
					}
					perror("MyEthThread: Error happened while polling");
				}
				break;
			case 0: // The call timed out
				break;
			default: // Success: meaning the event happened
				// targets[0].revents contains the result of the event
				// Cant'we change if condition here only to read if revents is POLLIN?
				if (targets[0].revents & (POLLIN | POLLERR)) {
					if ((len = read(DeviceSoc, buf, sizeof(buf))) <= 0) {
						// Can we see more detail error here?
						// Yes. Look at http://man7.org/linux/man-pages/man2/read.2.html
						// for what error read could return. 
						// Change here when I see this error often.
						perror("MyEthThread: Failed to read from the device");
					} else {
						EtherRecv(DeviceSoc, buf, len);
					}
				}
				break;
		}
	}

	return NULL;
}

void *StdInThread(void *arg) {
	int nready;
	struct pollfd targets[1];
	char buf[2048];

	// Get the file descriptor number from the standard in
	// http://man7.org/linux/man-pages/man3/fileno.3p.html
	// stdin is defined in stdio.c
	targets[0].fd = fileno(stdin);
	targets[0].events = POLLIN | POLLERR;

	while (EndFlag == 0) {
		switch ((nready = poll(targets, 1, 1000))) {
			case -1:
				if (errno != EINTR) {
					if (errno & EFAULT) {
						printf("The array given as argument was not contained in the "
						 "calling program's address space.");
					} else if (errno & EINVAL) {
						printf("The nfds value exceeds the RLIMIT_NOFILE value");
					} else if (errno & ENOMEM) {
						printf("There was no space to allocate file descriptor tables");
					}
					perror("MyEthThread: Error happened while polling");
				}
			case 0:
				break;
			default:
				if (targets[0].revents & (POLLIN | POLLERR)) {
					// fgets: get a string from a stream
					// This reads one line
					// http://man7.org/linux/man-pages/man3/fgets.3p.html
					fgets(buf, sizeof(buf), stdin);
					DoCmd(buf);

				}
				break;
		}
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	printf("ABC\n");
}
