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
#define TEST_MESSAGES_N_DFLT            10
#define SERVER_STOP_DFLT                0

#define LOOPBACK_TEST_PORT              10
/******************************* LOCAL TYPEDEFS *******************************/
typedef struct _client_args_t
{
    char *csp_conf_file;
    int count;
    int server_stop;
    int *addresses;
    int addresses_n;
} client_args_t;
#define CLIENT_DEFAULT_CFG { CSP_CONF_FILE_PATH_DFLT, TEST_MESSAGES_N_DFLT,\
    SERVER_STOP_DFLT, NULL, 0}

typedef struct _server_test_t
{
    int address;
    int count;
    int server_stop;
} server_test_t;
/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"csp_conf_file", 'f', "csp-conf-file.yaml", 0, "CSP configuration file", 0},
    {"count", 'c', "count", 0, "Test messages count", 0},
    {"server_stop", 's', "server-stop", 0, "Stop server after loopback test", 0},
    {"address", 'a', "adress", 0, "Address of the server", 0},
    { 0 }
};

/*********************** INPUT ARGS PARSE CHECK *************************/
static int parse_address(char *arg, struct argp_state *state) {
    client_args_t *arguments = state->input;
    arguments->addresses = realloc(arguments->addresses, (arguments->addresses_n + 1) * sizeof(int));
    if (arguments->addresses == NULL) {
        csp_print("Realloc failed");
        exit(EXIT_FAILURE);
    }
    arguments->addresses[arguments->addresses_n] = atoi(arg);
    arguments->addresses_n++;

    return 0;
}

static error_t parse_option( int key, char *arg, struct argp_state *state )
{
     client_args_t *arguments = state->input;

    switch (key) {
        case 'f':
            arguments->csp_conf_file = arg;
            break;
        case 'c':
            arguments->count = atoi(arg);
            break;
        case 's':
            arguments->server_stop = atoi(arg);
            break;
        case 'a':
            parse_address(arg, state);
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
    csp_print("[CLIENT] %s -f [csp_configuration_file.yaml] -c [test_msgs_cnt]\n",
        ( char *)exec_name);
    csp_print("[CLIENT] Run '%s --help' for more information\n", ( char *)exec_name);
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

static int server_loopback_test(server_test_t *server_test)
{
    int i = 0;
    int ret = 0;
    csp_conn_t *conn = NULL;
    csp_packet_t *i_packet = NULL;
    csp_packet_t *o_packet = NULL;

    conn = csp_connect(CSP_PRIO_NORM, server_test->address, LOOPBACK_TEST_PORT,
        1000, CSP_O_RDP);
    if (conn == NULL) {
        csp_print("[CLIENT] --- Error: Connection failed (%d)!\n", server_test->address);
        return -1;
    }

    for (i = 0; i < server_test->count; i ++)
    {
        o_packet = csp_buffer_get(0);
        if (o_packet == NULL) {
            csp_print("[CLIENT] --- Error: Failed to get CSP buffer\n");
            ret = -1;
            goto exit;
        }


        sprintf(o_packet->data, "Test message: %d", i);
        o_packet->length = strlen(o_packet->data) + 1;
        csp_print("[CLIENT] --> [SERVER %d]: %s\n", server_test->address, o_packet->data);
        csp_send(conn, o_packet);

        i_packet = csp_read(conn, 1000);
        if (i_packet != NULL)
        {
            csp_print("[CLIENT] <-- [SERVER %d]: %s\n", server_test->address, i_packet->data);
            if (0 != strcmp(i_packet->data, o_packet->data))
                csp_print("[CLIENT] --- %d Error: Data missmatch! %s != %s\n", i,
                     o_packet->data, i_packet->data);
            csp_buffer_free((csp_packet_t *)i_packet);
        }
        else
        {
            csp_print("[CLIENT] --- %d Error: Read failed!\n", i);
        }

        csp_buffer_free((csp_packet_t *)o_packet);
    }

    if (server_test->server_stop)
    {
        o_packet = csp_buffer_get(0);
        if (o_packet == NULL) {
            csp_print("[CLIENT] --- Error: Failed to get CSP buffer\n");
            ret = -1;
            goto exit;
        }

        sprintf(o_packet->data, "SERVER_STOP");
        o_packet->length = strlen(o_packet->data) + 1;
        csp_send(conn, o_packet);
        csp_buffer_free((csp_packet_t *)o_packet);
    }

exit:
    csp_close(conn);

    return ret;
}
/********************************** MAIN **************************************/
int main(int argc, char * argv[])
{
    client_args_t args = CLIENT_DEFAULT_CFG;
    server_test_t server_test;
    int i = 0;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    csp_print("[CLIENT] Initialising CSP\n");
    csp_conf.dedup = CSP_DEDUP_ALL;
    csp_init();
    router_start();

    csp_yaml_init(args.csp_conf_file, NULL);
    csp_print("[CLIENT] Interfaces\n");
    csp_iflist_print();

    server_test.count = args.count;
    server_test.server_stop = args.server_stop;
    for(i = 0; i < args.addresses_n; i ++) {
        server_test.address = args.addresses[i];
        csp_print("Loopback test to server on address: %d\n", server_test.address);
        server_loopback_test(&server_test);
    }

    return 0;
}
