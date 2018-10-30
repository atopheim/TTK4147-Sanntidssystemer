#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>

#include <semaphore.h>
#include "udp.h"
#include "time.h"

float Kp = 5;
float Ki = 800;
float Kd = 0;

float reference = 1;

// Shared variables between read_network and PID controller
float y = 0;

UDPConn* conn;

pthread_mutex_t mutex_y = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_y;
sem_t sem_signal;

// Decodes message on the form "GET_ACK:<DECIMAL NUMBER>" and returns 1 when "SIGNAL" is detected
int decode_msg(char *msg, float *y) {
	if (msg[0] == 'S') { // "SIGNAL"
		//printf("msg: %s\r\n", msg);
		return 1;
	} else if (msg[0] == 'G') {
		*y = atof(msg + 8);
		return 0;
	}
	return -1; // Error
}

// Encodes msg with val on the form "SET:123.456"
void encode_msg(char *msg, float val) {
	sprintf(msg, "SET:%f", val);
}

void pid_controller(void *args) {
	float error, integral, derivative, prev_error, u = 0;
	
	float dt = 0.003;
	double dt_nanosec = dt * 1000*1000*1000;

  struct timespec prev, target;
	struct timespec dt_timespec = {0, (long) dt_nanosec};

	clock_gettime(CLOCK_MONOTONIC, &prev);
	char msg[32];
	// Main controller loop
	while (1) {
		
		// Get y from server ("GET" request)
		udpconn_send(conn, "GET");
		
		// Wait for POST from network read thread
		sem_wait(&sem_y);
		
		// Do PID calculations with new y value
		pthread_mutex_lock(&mutex_y);
			error = reference - y;
		pthread_mutex_unlock(&mutex_y);

		integral += error * dt;
		derivative = (error - prev_error) / dt;
		prev_error = error;

		u = Kp*error + Kd*derivative + Ki*integral;
		
		// Send u to server
		encode_msg(msg, u);
		udpconn_send(conn, msg);
		
		// Find time equal to one period from previous iteration
		target = timespec_add( prev, dt_timespec );
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, NULL);
		clock_gettime(CLOCK_MONOTONIC, &prev);
	}
}

void signal_ACK(void *args) {

	while (1) {
		sem_wait(&sem_signal);
		udpconn_send(conn, "SIGNAL_ACK");
	}
}

void network_read(void *args) {
  char recvBuf[32];
  memset(recvBuf, 0, sizeof(recvBuf));
	
	int res;

	while (1) {
		// Read from network  
		udpconn_receive(conn, recvBuf, sizeof(recvBuf));
		
		pthread_mutex_lock(&mutex_y);
			res = decode_msg(recvBuf, &y);
		pthread_mutex_unlock(&mutex_y);

		if (res == 1) {	//SIGNAL		
			sem_post(&sem_signal);
		} else if (res == 0) { // New y value
			//printf("Posting semaphore in UDP reader\r\n");
			sem_post(&sem_y);
		}
	}
}

int main()
{
	// Open udp connection
	conn = udpconn_new("10.100.23.180", 9999);
	pthread_t pid_controller_thread, signal_ACK_thread, network_read_thread;

	sem_init(&sem_y, 0, 0);
	sem_init(&sem_signal, 0, 0);

	pthread_create( &pid_controller_thread, NULL, pid_controller, NULL);	
	pthread_create( &signal_ACK_thread, NULL, signal_ACK, NULL);	
	pthread_create( &network_read_thread, NULL, network_read, NULL);	
	
	// Start simulation
	char sendBuf[32];
  sprintf(sendBuf, "START"); 
  udpconn_send(conn, sendBuf);
	reference = 1.0;

	// Sleep 1 second before changing reference value
	struct timespec second_timespec = {1, 0};
	clock_nanosleep(CLOCK_MONOTONIC, 0, &second_timespec, NULL);
	reference = 0.0;

	// Sleep 1 more second before ending simulation
	clock_nanosleep(CLOCK_MONOTONIC, 0, &second_timespec, NULL);

	// Stop simulation
	sprintf(sendBuf, "STOP");   
  udpconn_send(conn, sendBuf);

	// Clean-up
	udpconn_delete(conn);
	
	//pthread cancel & join
	pthread_cancel(&pid_controller_thread);
	pthread_cancel(&signal_ACK_thread);
	pthread_cancel(&network_read_thread);
	pthread_join(&pid_controller_thread, NULL);
	pthread_join(&signal_ACK_thread, NULL);
	pthread_join(&network_read_thread, NULL);

	return 0;
}

