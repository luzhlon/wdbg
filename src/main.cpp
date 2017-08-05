
#include "wdbg.h"
#include "handler.h"

#include <Windows.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace srpc;

// Main session and auxiliary session
Session *g_ss = nullptr, *g_ass = nullptr;
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

static bool f_deamon = true;

static void parse_args(int argc, char **argv) {
    // ...
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    // bind and listen
    tstream::server ser;
    unsigned short port = 5100;
    for (; !ser.bind("127.0.0.1", port); ++port);
    ser.listen();
    printf("[PORT]: %d\n", port);
    // Main session and auxiliary session
    Session mss, ass;
    do {
        // Accept connection
        mss = ser.accept();
        ass = ser.accept();
        g_ss = &mss; g_ass = &ass;
        // Startup the auxiliary thread
        thread t(aux_thread); t.detach();
        mss.onopen = [](Session& s) { wdbg::init(); };
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
