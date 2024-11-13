/******************************** INCLUDE FILES *******************************/
// #include <stdlib.h>
// #include <pthread.h>
#include <fstream>
#include <csignal>
#include <thread>
#include <iostream>
#include <boost/program_options.hpp>

#include <csp/csp.h>
#include <csp/csp_yaml.h>
#include <csp/csp_debug.h>

#include "thread_pool.hpp"

using namespace std;
namespace prog_opts = boost::program_options;
/******************************** LOCAL DEFINES *******************************/
#define LOOPBACK_TEST_PORT     10
#define LOOPBACK_TEST_PORT_1   11

/******************************* LOCAL TYPEDEFS *******************************/

/********************************* LOCAL DATA *********************************/
static volatile bool SERVER_STOP;

/******************************* SIG HANDLERS *********************************/
void sig_stop_handler(int signum)
{
    // Perform cleanup here if needed
    SERVER_STOP = true;
}
/******************************* LOCAL FUNCTIONS ******************************/
void *router_task_handle(void)
{
    for(;;) {
        if (SERVER_STOP)
            break;

        csp_route_work();
    }

    return NULL;
}

int rtable_get(string &rtable_file_path, string &rtable)
{
    const char *rtable_file_path_cstr = rtable_file_path.c_str();
    csp_print("[SERVER] Loading routing table from: %s\n", rtable_file_path_cstr);

    ifstream file(rtable_file_path_cstr);
    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        return 1;
    }

    ostringstream sstream;
    sstream << file.rdbuf();
    rtable = sstream.str();

    return 0;
}

int conn_handler(csp_conn_t *conn)
{
    csp_packet_t *i_packet = NULL;
    csp_packet_t *o_packet = NULL;
    int packet_cnt = 0;
    int dest_port = 0;

    dest_port = csp_conn_dport(conn);
    csp_print("[SERVER] New connection on port: %d\n", dest_port);
    switch (dest_port)
    {
        case LOOPBACK_TEST_PORT:
        case LOOPBACK_TEST_PORT_1:
        for (;;)
        {
            i_packet = csp_read(conn, 1000);
            if (i_packet == NULL)
                break;

            if (0 == strcmp(reinterpret_cast<const char *>(i_packet->data) , "SERVER_STOP")) {
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

            strcpy(reinterpret_cast<char *>(o_packet->data),
                    reinterpret_cast<const char *>(i_packet->data));
            o_packet->length = strlen(reinterpret_cast<const char *>(i_packet->data)) + 1;
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

    return 0;
}
/********************************** MAIN **************************************/
int main(int argc, char * argv[])
{
    string csp_conf_file_path = "module_0_virt.yaml";
    string rtable_file_path;
    bool verbose = false;
    string rtable;

    int ret = 0;

    prog_opts::variables_map vm;
    prog_opts::options_description desc("Options");
    desc.add_options()
    ("help,h", "produce help message")
    ("csp_conf_file,f", prog_opts::value(&csp_conf_file_path), "CSP configuration file")
    ("rtable,r", prog_opts::value(&rtable_file_path), "Routing table")
    ("verbose,v", prog_opts::bool_switch(&verbose), "Enable verbose");

    prog_opts::store(prog_opts::parse_command_line(argc, argv, desc), vm);
    prog_opts::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    if (verbose) {
        csp_dbg_packet_print = 1;
    }

    csp_print("[SERVER] Initialising CSP\n");
    csp_conf.dedup = CSP_DEDUP_ALL;
    csp_init();

    char *csp_conf_file_path_cstr = new char[csp_conf_file_path.length() + 1];
    std::strcpy(csp_conf_file_path_cstr, csp_conf_file_path.c_str());
    csp_print("[SERVER] Configuring interfaces from: %s\n", csp_conf_file_path_cstr)
    csp_yaml_init(csp_conf_file_path_cstr, NULL);
    delete[] csp_conf_file_path_cstr;

    csp_print("[SERVER] Interfaces\r\n");
    csp_iflist_print();

    if (!rtable_file_path.empty())
    {
        rtable_get(rtable_file_path, rtable);
        const char *rtable_cstr = rtable.c_str();
        ret = csp_rtable_load(rtable_cstr);
        if (ret < 1) {
            csp_print("csp_rtable_load failed, error: %d\n", ret);
            return EXIT_FAILURE;
        }
        csp_print("Route table\n");
        csp_rtable_print();
    }
    std::thread router_task(router_task_handle);
    ThreadPool workers_pool(5);

    signal(SIGINT, sig_stop_handler);
    signal(SIGTERM, sig_stop_handler);

    csp_socket_t sock = {
        .opts = CSP_SO_RDPREQ,
    };
    csp_bind(&sock, CSP_ANY);
    csp_listen(&sock, 10);

    SERVER_STOP = 0;
    csp_print("[SERVER] Starting server\n");
    for(;;)
    {
        if (SERVER_STOP)
            break;

        /* Wait for a new connection, 10 mS timeout */
        csp_conn_t *conn = csp_accept(&sock, 10);
        if (conn == NULL)
            continue;

        workers_pool.enqueue(&conn_handler, conn);
    }

    router_task.join();

    return EXIT_SUCCESS;
}
