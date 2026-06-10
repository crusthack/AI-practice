// 03_Layered_HTTP: HTTP/1.1  "o "/    -  S   T" "   ~"   
//  <  -?: 03_Layered_HTTP.exe server  /  03_Layered_HTTP.exe client
//
//  "o "   s  S :
//   GET  /                   ?"   S   .^ , 
//   GET  /echo               ?" query: msg=...  -  "
//   GET  /time               ?"  "o "  <o 
//   GET  /dump               ?"  s"   -  "  "    "" (JSON)
//   POST /echo               ?"  s"   " ""  JSON o  o  ' < 
//   POST /chunked-echo       ?" Chunked Transfer o  ' < 
//   GET  /bigdata            ?" 50KB  "     "  (Chunked)
//
//  "o " ?  T z'  '   .O  O  s  ? -  "o http://127.0.0.1:8080  ' ?  "  ? S  .  <^ < .
#include "http.hpp"
#include <windows.h>
#include <ctime>

static constexpr USHORT HTTP_PORT = 8080;

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o "  .  "  Y   "  
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static void registerRoutes(HttpServer& srv) {

    // GET /
    srv.addRoute("GET", "/", [](const HttpRequest&) {
        HttpResponse r;
        r.setBody(
            "=== Hand-Built HTTP/1.1 Server ===\r\n\r\n"
            "GET  /                 ?"    .^ , \r\n"
            "GET  /echo?msg=<text>  ?"     -  "\r\n"
            "GET  /time             ?"  "o "  <o \r\n"
            "GET  /dump             ?"  s"   -  "   "" (JSON)\r\n"
            "POST /echo             ?"  " ""  -  " (JSON)\r\n"
            "POST /chunked-echo     ?" Chunked  ' <   -  "\r\n"
            "GET  /bigdata          ?" 50KB Chunked    " \r\n"
        );
        return r;
    });

    // GET /echo?msg=...
    srv.addRoute("GET", "/echo", [](const HttpRequest& req) {
        HttpResponse r;
        //     OO   "  -  "o msg=  '  " o
        std::string val;
        const std::string& q = req.query;
        size_t p = q.find("msg=");
        if (p != std::string::npos) {
            val = q.substr(p + 4);
            size_t amp = val.find('&');
            if (amp != std::string::npos) val.resize(amp);
        }
        r.setBody("Echo: " + (val.empty() ? "(empty)" : val) + "\r\n");
        return r;
    });

    // GET /time
    srv.addRoute("GET", "/time", [](const HttpRequest&) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        snprintf(buf, sizeof(buf),
                 "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 st.wYear, st.wMonth, st.wDay,
                 st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        HttpResponse r;
        r.setJson("{\"time\":\"" + std::string(buf) + "\"}");
        return r;
    });

    // GET /dump  ?"  s"   -  "  "  JSON o  o  ~ T~
    srv.addRoute("GET", "/dump", [](const HttpRequest& req) {
        std::string json = "{\n";
        json += "  \"method\": \"" + req.method + "\",\n";
        json += "  \"path\": \""   + req.path   + "\",\n";
        json += "  \"query\": \""  + req.query  + "\",\n";
        json += "  \"headers\": {\n";
        bool first = true;
        for (auto& [k, v] : req.headers) {
            if (!first) json += ",\n";
            json += "    \"" + k + "\": \"" + v + "\"";
            first = false;
        }
        json += "\n  }\n}";
        HttpResponse r;
        r.setJson(json);
        return r;
    });

    // POST /echo
    srv.addRoute("POST", "/echo", [](const HttpRequest& req) {
        HttpResponse r;
        r.setJson("{\"echo\":\"" + req.body + "\","
                  "\"length\":" + std::to_string(req.body.size()) + "}");
        return r;
    });

    // POST /chunked-echo  ?" Chunked Transfer Encoding o  o  ' < 
    srv.addRoute("POST", "/chunked-echo", [](const HttpRequest& req) {
        HttpResponse r;
        r.chunked = true;
        r.setJson("{\"chunked\":true,\"echo\":\"" + req.body + "\"}");
        return r;
    });

    // GET /bigdata  ?" 50KB  Chunked o  " < 
    srv.addRoute("GET", "/bigdata", [](const HttpRequest&) {
        HttpResponse r;
        r.chunked = true;
        r.headers["Content-Type"] = "text/plain";
        std::string big(50 * 1024, 'X');
        for (size_t i = 0; i < big.size(); i += 64)
            big[i] = '\n';
        r.body = big;
        return r;
    });
}

static int runServer() {
    HttpServer srv(HTTP_PORT);
    registerRoutes(srv);
    return srv.run() ? 0 : 1;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S : keep-alive o  -  Y   s" 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runClient() {
    HttpClient cli("127.0.0.1", HTTP_PORT);

    struct Test {
        const char* label;
        const char* method;
        const char* path;
        const char* query;   // GET s 
        const char* body;    // POST s 
        const char* ct;
    };

    Test tests[] = {
        { "  S ",          "GET",  "/",              "",              "",                           "" },
        { " -  "(  )",    "GET",  "/echo",           "msg=Winsock!",  "",                           "" },
        { " <o ",          "GET",  "/time",           "",              "",                           "" },
        { " -  "  """,      "GET",  "/dump",           "",              "",                           "" },
        { "POST  -  "",     "POST", "/echo",           "",              "raw socket HTTP payload",    "text/plain" },
        { "Chunked  -  "",  "POST", "/chunked-echo",   "",              "chunked transfer test",      "text/plain" },
        { " O? ~.    " ",   "GET",  "/bigdata",        "",              "",                           "" },
        { "404  .O S  S ",    "GET",  "/notfound",       "",              "",                           "" },
    };

    for (auto& t : tests) {
        printf("\n "? "? %s  "? "? %s %s%s%s  "? "?\n",
               t.label, t.method, t.path,
               t.query[0] ? "?" : "", t.query);

        int code = 0;
        std::string body;
        bool ok = false;

        if (strcmp(t.method, "GET") == 0)
            ok = cli.get(t.path, code, body, t.query);
        else
            ok = cli.post(t.path, t.body, t.ct, code, body);

        if (!ok) { printf("   o-  s"   <  O \n"); continue; }

        printf("  HTTP %d  body=%zu bytes\n", code, body.size());
        //      (100 z )
        size_t preLen = body.size() < 100 ? body.size() : 100;
        std::string preview = body.substr(0, preLen);
        for (auto& c : preview)
            if ((unsigned char)c < 32 && c != '\n' && c != '\r') c = '.';
        printf("  %s%s\n", preview.c_str(), body.size() > 100 ? " ..." : "");

        Sleep(50);
    }

    printf("\n[    -  S ]   "   s"   T" O\n");
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
int main(int argc, char* argv[]) {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup  <  O : %d\n", WSAGetLastError());
        return 1;
    }

    bool isServer = (argc < 2 || strcmp(argv[1], "server") == 0);
    int ret = isServer ? runServer() : runClient();

    WSACleanup();
    return ret;
}
