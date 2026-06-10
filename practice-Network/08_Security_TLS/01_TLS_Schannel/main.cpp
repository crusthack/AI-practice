#include "NetUtil.hpp"
#define SECURITY_WIN32
#include <schannel.h>
#include <security.h>
#include <string>
#include <vector>
#pragma comment(lib, "secur32.lib")

static constexpr const char* HOST = "example.com";

static bool connect_host(net::Socket& sock) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* result = nullptr;
    if (getaddrinfo(HOST, "443", &hints, &result) != 0) return false;
    sock.reset(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    bool ok = sock && connect(sock.value, result->ai_addr, (int)result->ai_addrlen) == 0;
    freeaddrinfo(result);
    return ok;
}

static bool send_security_buffer(SOCKET s, SecBuffer& buffer) {
    if (buffer.cbBuffer == 0 || buffer.pvBuffer == nullptr) return true;
    bool ok = net::send_all(s, static_cast<const char*>(buffer.pvBuffer), (int)buffer.cbBuffer) == (int)buffer.cbBuffer;
    FreeContextBuffer(buffer.pvBuffer);
    buffer.pvBuffer = nullptr;
    buffer.cbBuffer = 0;
    return ok;
}

static SECURITY_STATUS tls_handshake(SOCKET s, CredHandle& cred, CtxtHandle& ctx, std::vector<char>& extra) {
    DWORD flags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
                  ISC_REQ_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;
    DWORD attrs = 0;
    TimeStamp expiry{};

    SecBuffer out_buf{0, SECBUFFER_TOKEN, nullptr};
    SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};
    SECURITY_STATUS sc = InitializeSecurityContextA(&cred, nullptr, const_cast<char*>(HOST), flags, 0, 0,
                                                    nullptr, 0, &ctx, &out_desc, &attrs, &expiry);
    if (!send_security_buffer(s, out_buf)) return SEC_E_INTERNAL_ERROR;
    if (sc != SEC_I_CONTINUE_NEEDED) return sc;

    std::vector<char> input;
    char recv_buf[8192];
    while (sc == SEC_I_CONTINUE_NEEDED || sc == SEC_E_INCOMPLETE_MESSAGE) {
        if (sc == SEC_E_INCOMPLETE_MESSAGE || input.empty()) {
            int n = recv(s, recv_buf, sizeof(recv_buf), 0);
            if (n <= 0) return SEC_E_INTERNAL_ERROR;
            input.insert(input.end(), recv_buf, recv_buf + n);
        }

        SecBuffer in_bufs[2]{};
        in_bufs[0].BufferType = SECBUFFER_TOKEN;
        in_bufs[0].pvBuffer = input.data();
        in_bufs[0].cbBuffer = (unsigned long)input.size();
        in_bufs[1].BufferType = SECBUFFER_EMPTY;
        SecBufferDesc in_desc{SECBUFFER_VERSION, 2, in_bufs};

        out_buf = SecBuffer{0, SECBUFFER_TOKEN, nullptr};
        out_desc = SecBufferDesc{SECBUFFER_VERSION, 1, &out_buf};
        sc = InitializeSecurityContextA(&cred, &ctx, const_cast<char*>(HOST), flags, 0, 0,
                                        &in_desc, 0, &ctx, &out_desc, &attrs, &expiry);
        if (!send_security_buffer(s, out_buf)) return SEC_E_INTERNAL_ERROR;

        if (sc == SEC_E_INCOMPLETE_MESSAGE) continue;
        if (sc != SEC_E_OK && sc != SEC_I_CONTINUE_NEEDED) return sc;

        if (in_bufs[1].BufferType == SECBUFFER_EXTRA && in_bufs[1].cbBuffer > 0) {
            char* begin = input.data() + input.size() - in_bufs[1].cbBuffer;
            extra.assign(begin, begin + in_bufs[1].cbBuffer);
        } else {
            extra.clear();
        }
        input = extra;
    }
    return sc;
}

