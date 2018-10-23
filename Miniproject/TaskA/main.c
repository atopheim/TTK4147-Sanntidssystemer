#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>

#include "udp.h"
#include "time.h"

double Kp = 1;
double Ki = 10;
double Kd = 0;


// Decodes message on the form "GET_ACK:<DECIMAL NUMBER>" and returns the decimal number
int decode_msg(char *msg, double *y) {
	if (msg[0] == 'S') { // "SIGNAL"
		printf("msg: %s\r\n", msg);
		return -1;	
	}

	*y = atof(msg + 8);
	return 0;
}

// Encodes msg with val on the form "SET:123.456"
void encode_msg(char *msg, double val) {
	sprintf(msg, "SET:%f", val);
}

void pid_controller(void *args) {
	double error, integral, derivative, prev_error, y, u = 0;
	double reference = 1;
	double dt = 0.004;

	long dt_nanosec = dt * 1000*1000*1000;

  struct timespec start_of_simulation, start, now, diff;
	struct timespec dt_timespec = {0, dt_nanosec};

  UDPConn* conn = udpconn_new("10.100.23.180", 9999);
    
  char sendBuf[32];
  char recvBuf[32];    
  memset(recvBuf, 0, sizeof(recvBuf));
  
	// Start simulation
	printf("Starting simulation\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  sprintf(sendBuf, "START");    
  udpconn_send(conn, sendBuf);

	clock_gettime(CLOCK_MONOTONIC, &start_of_simulation);
  
	// Main controller loop
	while (1) {
		clock_gettime(CLOCK_MONOTONIC, &start);

		// Check amount of time that has passed since simulation start
		diff = timespec_sub(start, start_of_simulation);
		if (diff.tv_sec >= 2) {	// Two seconds have passed
				// Stop simulation
				sprintf(sendBuf, "STOP");    
				udpconn_send(conn, sendBuf);
				printf("Ended simulation\r\n");
				break;
		} 
		else if (diff.tv_sec >= 1) { // One second has passed
			reference = 0;
		}
		
		// Get y from server
		do {
			sprintf(sendBuf, "GET");    
			udpconn_send(conn, sendBuf);
	
			udpconn_receive(conn, recvBuf, sizeof(recvBuf));
		} while (decode_msg(recvBuf, &y) < 0);
		
		// PID calculations
		error = reference - y;
		integral += error * dt;
		derivative = (error - prev_error) / dt;
		prev_error = error;

		u = Kp*error + Kd*derivative + Ki*integral;
		
		printf("NEW ITERATION \r\n\r\n");
		printf("reference: %f\r\n", reference);
		printf("y: %f\r\n", y);
		printf("u: %f\r\n", u);
		printf("error: %f\r\n", error);
		

		// Find time spent in current iteration
		// If dt time has not passed, sleep for the remaining time
 		clock_gettime(CLOCK_MONOTONIC, &now);
		diff = timespec_sub(now, start);
		
		if (diff.tv_nsec < dt_nanosec) {
			const struct timespec request = timespec_sub(dt_timespec, diff);
			clock_nanosleep(CLOCK_MONOTONIC, 0, &request, NULL);
		}

		// Send u to server
		encode_msg(sendBuf, u);
		udpconn_send(conn, sendBuf);		
		
	}

	// Clean-up
	udpconn_delete(conn);
}


int main()
{
	pthread_t pid_controller_thread;
	pthread_create( &pid_controller_thread, NULL, pid_controller, NULL);	

	// Wait for simulation to finish
	pthread_join(pid_controller_thread, NULL);

	return 0;
}

