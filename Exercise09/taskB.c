#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>

int server_init(){
	printf("Init server\r\n");
	int channel = ChannelCreate(0);// Create channel
	int P_ID = getpid(); // Get process ID

	FILE *fp; // Creating file pointer
	fp = fopen("/root/channel_info.txt", "w+");
	fprintf(fp, "%d %d", channel, P_ID);
	fclose(fp);

	printf("Server has been initialized\n");
	return channel;
}


void client_fn(){
	int channel, P_ID;

	FILE *fp;
	fp = fopen("/root/channel_info.txt", "r+");
	fscanf(fp, "%d %d", &channel, &P_ID);
	fclose(fp);

	int conn = ConnectAttach(0, P_ID, channel, 0,0);
	printf("\nClient: Attached to channel. Conn: %d\n", conn);

	char replyBuf [32];
	char msg[32];
	sprintf(msg, "MESSAGE FROM CLIENT");

	printf("Client: Sending message\n");
	MsgSend(conn, msg, sizeof(msg), replyBuf, sizeof(replyBuf));

	printf("Client: Reply from server: %s\n", replyBuf);
	ConnectDetach(conn);
}

int main(int argc, char *argv[]) {
	int channel = server_init();

	pthread_t client_threads[4];
	int i;
	for (i = 0; i < 4; i++) {
		pthread_create(&client_threads[i], NULL, client_fn, NULL);
	}

	char msg [32];
	memset(msg, 0, sizeof(msg)*sizeof(char));
	struct _msg_info info;
	while(1){
		printf("\nServer: Listening for messages\n");
		int Receive_ID = MsgReceive(channel, msg, 7, &info);//listen for messages & replying
		printf("Server: Message received: %s\n", msg);
		printf("Server: Client info:\n"
				"Thread ID: %d\n"
				"Client connection ID: %d\n", info.tid, info.coid);
		MsgReply(Receive_ID, 0, "Nice One", 32);
	}

	return 0;
}
