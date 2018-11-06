#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include <semaphore.h>

#include "udp.h"
#include "time.h"

float reference = 1;
float y = 0; // Shared data between read_network and PID controller

UDPConn* conn; // Network connection

/* 
 * Decodes messages on the form "GET_ACK:<FLOATING POINT NUMBER>" or "SIGNAL"
 * returns 1 when message is "SIGNAL", 0 when floating point number is decoded,
 * and -1 otherwise
 */
int decode_msg(char *msg, float *y) {
	if (msg[0] == 'S') { // "SIGNAL"
		return -1;
	}
	*y = atof(msg + 8);
	return 0;
}

// Encodes msg with floating point number on the form "SET:<FLOATING POINT NUMBER>"
void encode_msg(char *msg, float val) {
	sprintf(msg, "SET:%f", val);
}

// Thread function for PI controller
void* pi_controller() {
	float error, integral, prev_error, u = 0;
	char msg[32];	
  char recvBuf[32];
  memset(recvBuf, 0, sizeof(recvBuf));
	
	// PI controller parameters
	float Kp = 5;
	float Ki = 800;

	float dt = 0.002;
	float dt_nanosec = dt * 1000*1000*1000;

  struct timespec prev, waketime;
	struct timespec dt_timespec = {0, (long) dt_nanosec};	
	clock_gettime(CLOCK_MONOTONIC, &prev);

	// Controller loop
	while (1) {
		// Get y from server
		udpconn_send(conn, "GET");
		
		do {
			udpconn_receive(conn, recvBuf, sizeof(recvBuf));
		}
		while (decode_msg(recvBuf, &y) < 0);
		
		// Do PI calculations with new y value
		error = reference - y; // optimistic concurrency control on "reference"
	
		integral += error * dt;
		prev_error = error;
		u = Kp*error + Ki*integral;
		
		// Send u to server
		encode_msg(msg, u);
		udpconn_send(conn, msg);
		
		// Sleep until one period from previous iteration
		waketime = timespec_add(prev, dt_timespec);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waketime, NULL);
		clock_gettime(CLOCK_MONOTONIC, &prev);
	}
}


// Spawns threads and controls program timing
int main()
{
	// Open udp connection
	conn = udpconn_new("10.100.23.180", 9999);

	pthread_t pi_controller_thread;
	pthread_create( &pi_controller_thread, NULL, pi_controller, NULL);	
	
	// Start simulation
	reference = 1.0;
  udpconn_send(conn, "START");

	// Sleep 1 second before changing reference value
	struct timespec second_timespec = {1, 0};
	clock_nanosleep(CLOCK_MONOTONIC, 0, &second_timespec, NULL);
	reference = 0.0; // optimistic concurrency control

	// Sleep 1 more second before ending simulation
	clock_nanosleep(CLOCK_MONOTONIC, 0, &second_timespec, NULL);

	// Stop simulation
  udpconn_send(conn, "STOP");

	// Clean-up
	udpconn_delete(conn);
	
	pthread_cancel(pi_controller_thread);
	pthread_join(pi_controller_thread, NULL);

	return 0;
}


