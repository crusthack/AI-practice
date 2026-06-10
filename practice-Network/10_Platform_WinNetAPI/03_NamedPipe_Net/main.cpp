#include <windows.h>
#include <cstdio>
static const wchar_t* PIPE = L"\\\\.\\pipe\\practice_network_pipe";
int server() {
    HANDLE p=CreateNamedPipeW(PIPE, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT, 1, 4096, 4096, 0, nullptr);
    std::puts("named pipe server waiting"); ConnectNamedPipe(p, nullptr);
    char b[256]{}; DWORD got=0,wrote=0; ReadFile(p,b,sizeof(b)-1,&got,nullptr); std::printf("read: %s\n",b); WriteFile(p,b,got,&wrote,nullptr); CloseHandle(p); return 0;
}
int client() {
    HANDLE p=CreateFileW(PIPE, GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    const char* msg="hello named pipe"; DWORD n=0; WriteFile(p,msg,(DWORD)strlen(msg),&n,nullptr); char b[256]{}; ReadFile(p,b,sizeof(b)-1,&n,nullptr); std::printf("echo: %s\n",b); CloseHandle(p); return 0;
}
int main(int argc,char**argv){ return argc>1 && strcmp(argv[1],"client")==0 ? client() : server(); }
