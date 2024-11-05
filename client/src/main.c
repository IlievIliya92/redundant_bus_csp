/******************************** INCLUDE FILES *******************************/
#include <argp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/csp_yaml.h>
#include <csp/csp_debug.h>
#include <csp/arch/csp_time.h>

/******************************** LOCAL DEFINES *******************************/
#define CSP_CONF_FILE_PATH_DFLT         "./client.yaml"
#define ROUTING_TABLE_FILE_PATH_DFLT    "./rtable.txt"
#define TEST_MESSAGES_N                 1

#define SERVER_ADDRESS                  10
#define SERVER_PORT                     10
#define RTABLE_MAX_LEN                  1000

/******************************* LOCAL TYPEDEFS *******************************/
typedef struct _client_args_t
{
    char *csp_conf_file;
    const char *routing_table;
    int count;
} client_args_t;
#define CLIENT_DEFAULT_CFG { CSP_CONF_FILE_PATH_DFLT, ROUTING_TABLE_FILE_PATH_DFLT,\
    TEST_MESSAGES_N}

/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"csp_conf_file", 'f', "csp-conf-file", 0, "CSP configuration file", 0},
    {"routing_table", 'r', "routing-table", 0, "Routing table", 0},
    {"count", 'c', "count", 0, "test messages count", 0},
    { 0 }
};

/*********************** INPUT ARGS PARSE CHECK *************************/
static error_t parse_option( int key, char *arg, struct argp_state *state )
{
     client_args_t *arguments = state->input;

    switch (key) {
        case 'f':
            arguments->csp_conf_file = arg;
            break;
        case 'r':
            arguments->routing_table = arg;
            break;
        case 'c':
            arguments->count = atoi(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static struct argp argp = {options, parse_option, NULL, NULL, NULL, NULL, NULL};

/******************************* LOCAL FUNCTIONS ******************************/
static void usage(const char *exec_name)
{
    csp_print("%s -f [csp_configuration_file.yaml] -r [routing_table.txt] -c [test_msgs_cnt]\n",
        ( char *)exec_name);
    csp_print("Run '%s --help' for more information\n", ( char *)exec_name);
}

static int routing_table_get(const char *routing_table_file, char *routing_table,
    int routing_table_len)
{
    int ret = 0;
    FILE* file = NULL;

    file = fopen(routing_table_file, "r");
    if (file == NULL) {
        csp_print("--- Error: Could not open file myfile.txt\n");
        return -1;
    }

    if (fgets(routing_table, routing_table_len, file) != NULL)
    {
        csp_print("Routing table: %s", routing_table);
    }
    else
    {
        csp_print("--- Error: Failed to read routing table from file: %s\n",
            routing_table_file);
        ret = -1;
    }
    fclose(file);

    return ret;
}

static void *router_task(void *param)
{
    while (1)
        csp_route_work();

    return NULL;
}

static int router_start(void)
{
    pthread_attr_t attributes;
    pthread_t handle;
    int ret = CSP_ERR_NONE;

    if (pthread_attr_init(&attributes) != 0)
        return CSP_ERR_NOMEM;

    /* no need to join with thread to free its resources */
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&handle, &attributes, router_task, NULL);
    pthread_attr_destroy(&attributes);

    return ret;
}

/********************************** MAIN **************************************/
int main(int argc, char * argv[])
{
    int i = 0;
    int ret = EXIT_SUCCESS;
    char rtable[RTABLE_MAX_LEN];
    csp_conn_t *conn = NULL;
    csp_packet_t *i_packet = NULL;
    csp_packet_t *o_packet = NULL;

    client_args_t args = CLIENT_DEFAULT_CFG;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    csp_init();
    router_start();

    csp_yaml_init(args.csp_conf_file, NULL);
    csp_print("Interfaces\n");
    csp_iflist_print();

    ret = routing_table_get(args.routing_table, rtable, RTABLE_MAX_LEN);
    if (ret != 0)
        return EXIT_FAILURE;
    csp_rtable_load(rtable);
    csp_print("Routing table\n");
    csp_rtable_print();

    for (i = 0; i < args.count; i ++)
    {
        o_packet = csp_buffer_get(0);
        if (o_packet == NULL) {
            csp_print("--- Error: Failed to get CSP buffer\n");
            ret = EXIT_FAILURE;
            break;
        }

        conn = csp_connect(CSP_PRIO_NORM, SERVER_ADDRESS, SERVER_PORT,
                1000, CSP_O_RDP);
        if (conn == NULL) {
            csp_print("--- Error: Connection failed\n");
            ret = EXIT_FAILURE;
            continue;
        }

        sprintf(o_packet->data, "Test message: %d", csp_get_ms());
        o_packet->length = strlen(o_packet->data) + 1;

        csp_print("--> %d %s\n", i, o_packet->data);
        csp_send(conn, o_packet);
        i_packet = csp_read(conn, 50);
        if (i_packet != NULL)
        {
            csp_print("<-- %d %s\n", i, i_packet->data);
            csp_buffer_free((csp_packet_t *)i_packet);
        }
        else
        {
            csp_print("--- %d Error: Read failed!\n", i);
        }
        csp_buffer_free((csp_packet_t *)o_packet);
        csp_close(conn);
    }

    return ret;
}
