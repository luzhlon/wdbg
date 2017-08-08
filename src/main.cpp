
#include "wdbg.h"

#include <Windows.h>
#include <iostream>
#include <thread>

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
            g_ass->notify("heartbeat", i);
            // printf("Sended heartbeat %lld\n", i);
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

static void print_version() {
    static int major_ver = 0;
    static int minor_ver = 1;
    printf(
        "wdbg v%d.%d, A debugger for windows based Microsoft's dbgeng\n",
        major_ver, minor_ver);
}

static void print_help() {
    printf(
        "Usage: wdbg [-D] [-p PORT] [-v]\n\n"
        "Options:\n"
        "    -D               non daemon mode\n"
        "    -p PORT          the port to listen, default 5100\n"
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
        else if (!strcmp(p, "-p")) {
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
    while (!ser.bind("127.0.0.1", f_port))
        ++f_port;
    ser.listen();
    printf("[PORT]: %d\n", f_port);
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
            g_syms->AppendSymbolPath("cache*; srv*http://msdl.microsoft.com/download/symbols");
            g_syms->AddSymbolOptions(0x100);
        };
        mss.onclose = [](Session& s, bool exp) {
            if (exp) {
                g_client->TerminateProcesses();
                printf("[EXITED EXCEPTED]: Terminated all processed\n");
            } else {
                printf("[EXITED NORMALLY]\n");
            }
        };
        mss.run();
    } while (f_deamon);
    return 0;
}
