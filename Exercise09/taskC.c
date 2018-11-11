#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>

#define SERVER_PRIO 30
#define CLIENT_LOW_PRIO SERVER_PRIO - 10
#define CLIENT_HIGH_PRIO SERVER_PRIO + 10

int set_priority(int priority){
    int policy;
    struct sched_param param;

    if(priority < 1 || priority > 63){
        return -1;
    }

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = priority;
    return pthread_setschedparam(pthread_self(), policy, &param);
}

int get_priority(){
    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    return param.sched_curpriority;
}

int server_init(){
	printf("Init server\r\n");
	int channel = ChannelCreate(_NTO_CHF_FIXED_PRIORITY);// Create channel
	int P_ID = getpid(); // Get process ID

	FILE *fp; // Creating file pointer
	fp = fopen("/root/channel_info.txt", "w+");
	fprintf(fp, "%d %d", channel, P_ID);
	fclose(fp);

	printf("Server has been initialized\n");
	return channel;
}

void client_fn(int *prio){
	int channel, P_ID;
	set_priority(*prio); // Set client priority

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
	int client_prios[] = {CLIENT_LOW_PRIO, CLIENT_LOW_PRIO, CLIENT_HIGH_PRIO, CLIENT_HIGH_PRIO};
	for (i = 0; i < 4; i++) {
		pthread_create(&client_threads[i], NULL, client_fn, &(client_prios[i]));
	}

	char msg [32];
	memset(msg, 0, sizeof(msg)*sizeof(char));
	struct _msg_info info;

	set_priority(SERVER_PRIO); // Set server priority
	while(1){
		printf("\nServer: Listening for messages\n");

		printf("Server: Server priority before MsgReceive(): %d\n", get_priority());
		int Receive_ID = MsgReceive(channel, msg, 7, &info);//listen for messages & replying
		printf("Server: Server priority after MsgReceive(): %d\n", get_priority());

		printf("Server: Message received: %s\n", msg);
		printf("Server: Client info:\n"
				"Thread ID: %d\n"
				"Client connection ID: %d\n"
				"Client priority: %d\n"
				, info.tid, info.coid, info.priority);
		MsgReply(Receive_ID, 0, "Nice One", 32);
	}

	return 0;
}
