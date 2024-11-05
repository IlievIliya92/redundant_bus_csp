/******************************** INCLUDE FILES *******************************/
#include <argp.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/csp_yaml.h>
#include <csp/csp_debug.h>
/******************************** LOCAL DEFINES *******************************/
#define SERVER_PORT     10
#define CSP_CONF_FILE_PATH_DFLT "./server_0.yaml"

/******************************* LOCAL TYPEDEFS *******************************/
typedef struct _server_args_t
{
    char *csp_conf_file;
} server_args_t;
#define SERVER_DEFAULT_CFG { CSP_CONF_FILE_PATH_DFLT}

/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"csp_conf_file", 'f', "csp-conf-file", 0, "CSP configuration file", 0},
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
    csp_print("%s -f [csp_configuration_file.yaml]\n", ( char *)exec_name);
    csp_print("Run '%s --help' for more information\n", ( char *)exec_name);
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
    csp_packet_t *packet = NULL;
    csp_conn_t *conn = NULL;
    server_args_t args = SERVER_DEFAULT_CFG;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    csp_print("Initialising CSP\n");
    csp_conf.dedup = CSP_DEDUP_INCOMING;
    csp_init();
    router_start();

    csp_yaml_init(args.csp_conf_file, NULL);

    csp_print("Interfaces\r\n");
    csp_iflist_print();

    csp_socket_t sock = {
        .opts = CSP_SO_RDPREQ,
    };

    csp_bind(&sock, CSP_ANY);
    csp_listen(&sock, 10);

    signal(SIGINT, sig_stop_handler);
    signal(SIGTERM, sig_stop_handler);

    SERVER_STOP = 0;
    csp_print("Starting server\n");
    for(;;)
    {
        if (SERVER_STOP)
            break;

        /* Wait for a new connection, 10000 mS timeout */
        if ((conn = csp_accept(&sock, 10000)) == NULL)
            continue;

        /* Read packets on connection, timout is 100 mS */
        while ((packet = csp_read(conn, 50)) != NULL)
        {
            switch (csp_conn_dport(conn))
            {
            case SERVER_PORT:
                /* Process packet here */
                csp_print("New packet (on port %d): %s\n", SERVER_PORT, (char *) packet->data);
                csp_send(conn, packet);
                csp_buffer_free(packet);
                break;

            default:
                csp_service_handler(packet);
                break;
            }
        }

        /* Close current connection */
        csp_close(conn);
    }

    return 0;
}