static bool tls_send(SOCKET s, CtxtHandle& ctx, const SecPkgContext_StreamSizes& sizes, const std::string& plain) {
    std::vector<char> packet(sizes.cbHeader + plain.size() + sizes.cbTrailer);
    std::memcpy(packet.data() + sizes.cbHeader, plain.data(), plain.size());
    SecBuffer bufs[4]{};
    bufs[0] = SecBuffer{sizes.cbHeader, SECBUFFER_STREAM_HEADER, packet.data()};
    bufs[1] = SecBuffer{(unsigned long)plain.size(), SECBUFFER_DATA, packet.data() + sizes.cbHeader};
    bufs[2] = SecBuffer{sizes.cbTrailer, SECBUFFER_STREAM_TRAILER, packet.data() + sizes.cbHeader + plain.size()};
    bufs[3] = SecBuffer{0, SECBUFFER_EMPTY, nullptr};
    SecBufferDesc desc{SECBUFFER_VERSION, 4, bufs};
    SECURITY_STATUS sc = EncryptMessage(&ctx, 0, &desc, 0);
    if (sc != SEC_E_OK) return false;
    int total = (int)(bufs[0].cbBuffer + bufs[1].cbBuffer + bufs[2].cbBuffer);
    return net::send_all(s, packet.data(), total) == total;
}

static void tls_recv_print(SOCKET s, CtxtHandle& ctx, std::vector<char> encrypted) {
    char buf[8192];
    while (true) {
        if (encrypted.empty()) {
            int n = recv(s, buf, sizeof(buf), 0);
            if (n <= 0) break;
            encrypted.assign(buf, buf + n);
        }

        SecBuffer bufs[4]{};
        bufs[0] = SecBuffer{(unsigned long)encrypted.size(), SECBUFFER_DATA, encrypted.data()};
        bufs[1].BufferType = SECBUFFER_EMPTY;
        bufs[2].BufferType = SECBUFFER_EMPTY;
        bufs[3].BufferType = SECBUFFER_EMPTY;
        SecBufferDesc desc{SECBUFFER_VERSION, 4, bufs};
        SECURITY_STATUS sc = DecryptMessage(&ctx, &desc, 0, nullptr);
        if (sc == SEC_E_INCOMPLETE_MESSAGE) {
            int n = recv(s, buf, sizeof(buf), 0);
            if (n <= 0) break;
            encrypted.insert(encrypted.end(), buf, buf + n);
            continue;
        }
        if (sc == SEC_I_CONTEXT_EXPIRED) break;
        if (sc != SEC_E_OK && sc != SEC_I_RENEGOTIATE) {
            std::printf("DecryptMessage failed: 0x%08lx\n", sc);
            break;
        }

        std::vector<char> extra;
        for (SecBuffer& b : bufs) {
            if (b.BufferType == SECBUFFER_DATA && b.cbBuffer > 0) {
                std::fwrite(b.pvBuffer, 1, b.cbBuffer, stdout);
            } else if (b.BufferType == SECBUFFER_EXTRA && b.cbBuffer > 0) {
                char* p = static_cast<char*>(b.pvBuffer);
                extra.assign(p, p + b.cbBuffer);
            }
        }
        encrypted = extra;
    }
}

int main() {
    net::WsaSession wsa;
    net::Socket sock;
    if (!connect_host(sock)) {
        net::print_error("connect");
        return 1;
    }

    SCHANNEL_CRED schannel{};
    schannel.dwVersion = SCHANNEL_CRED_VERSION;
    schannel.grbitEnabledProtocols = SP_PROT_TLS1_2_CLIENT | SP_PROT_TLS1_3_CLIENT;

    CredHandle cred{};
    CtxtHandle ctx{};
    TimeStamp expiry{};
    SECURITY_STATUS sc = AcquireCredentialsHandleA(nullptr, const_cast<char*>(UNISP_NAME_A), SECPKG_CRED_OUTBOUND,
                                                   nullptr, &schannel, nullptr, nullptr, &cred, &expiry);
    if (sc != SEC_E_OK) {
        std::printf("AcquireCredentialsHandle failed: 0x%08lx\n", sc);
        return 1;
    }

    std::vector<char> extra;
    sc = tls_handshake(sock.value, cred, ctx, extra);
    if (sc != SEC_E_OK) {
        std::printf("TLS handshake failed: 0x%08lx\n", sc);
        FreeCredentialsHandle(&cred);
        return 1;
    }

    SecPkgContext_StreamSizes sizes{};
    QueryContextAttributesA(&ctx, SECPKG_ATTR_STREAM_SIZES, &sizes);
    std::string request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\nAccept: */*\r\n\r\n";
    if (tls_send(sock.value, ctx, sizes, request)) tls_recv_print(sock.value, ctx, extra);

    DeleteSecurityContext(&ctx);
    FreeCredentialsHandle(&cred);
    return 0;
}
