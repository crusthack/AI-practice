#pragma once
#include "ip_headers.hpp"
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <vector>

//                                                                             
// HTTP/1.1   SOCK_STREAM             .
//
//       :
//   TCP(SOCK_STREAM)                 "   "      .
//   HTTP                       :
//     -   /      +      \r\n       , \r\n\r\n      
//     -         Content-Length       Chunked Transfer Encoding
//
//        :
//     HttpRequest         
//     HttpResponse         
//     HttpRouter        +         
//     HttpServer     accept    + Connection: keep-alive
//     HttpClient           (keep-alive) + Chunked    
//                                                                             

//                                                                         
static inline std::string strLower(std::string s) {
    for (auto& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

//                                                                        
static inline std::string strTrim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return {};
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

//                                                                             
// HTTP   
//                                                                             
struct HttpRequest {
    std::string method;     // GET POST PUT DELETE ...
    std::string path;       //    (/foo/bar)
    std::string query;      //        (key=val&...)
    std::string version;    // HTTP/1.0 or HTTP/1.1
    std::unordered_map<std::string, std::string> headers; //      
    std::string body;

    //       
    bool isKeepAlive() const {
        auto it = headers.find("connection");
        if (it == headers.end())
            return version == "HTTP/1.1";   // 1.1       keep-alive
        return strLower(it->second) != "close";
    }

    std::string header(const std::string& key) const {
        auto it = headers.find(strLower(key));
        return it != headers.end() ? it->second : "";
    }

    void print() const {
        printf("  %s %s%s%s %s\n", method.c_str(), path.c_str(),
               query.empty() ? "" : "?", query.c_str(), version.c_str());
        for (auto& [k, v] : headers)
            printf("  %s: %s\n", k.c_str(), v.c_str());
        if (!body.empty())
            printf("  body(%zu): %s\n", body.size(), body.substr(0, 80).c_str());
    }
};

//                                                                             
// HTTP      
//                                                                             
struct HttpResponse {
    int         status    = 200;
    std::string statusMsg = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool        chunked   = false;  // Chunked Transfer Encoding      

    HttpResponse& setStatus(int s, const char* msg) {
        status = s; statusMsg = msg; return *this;
    }
    HttpResponse& setHeader(const std::string& k, const std::string& v) {
        headers[k] = v; return *this;
    }
    HttpResponse& setBody(const std::string& b,
                          const std::string& ct = "text/plain; charset=utf-8") {
        body = b;
        headers["Content-Type"] = ct;
        return *this;
    }
    HttpResponse& setJson(const std::string& j) {
        return setBody(j, "application/json; charset=utf-8");
    }

    //     HTTP/1.1                 .
    std::string serialize(bool keepAlive = false) const {
        std::string out;
        out += "HTTP/1.1 " + std::to_string(status) + " " + statusMsg + "\r\n";
        out += "Server: HandBuiltHTTP/2.0\r\n";
        out += std::string("Connection: ") + (keepAlive ? "keep-alive" : "close") + "\r\n";

        if (chunked) {
            out += "Transfer-Encoding: chunked\r\n";
            for (auto& [k, v] : headers) out += k + ": " + v + "\r\n";
            out += "\r\n";
            // Chunked    : [hex   ]\r\n[   ]\r\n ... 0\r\n\r\n
            if (!body.empty()) {
                std::ostringstream ss;
                ss << std::hex << body.size();
                out += ss.str() + "\r\n" + body + "\r\n";
            }
            out += "0\r\n\r\n";
        } else {
            out += "Content-Length: " + std::to_string(body.size()) + "\r\n";
            for (auto& [k, v] : headers) out += k + ": " + v + "\r\n";
            out += "\r\n";
            out += body;
        }
        return out;
    }

    static HttpResponse make404(const std::string& path) {
        HttpResponse r;
        r.setStatus(404, "Not Found")
         .setBody("404 Not Found: " + path + "\r\n");
        return r;
    }
    static HttpResponse make405(const std::string& allow) {
        HttpResponse r;
        r.setStatus(405, "Method Not Allowed")
         .setHeader("Allow", allow)
         .setBody("405 Method Not Allowed\r\n");
        return r;
    }
    static HttpResponse make400() {
        HttpResponse r;
        r.setStatus(400, "Bad Request").setBody("400 Bad Request\r\n");
        return r;
    }
};

//                                                                             
// HTTP    (        HttpRequest   )
//                                                                             
class HttpParser {
public:
    //     (\r\n\r\n)             
    //    : true=  , false=              
    static bool parseRequest(SOCKET s, HttpRequest& req) {
        //                                                           
        std::string raw;
        char chunk[512];
        size_t hdrEnd = std::string::npos;
        while (hdrEnd == std::string::npos) {
            int r = recv(s, chunk, sizeof(chunk), 0);
            if (r <= 0) return false;
            raw.append(chunk, r);
            hdrEnd = raw.find("\r\n\r\n");
            if (raw.size() > 64 * 1024) return false;  //        
        }
        std::string bodyOverflow = raw.substr(hdrEnd + 4);
        std::string hdrBlock     = raw.substr(0, hdrEnd);

        //                                                           
        size_t lineEnd = hdrBlock.find("\r\n");
        if (lineEnd == std::string::npos) return false;
        std::string reqLine = hdrBlock.substr(0, lineEnd);

        size_t s1 = reqLine.find(' ');
        size_t s2 = reqLine.rfind(' ');
        if (s1 == std::string::npos || s1 == s2) return false;

        req.method  = reqLine.substr(0, s1);
        req.version = reqLine.substr(s2 + 1);

        std::string pathQuery = reqLine.substr(s1 + 1, s2 - s1 - 1);
        size_t qPos = pathQuery.find('?');
        if (qPos != std::string::npos) {
            req.path  = pathQuery.substr(0, qPos);
            req.query = pathQuery.substr(qPos + 1);
        } else {
            req.path = pathQuery;
        }

        //                                                             
        size_t pos = lineEnd + 2;
        while (pos < hdrBlock.size()) {
            size_t end = hdrBlock.find("\r\n", pos);
            if (end == std::string::npos) end = hdrBlock.size();
            std::string line = hdrBlock.substr(pos, end - pos);
            size_t col = line.find(':');
            if (col != std::string::npos) {
                req.headers[strLower(line.substr(0, col))] =
                    strTrim(line.substr(col + 1));
            }
            pos = end + 2;
        }

        //                                                             
        auto cl = req.headers.find("content-length");
        if (cl != req.headers.end()) {
            int need = std::stoi(cl->second);
            req.body = bodyOverflow;
            int rem  = need - (int)bodyOverflow.size();
            while (rem > 0) {
                int r = recv(s, chunk, (int)min((int)sizeof(chunk), rem), 0);
                if (r <= 0) return false;
                req.body.append(chunk, r);
                rem -= r;
            }
        } else if (req.headers.count("transfer-encoding")) {
            // Chunked       
            req.body = decodeChunked(s, bodyOverflow);
        }
        return true;
    }

    // Chunked Transfer Encoding    
    static std::string decodeChunked(SOCKET s, const std::string& initial) {
        std::string buf = initial;
        std::string result;
        char tmp[512];

        auto readMore = [&]() {
            int r = recv(s, tmp, sizeof(tmp), 0);
            if (r > 0) buf.append(tmp, r);
        };

        while (true) {
            //                    
            while (buf.find("\r\n") == std::string::npos) readMore();
            size_t nl = buf.find("\r\n");
            int chunkSize = (int)std::stoul(buf.substr(0, nl), nullptr, 16);
            buf.erase(0, nl + 2);
            if (chunkSize == 0) break;

            //        + \r\n
            while ((int)buf.size() < chunkSize + 2) readMore();
            result.append(buf.data(), chunkSize);
            buf.erase(0, chunkSize + 2);
        }
        return result;
    }
};

//                                                                             
// HTTP    
//                                                                             
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

class HttpRouter {
public:
    //       : method  ""             
    void addRoute(const std::string& method, const std::string& path,
                  HttpHandler handler) {
        routes_.push_back({ strLower(method), path, handler });
    }

    HttpResponse dispatch(const HttpRequest& req) const {
        bool pathFound = false;
        for (auto& [m, p, h] : routes_) {
            if (p != req.path) continue;
            pathFound = true;
            if (!m.empty() && m != strLower(req.method)) continue;
            return h(req);
        }
        if (pathFound) return HttpResponse::make405("GET, POST");
        return HttpResponse::make404(req.path);
    }

private:
    struct Route { std::string method, path; HttpHandler handler; };
    std::vector<Route> routes_;
};

//                                                                             
// HTTP   
//                                                                             
class HttpServer {
public:
    explicit HttpServer(USHORT port) : port_(port) {}

    void addRoute(const std::string& method, const std::string& path,
                  HttpHandler handler) {
        router_.addRoute(method, path, handler);
    }

    //    accept    (Ctrl+C    )
    bool run() {
        SOCKET ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        int opt = 1;
        setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(ls, (sockaddr*)&addr, sizeof(addr));
        listen(ls, SOMAXCONN);

        printf("[HttpServer] http://127.0.0.1:%u  (    /curl      )\n", port_);

        while (true) {
            sockaddr_in cli{};
            int cLen = sizeof(cli);
            SOCKET conn = accept(ls, (sockaddr*)&cli, &cLen);
            if (conn == INVALID_SOCKET) continue;

            char cIp[16];
            inet_ntop(AF_INET, &cli.sin_addr, cIp, sizeof(cIp));
            handleConnection(conn, cIp);
        }
        closesocket(ls);
        return true;
    }

private:
    USHORT     port_;
    HttpRouter router_;

    void handleConnection(SOCKET conn, const char* cIp) {
        // Keep-alive:                    
        while (true) {
            HttpRequest req;
            if (!HttpParser::parseRequest(conn, req)) break;

            printf("[%s] %s %s%s%s\n", cIp,
                   req.method.c_str(), req.path.c_str(),
                   req.query.empty() ? "" : "?", req.query.c_str());

            HttpResponse resp = router_.dispatch(req);
            bool keepAlive    = req.isKeepAlive();
            std::string out   = resp.serialize(keepAlive);
            send(conn, out.c_str(), (int)out.size(), 0);

            printf("      %d %s  (%zu bytes)  keep-alive=%d\n",
                   resp.status, resp.statusMsg.c_str(),
                   resp.body.size(), keepAlive);

            if (!keepAlive) break;
        }
        closesocket(conn);
    }
};

//                                                                             
// HTTP       (keep-alive       )
//                                                                             
class HttpClient {
public:
    HttpClient(const char* host, USHORT port)
        : host_(host), port_(port), sock_(INVALID_SOCKET) {}

    ~HttpClient() { disconnect(); }

    // GET   
    bool get(const std::string& path,
             int& statusOut, std::string& bodyOut,
             const std::string& queryStr = "") {
        std::string fullPath = queryStr.empty() ? path : path + "?" + queryStr;
        return request("GET", fullPath, "", "", statusOut, bodyOut);
    }

    // POST   
    bool post(const std::string& path, const std::string& body,
              const std::string& ct,
              int& statusOut, std::string& bodyOut) {
        return request("POST", path, body, ct, statusOut, bodyOut);
    }

    void disconnect() {
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
    }

private:
    std::string host_;
    USHORT      port_;
    SOCKET      sock_;

    bool ensureConnected() {
        if (sock_ != INVALID_SOCKET) return true;
        sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in sv{};
        sv.sin_family = AF_INET;
        sv.sin_port   = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &sv.sin_addr);
        if (connect(sock_, (sockaddr*)&sv, sizeof(sv)) == SOCKET_ERROR) {
            closesocket(sock_); sock_ = INVALID_SOCKET;
            return false;
        }
        return true;
    }

    bool request(const std::string& method, const std::string& path,
                 const std::string& body, const std::string& ct,
                 int& statusOut, std::string& bodyOut) {
        //              
        if (!ensureConnected()) {
            printf("[HttpClient]      \n");
            return false;
        }

        //      
        std::string req;
        req += method + " " + path + " HTTP/1.1\r\n";
        req += "Host: " + host_ + ":" + std::to_string(port_) + "\r\n";
        req += "User-Agent: HandBuiltClient/2.0\r\n";
        req += "Accept: */*\r\n";
        req += "Connection: keep-alive\r\n";
        if (!body.empty()) {
            req += "Content-Type: " + ct + "\r\n";
            req += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        }
        req += "\r\n";
        req += body;

        if (send(sock_, req.c_str(), (int)req.size(), 0) == SOCKET_ERROR) {
            disconnect();
            return false;
        }

        //         
        std::string raw;
        char chunk[512];
        size_t hdrEnd = std::string::npos;
        while (hdrEnd == std::string::npos) {
            int r = recv(sock_, chunk, sizeof(chunk), 0);
            if (r <= 0) { disconnect(); return false; }
            raw.append(chunk, r);
            hdrEnd = raw.find("\r\n\r\n");
        }
        std::string overflow  = raw.substr(hdrEnd + 4);
        std::string hdrBlock  = raw.substr(0, hdrEnd);

        //         
        size_t le = hdrBlock.find("\r\n");
        if (le == std::string::npos) return false;
        std::string sl = hdrBlock.substr(0, le);
        size_t sp = sl.find(' ');
        if (sp == std::string::npos) return false;
        statusOut = std::stoi(sl.substr(sp + 1, 3));

        //      
        std::unordered_map<std::string, std::string> respHdrs;
        size_t pos = le + 2;
        while (pos < hdrBlock.size()) {
            size_t end = hdrBlock.find("\r\n", pos);
            if (end == std::string::npos) end = hdrBlock.size();
            std::string line = hdrBlock.substr(pos, end - pos);
            size_t col = line.find(':');
            if (col != std::string::npos)
                respHdrs[strLower(line.substr(0, col))] =
                    strTrim(line.substr(col + 1));
            pos = end + 2;
        }

        //    keep-alive       (    close        )
        bool serverKeepAlive = true;
        auto cit = respHdrs.find("connection");
        if (cit != respHdrs.end() && strLower(cit->second) == "close")
            serverKeepAlive = false;

        //      
        auto cl = respHdrs.find("content-length");
        auto te = respHdrs.find("transfer-encoding");
        if (te != respHdrs.end() && strLower(te->second) == "chunked") {
            bodyOut = HttpParser::decodeChunked(sock_, overflow);
        } else if (cl != respHdrs.end()) {
            int need = std::stoi(cl->second);
            bodyOut  = overflow;
            int rem  = need - (int)overflow.size();
            while (rem > 0) {
                int r = recv(sock_, chunk, (int)min((int)sizeof(chunk), rem), 0);
                if (r <= 0) break;
                bodyOut.append(chunk, r);
                rem -= r;
            }
        } else {
            // Connection: close   EOF     
            bodyOut = overflow;
            while (true) {
                int r = recv(sock_, chunk, sizeof(chunk), 0);
                if (r <= 0) break;
                bodyOut.append(chunk, r);
            }
            serverKeepAlive = false;
        }

        if (!serverKeepAlive) disconnect();
        return true;
    }
};
