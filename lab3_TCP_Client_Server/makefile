all : tcps tcpc

tcps : tcp_server.c
	gcc -o tcps tcp_server.c

tcpc : tcp_client.c
	gcc -o tcpc tcp_client.c

clean :
	rm -f tcps tcpc
