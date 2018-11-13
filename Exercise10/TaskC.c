#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

dispatch_t              *dpp;
resmgr_attr_t           resmgr_attr;
dispatch_context_t      *ctp;
resmgr_connect_funcs_t  connect_funcs;
resmgr_io_funcs_t       io_funcs;
iofunc_attr_t           io_attr;

int io_read(resmgr_context_t* ctp, io_read_t* msg, iofunc_ocb_t* ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);

char buf[128];
char res[32];

enum counter {
	UP,
	DOWN,
	STOP,
};

enum counter mode_g = UP;
int count_g = 0;

pthread_t counter_thread;
pthread_mutex_t mode_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_mtx = PTHREAD_MUTEX_INITIALIZER;

void counter_fn() {
	int count = 0;
	enum counter mode = UP;

	while (1) {
		pthread_mutex_lock(&mode_mtx);
			mode = mode_g;
		pthread_mutex_unlock(&mode_mtx);

		switch (mode) {
		case UP:
			count++;
			break;
		case DOWN:
			count--;
			break;
		case STOP:
			break;
		default:
			printf("Not a valid mode\r\n");
			break;
		}

		pthread_mutex_lock(&count_mtx);
			count_g = count;
		pthread_mutex_unlock(&count_mtx);

		// sleep 100 ms
		delay(100);
	}
}

void error(char* s){
	perror(s);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	printf("Start resource manager\n");

	// create dispatch.
	if (!(dpp = dispatch_create())){
		error("Create dispatch");
	}

	// initialize resource manager attributes.
	memset(&resmgr_attr, 0, sizeof(resmgr_attr));
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;

	// set standard connect and io functions.
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	iofunc_attr_init(&io_attr, S_IFNAM | 0666, 0, 0);

	// override functions
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	// establish resource manager
	if (resmgr_attach(dpp, &resmgr_attr, "/dev/myresource", _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &io_attr) < 0)
		error("Resmanager attach");
	ctp = dispatch_context_alloc(dpp);

	// create counter thread
	pthread_create(&counter_thread, NULL, counter_fn, NULL);

	// wait forever, handling messages.
	while(1){
		if (!(ctp = dispatch_block(ctp))){
			error("Dispatch block");
		}
		dispatch_handler(ctp);
	}

	exit(EXIT_SUCCESS);
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb){

	//char buf[] = "Hello World\n";

	if(!ocb->offset){
		// set data to return
		pthread_mutex_lock(&count_mtx);
			sprintf(res, "Count: %d\n", count_g);
		pthread_mutex_unlock(&count_mtx);
		printf("%s\n", res);

		SETIOV(ctp->iov, res, strlen(res));
		_IO_SET_READ_NBYTES(ctp, strlen(res));

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;

		// return
		return (_RESMGR_NPARTS(1));
	} else {
		// set to return no data
		_IO_SET_READ_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	/* set up the number of bytes (returned by client's write()) */

	_IO_SET_WRITE_NBYTES (ctp, msg->i.nbytes);

    resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i));
    buf [msg->i.nbytes] = '\0'; /* just in case the text is not NULL terminated */
    printf ("Received %d bytes = '%s'\n", msg -> i.nbytes, buf);

	// check input
    int mode = UP; // default
	if (buf[0] == 'u') {
		mode = UP;
	} else if (buf[0] == 'd') {
		mode = DOWN;
	} else if (buf[0] == 's') {
		mode = STOP;
	}
	pthread_mutex_lock(&mode_mtx);
		mode_g = mode;
	pthread_mutex_unlock(&mode_mtx);

    if (msg->i.nbytes > 0)
         ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS (0));
}
