all:	selectserver client UdsClient
selectserver: selectserver.c
	gcc -o selectserver selectserver.c
client: client.c
	gcc -o client client.c
UdsClient: UdsClient.c
	gcc -o UdsClient UdsClient.c
clean:
	rm -f selectserver client UdsClient
