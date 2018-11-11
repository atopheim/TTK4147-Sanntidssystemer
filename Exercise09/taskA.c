#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>

int server_init(){
	printf("Init server\r\n");
	int channel = ChannelCreate(0);// Create channel
	printf("Channel: %d\n", channel);

	FILE *fp; // Creating file pointer
	fp = fopen("/root/channel_info.txt", "w+");
	fprintf(fp, "%d", channel);
	fclose(fp);

	printf("Server has been initialized\n");
	return channel;

}
void client_fn(){
	int P_ID = getpid(); // Get process ID of channel
	int channel;
	FILE *fp;
	fp = fopen("/root/channel_info.txt", "r+");
	fscanf(fp, "%d", &channel);
	fclose(fp);
	printf("Channel: %d\n", channel);

	int conn = ConnectAttach(0, P_ID, channel, 0,0);
	printf("Conn: %d\n", conn);
	char RecvBuff [32];
	char SendBuff [32];
	sprintf(&SendBuff, "TEST MESSAGE");
	printf("Sending message\n");
	int sendID = MsgSend(conn, "TEST", 4, RecvBuff, 32);
	printf("SendID: %d\n", sendID);
	printf("%s\n", RecvBuff);
	ConnectDetach(conn);
}
int main(int argc, char *argv[]) {
	pthread_t client_thread;
	int channel = server_init();
	pthread_create(&client_thread, NULL, client_fn, NULL);
	char msg [32];
	while(1){
		printf("Listening for messages\n");
		int Receive_ID = MsgReceive(channel, msg, 32, NULL);//listen for messages & replying
		printf("Message received: %s\n", msg);
		MsgReply(Receive_ID, 0, "Nice One", 32);
	}

	return 0;
}
