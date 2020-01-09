all: client serve

client:
	gcc -o DUMBclient dumbclient.c -lpthread

serve:
	gcc -o DUMBserve dumbserver.c -lpthread


clean:
	rm DUMBclient DUMBserve
