
#include <time.h>


int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec* request, struct timespec* remain);

struct timespec timespec_normalized(time_t sec, long nsec);
struct timespec timespec_sub(struct timespec lhs, struct timespec rhs);
struct timespec timespec_add(struct timespec lhs, struct timespec rhs);
int timespec_cmp(struct timespec lhs, struct timespec rhs);
