/******************************** INCLUDE FILES *******************************/

/*********************************** DEFINES **********************************/
#define RTABLE_MAX_LEN 1000

/********************************* TYPEDEFS ***********************************/
typedef void * (*task_hanlder_t) (void *);

/**************************** INTERFACE FUNCTIONS *****************************/
int utils_thread_start(task_hanlder_t task_handler);
int utils_rtable_get(const char *rtable_file, char *rtable);

