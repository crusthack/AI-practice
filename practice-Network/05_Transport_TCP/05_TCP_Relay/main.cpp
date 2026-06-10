#include "NetUtil.hpp"
#include <thread>
static constexpr unsigned short LISTEN_PORT=27041, BACKEND_PORT=27015;
void pump(SOCKET from, SOCKET to, const char* tag) { char b[4096]; for(;;){ int n=recv(from,b,sizeof(b),0); if(n<=0) break; std::printf("%s %d bytes\n",tag,n); if(net::send_all(to,b,n)==SOCKET_ERROR) break; } shutdown(to,SD_SEND); }
void relay(SOCKET raw) {
    net::Socket client(raw); auto backend=net::tcp_socket(); if(!net::connect_loopback(backend.value,BACKEND_PORT)) return;
    std::thread a(pump, client.value, backend.value, "client->backend"); std::thread b(pump, backend.value, client.value, "backend->client"); a.join(); b.join();
}
int main(){ net::WsaSession wsa; auto ls=net::tcp_socket(); net::bind_listen(ls.value,LISTEN_PORT); std::puts("relay 27041 -> 27015"); while(true){ SOCKET c=accept(ls.value,nullptr,nullptr); if(c!=INVALID_SOCKET) std::thread(relay,c).detach(); } }
