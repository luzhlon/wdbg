
#include "wdbg.h"

#include <Windows.h>
#include <iostream>
#include <thread>

#ifdef _WIN64
#define WDBG_ARCH "x64"
#else
#define WDBG_ARCH "x86"
#endif // _WIN64

using namespace std;
using namespace srpc;

// Main session and auxiliary session
Session *g_ss = nullptr;
Session *g_ass = nullptr;
// HeartBeat number
uint64_t last_num = 0;
// Auxiliary thread, used to interrupt the WaitForEvent
void aux_thread() {
    g_ass->addfunc("heartbeat", [](Session& rpc, Tuple& args) {
        auto n = args[0]; assert(n.isint()); last_num = n;
    });
    extern void interrupt(Session& rpc, Tuple& args);
    g_ass->addfunc("interrupt", interrupt);
    // Startup the heartbeat-detect thread
    thread t([](){
        for (uint64_t i = last_num; ; ++i) {
            if (g_ass->isclosed())
                return;
            g_ass->notify("heartbeat", i);
            this_thread::sleep_for(chrono::seconds(1));
            if (last_num != i) {
                printf("[LOSED CONNECTION]\n");
                break;
            }
        }
        // Force the main session exit
        g_ctrl->SetInterrupt(DEBUG_INTERRUPT_ACTIVE);
    }); t.detach();
    // loop
    g_ass->run();
}

// command options
static bool f_deamon = true;
unsigned short f_port = 5100;
const char *f_ipaddr = "0.0.0.0";

static void print_version() {
    static int major_ver = 0;
    static int minor_ver = 1;
    printf(
        "wdbg(%s) v%d.%d, A debugger for windows based Microsoft's dbgeng\n",
        WDBG_ARCH, major_ver, minor_ver);
}

static void print_help() {
    printf(
        "Usage: wdbg [-D] [-p PORT] [-v]\n\n"
        "Options:\n"
        "    -D               non daemon mode\n"
        "    -p PORT          the port to listen, default 5100\n"
        "    -a ADDR          the adapter to listen, default 0.0.0.0\n"
        "    -v               show the version\n"
        "    -h               show help\n"
    );
}
// if value <= 0, exit
static int parse_args(int argc, char **argv) {
    for (size_t i = 1; i < argc; i++) {
        char *p = argv[i];
        if (!strcmp(p, "-h"))
            return print_help(), 0;
        else if (!strcmp(p, "-D"))
            f_deamon = false;
        else if (!strcmp(p, "-v"))
            return print_version(), 0;
        else if (!strcmp(p, "-a")) {
            if (++i >= argc)
                return printf("no adapter specified\n"), -1;
            f_ipaddr = argv[i];
        } else if (!strcmp(p, "-p")) {
            if (++i >= argc)
                return printf("no port specified\n"), -1;
            f_port = atoi(argv[i]);
            if (!f_port)
                return printf("error port\n"), -1;
        } else
            return printf("unknown option: %s\n", p), -1;
    }
    return 1;
}

int main(int argc, char **argv) {
    int code = parse_args(argc, argv);
    if (code <= 0) return code;
    // bind and listen
    tstream::server ser;
    while (!ser.bind(f_ipaddr, f_port))
        ++f_port;
    ser.listen();
    cout << "[PORT] " << f_port << endl;
    cout << "[ARCH] " << WDBG_ARCH << endl;
    // Main session and auxiliary session
    Session mss, ass;
    do {
        // Accept connection
        mss = ser.accept();
        ass = ser.accept();
        g_ss = &mss; g_ass = &ass;
        // Startup the auxiliary thread
        thread t(aux_thread); t.detach();
        mss.onopen = [](Session& s) {
            wdbg::init();
            g_ss->notify("tellinfo", WDBG_ARCH);
        };
        mss.onclose = [](Session& s, bool exp) {
            if (exp) {
                printf("[EXIT] Abnormally\n");
            } else {
                printf("[EXIT] Normally\n");
            }
            g_client->EndSession(DEBUG_END_ACTIVE_TERMINATE);
            g_ass->close();
        };
        mss.run();
    } while (f_deamon);
    return 0;
}
