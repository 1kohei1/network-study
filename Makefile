PROGRAM=MyEth
OBJS=main.o param.o sock.o ether.o arp.o ip.o icmp.o cmd.o
SRCS=$(OBJS:%.o=%.c)
CFLAGS=-Wall -g -D u_short="unsigned short"
LDFLAGS=-lpthread
$(PROGRAM):$(OBJS) $(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LDLIBS)

