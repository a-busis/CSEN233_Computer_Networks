all : udps udpc

udps : udp_rdt_server.c
	gcc -o udps udp_rdt_server.c

udpc : udp_rdt_client.c
	gcc -o udpc udp_rdt_client.c

clean :
	rm -f udps udpc
