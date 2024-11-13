/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <pthread.h>

#include "utils.h"
/******************************** LOCAL DEFINES *******************************/

/**************************** INTERFACE FUNCTIONS *****************************/
int utils_thread_start(task_hanlder_t task_handler, void *args)
{
    pthread_attr_t attributes;
    pthread_t handle;
    int ret = 0;

    if (pthread_attr_init(&attributes) != 0)
        return -1;

    /* no need to join with thread to free its resources */
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&handle, &attributes, task_handler, args);
    pthread_attr_destroy(&attributes);

    return ret;
}

int utils_rtable_get(const char *rtable_file, char *rtable)
{
    int ret = 0;
    FILE* file = NULL;

    file = fopen(rtable_file, "r");
    if (file == NULL) {
        return -1;
    }

    if (fgets(rtable, RTABLE_MAX_LEN, file) == NULL)
        ret = -1;

    fclose(file);

    return ret;
}
