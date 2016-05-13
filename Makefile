CFLAGS=-Wall -g

all:
	gcc -o edge_fw_update edge_fw_update.c -ludev 

clean:
	rm -f edge_fw_update
