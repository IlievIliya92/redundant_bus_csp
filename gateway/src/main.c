/******************************** INCLUDE FILES *******************************/
#include <argp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <csp/csp.h>
#include <csp/csp_yaml.h>
#include <csp/csp_debug.h>
#include <csp/arch/csp_time.h>

#include "utils.h"
/******************************** LOCAL DEFINES *******************************/
#define CSP_CONF_FILE_PATH_DFLT         "./gateway.yaml"
#define CSP_MODE_DFLT                   "router"

/******************************* LOCAL TYPEDEFS *******************************/
typedef struct _client_args_t
{
    char *csp_conf_file;
    char *mode;
} client_args_t;
#define GW_DEFAULT_CFG { CSP_CONF_FILE_PATH_DFLT, CSP_MODE_DFLT }

/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"csp_conf_file", 'f', "csp-conf-file.yaml", 0, "CSP configuration file", 0},
    {"mode", 'm', "mode", 0, "Operation mode: [router | bridge]", 0},
    { 0 }
};

/***************************** INPUT ARGS PARSE CHECK *************************/
static error_t parse_option( int key, char *arg, struct argp_state *state )
{
     client_args_t *arguments = state->input;

    switch (key) {
        case 'f':
            arguments->csp_conf_file = arg;
            break;
        case 'm':
            arguments->mode = arg;
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
    csp_print("[GATEWAY] %s -f [csp_configuration_file.yaml] -m [router | bridge]\n",
        ( char *)exec_name);
    csp_print("[GATEWAY] Run '%s --help' for more information\n", ( char *)exec_name);
}

static void *router_task(void *param)
{
    for(;;)
    {
        csp_route_work();
    }

    return NULL;
}

static void *bridge_task(void *param)
{
    for(;;)
    {
        csp_bridge_work();
    }

    return NULL;
}

/********************************** MAIN **************************************/
int main(int argc, char * argv[])
{
    client_args_t args = GW_DEFAULT_CFG;
    int i = 0;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Uncomment to enable debug i/o packets print */
    //csp_dbg_packet_print = 1;
    csp_print("[GATEWAY] Initialising CSP\n");
    csp_init();

    csp_yaml_init(args.csp_conf_file, NULL);
    csp_print("[GATEWAY] Interfaces\n");
    csp_iflist_print();

    csp_print("[GATEWAY] %s mode\n", args.mode);
    if (0 == strcmp(args.mode, "router"))
    {
        utils_thread_start(router_task);
    }
    else if (0 == strcmp(args.mode, "bridge"))
    {
        csp_iface_t *gw = csp_iflist_get_by_index(1);
        csp_iface_t *can0 = csp_iflist_get_by_index(2);
        csp_iface_t *can1 = csp_iflist_get_by_index(3);
        csp_print("[GATEWAY] Bridging %s to %s\n", gw->name, can0->name);
        csp_bridge_set_interfaces(gw, can0);
        csp_print("[GATEWAY] Bridging %s to %s\n", gw->name, can1->name);
        csp_bridge_set_interfaces(gw, can1);
        utils_thread_start(bridge_task);
    }
    else
    {
        csp_print("[GATEWAY] --- Error: Unknown mode of operation!");
        return -1;
    }

    for (;;)
    {
        sleep(1);
    }

    return 0;
}
