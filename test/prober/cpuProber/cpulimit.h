//
// Created by antq on 7/27/16.
//

#ifndef PROCTRL_CPULIMIT_H
#define PROCTRL_CPULIMIT_H
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif

void limit_process(pid_t pid, double limit, int include_children);

#ifdef __cplusplus
}
#endif
#endif //PROCTRL_CPULIMIT_H
