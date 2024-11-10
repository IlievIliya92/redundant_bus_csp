/******************************** INCLUDE FILES *******************************/
#include <argp.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/csp_yaml.h>
#include <csp/csp_debug.h>

#include "utils.h"
/******************************** LOCAL DEFINES *******************************/
#define CSP_CONF_FILE_PATH_DFLT "./server.yaml"

#define LOOPBACK_TEST_PORT     10

/******************************* LOCAL TYPEDEFS *******************************/
typedef struct _server_args_t
{
    char *csp_conf_file;
    const char *rtable_file;
} server_args_t;
#define SERVER_DEFAULT_CFG { CSP_CONF_FILE_PATH_DFLT}

/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"csp_conf_file", 'f', "csp-conf-file", 0, "CSP configuration file", 0},
    {"rtable", 'r', "routing-table", 0, "Routing table", 0},
    { 0 }
};
static volatile sig_atomic_t SERVER_STOP;

/*********************** INPUT ARGS PARSE CHECK *************************/
static error_t parse_option( int key, char *arg, struct argp_state *state )
{
     server_args_t *arguments = state->input;

    switch (key) {
        case 'f':
            arguments->csp_conf_file = arg;
            break;
        case 'r':
            arguments->rtable_file = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static struct argp argp = {options, parse_option, NULL, NULL, NULL, NULL, NULL};

/******************************* SIG HANDLERS *********************************/
static
void sig_stop_handler(int sig)
{
    (void)(sig);

    csp_print("Stoping server...\n");
    SERVER_STOP = 1;

    return;
}

/******************************* LOCAL FUNCTIONS ******************************/
static void usage(const char *exec_name)
{
    csp_print("%s -f [csp_configuration_file.yaml] -r [routing_table.txt]\n", ( char *)exec_name);
    csp_print("Run '%s --help' for more information\n", ( char *)exec_name);
}

static void *router_task(void *param)
{
    for(;;)
    {
        csp_route_work();
    }

    return NULL;
}

/********************************** MAIN **************************************/
int main(int argc, char * argv[])
{
    int ret = 0;
    int packet_cnt = 0;
    int dest_port = 0;
    csp_packet_t *i_packet = NULL;
    csp_packet_t *o_packet = NULL;
    csp_conn_t *conn = NULL;
    char rtable[RTABLE_MAX_LEN];
    server_args_t args = SERVER_DEFAULT_CFG;


    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Uncomment to enable debug i/o packets print */
    // csp_dbg_packet_print = 1;
    csp_print("[SERVER] Initialising CSP\n");
    csp_conf.dedup = CSP_DEDUP_ALL;
    csp_init();

    csp_print("[SERVER] Configuring interfaces from: %s\n", args.csp_conf_file)
    csp_yaml_init(args.csp_conf_file, NULL);

    csp_print("[SERVER] Interfaces\r\n");
    csp_iflist_print();

    csp_socket_t sock = {
        .opts = CSP_SO_RDPREQ,
    };
    csp_bind(&sock, CSP_ANY);
    csp_listen(&sock, 10);

    utils_thread_start(router_task);

    signal(SIGINT, sig_stop_handler);
    signal(SIGTERM, sig_stop_handler);

    if (args.rtable_file != NULL)
    {
        csp_print("[SERVE] Loading routing table from: %s\n", args.rtable_file)
        utils_rtable_get(args.rtable_file, rtable);
        ret = csp_rtable_load(rtable);
        if (ret < 1) {
            csp_print("csp_rtable_load(%s) failed, error: %d\n", rtable, ret);
            return EXIT_FAILURE;
        }
        csp_print("Route table\r\n");
        csp_rtable_print();
    }

    SERVER_STOP = 0;
    csp_print("[SERVER] Starting server\n");
    for(;;)
    {
        if (SERVER_STOP)
            break;

        /* Wait for a new connection, 10 mS timeout */
        if ((conn = csp_accept(&sock, 10)) == NULL)
            continue;

        packet_cnt = 0;
        dest_port = csp_conn_dport(conn);
        csp_print("[SERVER] New connection on port: %d\n", dest_port);
        switch (dest_port)
        {
            case LOOPBACK_TEST_PORT:
            for (;;)
            {
                i_packet = csp_read(conn, 1000);
                if (i_packet == NULL)
                    break;

                if (0 == strcmp(i_packet->data, "SERVER_STOP")) {
                    SERVER_STOP = 1;
                    break;
                }

                o_packet = csp_buffer_get(0);
                if (o_packet == NULL) {
                    csp_print("[SERVER] --- Error: Failed to get CSP buffer\n");
                    return EXIT_FAILURE;
                }

                csp_print("[SERVER] New packet %d (on port %d): %s\n", packet_cnt, LOOPBACK_TEST_PORT,
                    (char *) i_packet->data);

                strcpy(o_packet->data, i_packet->data);
                o_packet->length = strlen(i_packet->data) + 1;
                csp_send(conn, o_packet);

                csp_buffer_free(o_packet);
                csp_buffer_free(i_packet);
                packet_cnt++;
            }
            break;

            default:
            break;
        }

        csp_close(conn);
        csp_print("[SERVER] Connection closed\n");
        csp_print("[SERVER] Packets received: %d\n", packet_cnt);
    }

    return 0;
}
