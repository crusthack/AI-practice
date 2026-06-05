// 27_Cryptography — CNG + DPAPI + Legacy CryptoAPI Workbench
// Modes: Hash Lab, AES, RSA, ECDSA Signature, DPAPI, Random & HMAC
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define NT_SUCCESS(s)  ((NTSTATUS)(s) >= 0)

namespace {

enum {
    IDC_BTN_HASH   = 101,
    IDC_BTN_AES    = 102,
    IDC_BTN_RSA    = 103,
    IDC_BTN_ECDSA  = 104,
    IDC_BTN_DPAPI2 = 105,
    IDC_BTN_RANDOM = 106,
    IDC_EDIT_IN    = 107,
    IDC_EDIT_OUT   = 108,
    IDC_STATUS     = 109,
    IDC_LBL_INPUT  = 110,
};

HWND      g_hwnd    = nullptr;
HWND      g_hEditIn = nullptr;
HWND      g_hEdit   = nullptr;
HWND      g_hStatus = nullptr;
HINSTANCE g_hInst   = nullptr;

// ── Helpers ──────────────────────────────────────────────────────────────────

void StatusSet(const wchar_t* msg) {
    SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)msg);
}
void StatusSetR(const wchar_t* right) {
    SendMessage(g_hStatus, SB_SETTEXT, 1, (LPARAM)right);
}

void OutClear() { SetWindowTextW(g_hEdit, L""); }

void OutLine(const wchar_t* text) {
    int len = GetWindowTextLengthW(g_hEdit);
    SendMessage(g_hEdit, EM_SETSEL, len, len);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
}

void OutFmt(const wchar_t* fmt, ...) {
    wchar_t buf[1024];
    va_list va; va_start(va, fmt);
    StringCchVPrintfW(buf, 1024, fmt, va);
    va_end(va);
    OutLine(buf);
}

// Convert bytes to hex string (space-separated groups of 2)
void BytesToHex(const BYTE* data, DWORD len, wchar_t* out, DWORD outCch) {
    out[0] = L'\0';
    for (DWORD i = 0; i < len && (i * 3 + 3) < outCch; i++) {
        wchar_t h[4];
        StringCchPrintf(h, 4, L"%02X", data[i]);
        StringCchCat(out, outCch, h);
    }
}

// Get input text as UTF-8 bytes for hashing
DWORD GetInputBytes(BYTE* buf, DWORD bufSize) {
    wchar_t wbuf[1024];
    GetWindowTextW(g_hEditIn, wbuf, 1024);
    if (!wbuf[0]) StringCchCopyW(wbuf, 1024, L"Hello, Win32 Cryptography!");
    int n = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, (char*)buf, bufSize, nullptr, nullptr);
    return (n > 0) ? (DWORD)(n - 1) : 0;
}

// ── Mode 1: Hash Lab ─────────────────────────────────────────────────────────

// SHA-1 via legacy CryptoAPI
BOOL LegacyHash(ALG_ID algId, const BYTE* data, DWORD dataLen,
                BYTE* digest, DWORD* digestLen) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContextW(&hProv, nullptr, nullptr,
            PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return FALSE;
    if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0); return FALSE;
    }
    CryptHashData(hHash, data, dataLen, 0);
    *digestLen = *digestLen; // pass in as buffer size
    BOOL ok = CryptGetHashParam(hHash, HP_HASHVAL, digest, digestLen, 0);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return ok;
}

// BCrypt hash
BOOL BcryptHash(LPCWSTR algo, const BYTE* data, DWORD dataLen,
                BYTE* digest, DWORD* digestLen) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, algo, nullptr, 0)))
        return FALSE;
    DWORD hashLen = 0, retLen = 0;
    BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (BYTE*)&hashLen, sizeof(hashLen), &retLen, 0);
    if (*digestLen < hashLen) { BCryptCloseAlgorithmProvider(hAlg, 0); return FALSE; }
    *digestLen = hashLen;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    BCryptHashData(hHash, (PUCHAR)data, dataLen, 0);
    BCryptFinishHash(hHash, digest, hashLen, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return TRUE;
}

void ModeHash() {
    OutClear();
    OutLine(L"=== Hash Lab: SHA-1 (CryptoAPI) + SHA-256/384/512 (BCrypt) + MD5 (CryptoAPI) ===");
    OutLine(L"");

    BYTE data[1024]; DWORD dataLen = GetInputBytes(data, sizeof(data));
    wchar_t hexBuf[256];
    OutFmt(L"Input: %u bytes", dataLen);
    OutLine(L"");

    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);

    // SHA-1 (legacy)
    BYTE sha1[20]; DWORD sha1Len = 20;
    QueryPerformanceCounter(&t1);
    if (LegacyHash(CALG_SHA1, data, dataLen, sha1, &sha1Len)) {
        QueryPerformanceCounter(&t2);
        BytesToHex(sha1, sha1Len, hexBuf, 256);
        OutFmt(L"SHA-1   (CryptoAPI): %s", hexBuf);
        OutFmt(L"         Time: %.3f ms", (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart);
    }

    // MD5 (legacy)
    BYTE md5[16]; DWORD md5Len = 16;
    QueryPerformanceCounter(&t1);
    if (LegacyHash(CALG_MD5, data, dataLen, md5, &md5Len)) {
        QueryPerformanceCounter(&t2);
        BytesToHex(md5, md5Len, hexBuf, 256);
        OutFmt(L"MD5     (CryptoAPI): %s", hexBuf);
        OutFmt(L"         Time: %.3f ms", (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart);
    }

    // SHA-256 (BCrypt)
    BYTE sha256[32]; DWORD sh256Len = 32;
    QueryPerformanceCounter(&t1);
    if (BcryptHash(BCRYPT_SHA256_ALGORITHM, data, dataLen, sha256, &sh256Len)) {
        QueryPerformanceCounter(&t2);
        BytesToHex(sha256, sh256Len, hexBuf, 256);
        OutFmt(L"SHA-256 (BCrypt):    %s", hexBuf);
        OutFmt(L"         Time: %.3f ms", (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart);
    }

    // SHA-384 (BCrypt)
    BYTE sha384[48]; DWORD sh384Len = 48;
    QueryPerformanceCounter(&t1);
    if (BcryptHash(BCRYPT_SHA384_ALGORITHM, data, dataLen, sha384, &sh384Len)) {
        QueryPerformanceCounter(&t2);
        BytesToHex(sha384, sh384Len, hexBuf, 256);
        OutFmt(L"SHA-384 (BCrypt):    %s...", hexBuf); // long, truncate display
        OutFmt(L"         Time: %.3f ms", (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart);
    }

    // SHA-512 (BCrypt)
    BYTE sha512[64]; DWORD sh512Len = 64;
    QueryPerformanceCounter(&t1);
    if (BcryptHash(BCRYPT_SHA512_ALGORITHM, data, dataLen, sha512, &sh512Len)) {
        QueryPerformanceCounter(&t2);
        // Show first 32 bytes of 64
        BytesToHex(sha512, 32, hexBuf, 256);
        OutFmt(L"SHA-512 (BCrypt):    %s...", hexBuf);
        OutFmt(L"         Time: %.3f ms", (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart);
    }

    StatusSet(L"Hash Lab — CryptCreateHash(SHA1,MD5) + BCryptCreateHash(SHA256/384/512)");
}

// ── Mode 2: AES Encryption ────────────────────────────────────────────────────

void ModeAES() {
    OutClear();
    OutLine(L"=== AES-128-CBC and AES-256-CBC (BCrypt) ===");
    OutLine(L"");

    BYTE plainData[1024]; DWORD dataLen = GetInputBytes(plainData, 1000);
    if (dataLen == 0) {
        const char* def = "Win32 AES Demo Text";
        dataLen = (DWORD)lstrlenA(def);
        CopyMemory(plainData, def, dataLen);
    }
    OutFmt(L"Plaintext: %u bytes", dataLen);

    const struct { LPCWSTR algo; DWORD keyBits; } modes[] = {
        {BCRYPT_AES_ALGORITHM, 128},
        {BCRYPT_AES_ALGORITHM, 256},
    };

    for (auto& m : modes) {
        OutLine(L"");
        OutFmt(L"--- AES-%u-CBC ---", m.keyBits);

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCryptOpenAlgorithmProvider(&hAlg, m.algo, nullptr, 0);
        BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
            (BYTE*)BCRYPT_CHAIN_MODE_CBC,
            (DWORD)((wcslen(BCRYPT_CHAIN_MODE_CBC) + 1) * sizeof(wchar_t)), 0);

        DWORD keyBytes = m.keyBits / 8;
        BYTE  keyData[32] = {};
        // Fill key with pattern
        for (DWORD i = 0; i < keyBytes; i++) keyData[i] = (BYTE)(i + 1);

        // Generate IV (all 0x2A for demo)
        DWORD blockLen = 16;
        BYTE  iv[16] = {};
        for (int i = 0; i < 16; i++) iv[i] = 0x2A;
        BYTE  ivCopy[16];
        CopyMemory(ivCopy, iv, 16);

        // Generate key object
        DWORD objLen = 0, retLen = 0;
        BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (BYTE*)&objLen, sizeof(objLen), &retLen, 0);
        BYTE* keyObj = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, objLen);

        BCRYPT_KEY_HANDLE hKey = nullptr;
        NTSTATUS s = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj, objLen,
            keyData, keyBytes, 0);

        if (NT_SUCCESS(s)) {
            // Determine ciphertext size (PKCS7 padding: round up to block)
            DWORD cipherLen = 0;
            DWORD padDataLen = ((dataLen / blockLen) + 1) * blockLen;
            BYTE* cipher = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, padDataLen + 16);

            BCryptEncrypt(hKey, plainData, dataLen, nullptr,
                ivCopy, blockLen, cipher, padDataLen, &cipherLen, BCRYPT_BLOCK_PADDING);

            wchar_t hexBuf[256];
            BytesToHex(keyData, min(keyBytes, (DWORD)16), hexBuf, 256);
            OutFmt(L"Key (first 16B hex): %s...", hexBuf);
            BytesToHex(iv, 16, hexBuf, 256);
            OutFmt(L"IV  (16B hex):       %s", hexBuf);
            BytesToHex(cipher, min(cipherLen, (DWORD)32), hexBuf, 256);
            OutFmt(L"Cipher (first 32B):  %s...", hexBuf);
            OutFmt(L"Ciphertext length:   %u bytes", cipherLen);

            // Decrypt
            CopyMemory(ivCopy, iv, 16);
            BYTE* plain2 = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cipherLen + 16);
            DWORD plain2Len = 0;
            // Re-import key (IV is consumed)
            BCRYPT_KEY_HANDLE hKey2 = nullptr;
            BCryptGenerateSymmetricKey(hAlg, &hKey2, keyObj, objLen, keyData, keyBytes, 0);
            BCryptDecrypt(hKey2, cipher, cipherLen, nullptr,
                ivCopy, blockLen, plain2, cipherLen, &plain2Len, BCRYPT_BLOCK_PADDING);

            BOOL match = (plain2Len == dataLen &&
                          memcmp(plain2, plainData, dataLen) == 0);
            OutFmt(L"Decrypt match:       %s (%u bytes)", match ? L"YES" : L"NO", plain2Len);

            BCryptDestroyKey(hKey2);
            HeapFree(GetProcessHeap(), 0, plain2);
            HeapFree(GetProcessHeap(), 0, cipher);
            BCryptDestroyKey(hKey);
        } else {
            OutFmt(L"BCryptGenerateSymmetricKey failed: 0x%08X", s);
        }

        HeapFree(GetProcessHeap(), 0, keyObj);
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }

    StatusSet(L"AES — BCryptOpenAlgorithmProvider + BCryptGenerateSymmetricKey + BCryptEncrypt/Decrypt");
}

// ── Mode 3: RSA Operations ────────────────────────────────────────────────────

void ModeRSA() {
    OutClear();
    OutLine(L"=== RSA 1024-bit (BCrypt) — Encrypt/Decrypt OAEP ===");
    OutLine(L"");

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS s = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0);
    if (!NT_SUCCESS(s)) {
        OutFmt(L"BCryptOpenAlgorithmProvider(RSA) failed: 0x%08X", s);
        return;
    }

    OutLine(L"Generating 1024-bit RSA key pair...");
    BCRYPT_KEY_HANDLE hKey = nullptr;
    s = BCryptGenerateKeyPair(hAlg, &hKey, 1024, 0);
    if (!NT_SUCCESS(s)) {
        OutFmt(L"BCryptGenerateKeyPair failed: 0x%08X", s);
        BCryptCloseAlgorithmProvider(hAlg, 0); return;
    }

    s = BCryptFinalizeKeyPair(hKey, 0);
    if (!NT_SUCCESS(s)) {
        OutFmt(L"BCryptFinalizeKeyPair failed: 0x%08X", s);
        BCryptDestroyKey(hKey); BCryptCloseAlgorithmProvider(hAlg, 0); return;
    }
    OutLine(L"BCryptFinalizeKeyPair: OK");

    // Export public key blob
    DWORD pubBlobLen = 0;
    BCryptExportKey(hKey, nullptr, BCRYPT_PUBLIC_KEY_BLOB, nullptr, 0, &pubBlobLen, 0);
    BYTE* pubBlob = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pubBlobLen);
    s = BCryptExportKey(hKey, nullptr, BCRYPT_PUBLIC_KEY_BLOB,
        pubBlob, pubBlobLen, &pubBlobLen, 0);
    if (NT_SUCCESS(s)) {
        OutFmt(L"Public key blob: %u bytes (BCRYPT_PUBLIC_KEY_BLOB)", pubBlobLen);
        wchar_t hexBuf[128];
        BytesToHex(pubBlob, min(pubBlobLen, (DWORD)24), hexBuf, 128);
        OutFmt(L"Pub key header (hex): %s...", hexBuf);

        // The BCRYPT_RSAKEY_BLOB header tells us the key size
        BCRYPT_RSAKEY_BLOB* rsaBlob = (BCRYPT_RSAKEY_BLOB*)pubBlob;
        OutFmt(L"  Magic:       0x%08X (RSAPUBLIC=0x%08X)",
            rsaBlob->Magic, BCRYPT_RSAPUBLIC_MAGIC);
        OutFmt(L"  BitLength:   %u", rsaBlob->BitLength);
        OutFmt(L"  cbPublicExp: %u", rsaBlob->cbPublicExp);
        OutFmt(L"  cbModulus:   %u", rsaBlob->cbModulus);
    }

    // Plaintext to encrypt
    BYTE plainData[64]; DWORD plainLen = GetInputBytes(plainData, 60);
    if (plainLen == 0 || plainLen > 60) {
        const char* def = "RSA OAEP Demo";
        plainLen = (DWORD)lstrlenA(def);
        CopyMemory(plainData, def, plainLen);
    }
    OutFmt(L"\nPlaintext (%u bytes): encrypting with RSA-OAEP...", plainLen);

    // Encrypt with OAEP using public key (we use hKey which has both parts)
    BCRYPT_OAEP_PADDING_INFO oaep = {};
    oaep.pszAlgId = BCRYPT_SHA1_ALGORITHM;

    DWORD cipherLen = 0;
    BCryptEncrypt(hKey, plainData, plainLen, &oaep, nullptr, 0,
        nullptr, 0, &cipherLen, BCRYPT_PAD_OAEP);
    BYTE* cipher = (BYTE*)HeapAlloc(GetProcessHeap(), 0, cipherLen);

    s = BCryptEncrypt(hKey, plainData, plainLen, &oaep, nullptr, 0,
        cipher, cipherLen, &cipherLen, BCRYPT_PAD_OAEP);
    if (NT_SUCCESS(s)) {
        OutFmt(L"BCryptEncrypt(OAEP): OK, ciphertext=%u bytes", cipherLen);
        wchar_t hexBuf[128];
        BytesToHex(cipher, min(cipherLen, (DWORD)24), hexBuf, 128);
        OutFmt(L"Cipher (first 24B): %s...", hexBuf);

        // Decrypt
        BCRYPT_OAEP_PADDING_INFO oaep2 = {};
        oaep2.pszAlgId = BCRYPT_SHA1_ALGORITHM;
        DWORD plain2Len = 0;
        BCryptDecrypt(hKey, cipher, cipherLen, &oaep2, nullptr, 0,
            nullptr, 0, &plain2Len, BCRYPT_PAD_OAEP);
        BYTE* plain2 = (BYTE*)HeapAlloc(GetProcessHeap(), 0, plain2Len + 4);

        s = BCryptDecrypt(hKey, cipher, cipherLen, &oaep2, nullptr, 0,
            plain2, plain2Len, &plain2Len, BCRYPT_PAD_OAEP);
        if (NT_SUCCESS(s)) {
            BOOL match = (plain2Len == plainLen &&
                          memcmp(plain2, plainData, plainLen) == 0);
            OutFmt(L"BCryptDecrypt(OAEP): OK, plaintext=%u bytes, match=%s",
                plain2Len, match ? L"YES" : L"NO");
        } else OutFmt(L"BCryptDecrypt failed: 0x%08X", s);

        HeapFree(GetProcessHeap(), 0, plain2);
    } else {
        OutFmt(L"BCryptEncrypt(OAEP) failed: 0x%08X", s);
    }

    HeapFree(GetProcessHeap(), 0, cipher);
    HeapFree(GetProcessHeap(), 0, pubBlob);
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    StatusSet(L"RSA — BCryptGenerateKeyPair(1024) + BCryptExportKey + BCryptEncrypt/Decrypt OAEP");
}

// ── Mode 4: ECDSA Signature ───────────────────────────────────────────────────

void ModeECDSA() {
    OutClear();
    OutLine(L"=== ECDSA P-256 Digital Signature (BCrypt) ===");
    OutLine(L"");

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS s = BCryptOpenAlgorithmProvider(&hAlg,
        BCRYPT_ECDSA_P256_ALGORITHM, nullptr, 0);
    if (!NT_SUCCESS(s)) {
        OutFmt(L"BCryptOpenAlgorithmProvider(ECDSA_P256) failed: 0x%08X", s);
        return;
    }
    OutLine(L"BCryptOpenAlgorithmProvider(BCRYPT_ECDSA_P256_ALGORITHM): OK");

    BCRYPT_KEY_HANDLE hKey = nullptr;
    s = BCryptGenerateKeyPair(hAlg, &hKey, 256, 0);
    if (!NT_SUCCESS(s)) {
        OutFmt(L"BCryptGenerateKeyPair failed: 0x%08X", s);
        BCryptCloseAlgorithmProvider(hAlg, 0); return;
    }
    s = BCryptFinalizeKeyPair(hKey, 0);
    OutFmt(L"BCryptFinalizeKeyPair(P-256): %s", NT_SUCCESS(s) ? L"OK" : L"FAIL");

    // Export public key
    DWORD pubLen = 0;
    BCryptExportKey(hKey, nullptr, BCRYPT_ECCPUBLIC_BLOB, nullptr, 0, &pubLen, 0);
    BYTE* pubBlob = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pubLen);
    BCryptExportKey(hKey, nullptr, BCRYPT_ECCPUBLIC_BLOB, pubBlob, pubLen, &pubLen, 0);
    OutFmt(L"Public key blob: %u bytes (BCRYPT_ECCPUBLIC_BLOB)", pubLen);

    // Hash the input data with SHA-256
    BYTE data[1024]; DWORD dataLen = GetInputBytes(data, sizeof(data));
    BYTE digest[32]; DWORD dgLen = 32;
    BcryptHash(BCRYPT_SHA256_ALGORITHM, data, dataLen, digest, &dgLen);
    wchar_t hexBuf[128];
    BytesToHex(digest, dgLen, hexBuf, 128);
    OutFmt(L"SHA-256 of input:   %s", hexBuf);

    // Sign the hash
    DWORD sigLen = 0;
    BCryptSignHash(hKey, nullptr, digest, dgLen, nullptr, 0, &sigLen, 0);
    BYTE* sig = (BYTE*)HeapAlloc(GetProcessHeap(), 0, sigLen);
    s = BCryptSignHash(hKey, nullptr, digest, dgLen, sig, sigLen, &sigLen, 0);
    if (NT_SUCCESS(s)) {
        wchar_t sigHex[128];
        BytesToHex(sig, min(sigLen, (DWORD)32), sigHex, 128);
        OutFmt(L"BCryptSignHash: OK, sig=%u bytes", sigLen);
        OutFmt(L"Signature (first 32B): %s...", sigHex);

        // Verify signature
        s = BCryptVerifySignature(hKey, nullptr, digest, dgLen, sig, sigLen, 0);
        OutFmt(L"BCryptVerifySignature: %s", NT_SUCCESS(s) ? L"VALID" : L"INVALID");

        // Tamper the signature — should fail
        sig[0] ^= 0xFF;
        s = BCryptVerifySignature(hKey, nullptr, digest, dgLen, sig, sigLen, 0);
        OutFmt(L"BCryptVerifySignature (tampered): %s", NT_SUCCESS(s) ? L"Valid?!" : L"INVALID (correct)");
    } else {
        OutFmt(L"BCryptSignHash failed: 0x%08X", s);
    }

    HeapFree(GetProcessHeap(), 0, sig);
    HeapFree(GetProcessHeap(), 0, pubBlob);
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    StatusSet(L"ECDSA — BCryptOpenAlgorithmProvider(P256) + BCryptSignHash + BCryptVerifySignature");
}

// ── Mode 5: DPAPI ─────────────────────────────────────────────────────────────

void ModeDPAPI() {
    OutClear();
    OutLine(L"=== DPAPI — CryptProtectData + Base64 + Memory Protection ===");
    OutLine(L"");

    BYTE data[1024]; DWORD dataLen = GetInputBytes(data, sizeof(data));
    DATA_BLOB input = {dataLen, data};

    // User scope
    DATA_BLOB encrypted = {};
    if (CryptProtectData(&input, L"Win32 Demo", nullptr, nullptr, nullptr,
            CRYPTPROTECT_UI_FORBIDDEN, &encrypted)) {
        OutFmt(L"CryptProtectData (user): OK, blob=%u bytes", encrypted.cbData);

        // Base64 encode
        DWORD b64Len = 0;
        CryptBinaryToStringW(encrypted.pbData, encrypted.cbData,
            CRYPT_STRING_BASE64, nullptr, &b64Len);
        wchar_t* b64 = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, b64Len * sizeof(wchar_t));
        CryptBinaryToStringW(encrypted.pbData, encrypted.cbData,
            CRYPT_STRING_BASE64, b64, &b64Len);
        // Show first 80 chars of base64
        wchar_t preview[128] = {};
        StringCchCopyN(preview, 128, b64, min((DWORD)80, b64Len));
        OutFmt(L"Base64 (first 80 chars): %s...", preview);
        HeapFree(GetProcessHeap(), 0, b64);

        // Machine scope
        DATA_BLOB encMachine = {};
        if (CryptProtectData(&input, L"Machine", nullptr, nullptr, nullptr,
                CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_LOCAL_MACHINE, &encMachine)) {
            OutFmt(L"CryptProtectData (machine): OK, blob=%u bytes", encMachine.cbData);
            LocalFree(encMachine.pbData);
        }

        // Decrypt
        DATA_BLOB decrypted = {};
        LPWSTR desc = nullptr;
        if (CryptUnprotectData(&encrypted, &desc, nullptr, nullptr, nullptr,
                CRYPTPROTECT_UI_FORBIDDEN, &decrypted)) {
            OutFmt(L"CryptUnprotectData: OK, %u bytes", decrypted.cbData);
            if (desc) { OutFmt(L"Description: %s", desc); LocalFree(desc); }
            BOOL match = (decrypted.cbData == dataLen &&
                          memcmp(decrypted.pbData, data, dataLen) == 0);
            OutFmt(L"Round-trip match: %s", match ? L"YES" : L"NO");
            LocalFree(decrypted.pbData);
        }
        LocalFree(encrypted.pbData);
    }

    OutLine(L"");

    // CryptProtectMemory
    wchar_t memBuf[128];
    StringCchCopyW(memBuf, 128, L"Memory-protected secret value!");
    DWORD memSize = 128 * sizeof(wchar_t);
    if (CryptProtectMemory(memBuf, memSize, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
        OutLine(L"CryptProtectMemory (SAME_PROCESS): OK — buffer opaque");
        if (CryptUnprotectMemory(memBuf, memSize, CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            OutFmt(L"CryptUnprotectMemory: OK — \"%s\"", memBuf);
        }
    }

    // CryptStringToBinary (decode a known base64 string)
    const wchar_t* testB64 = L"SGVsbG8gV29ybGQ=";  // "Hello World"
    BYTE binBuf[64]; DWORD binLen = 64;
    DWORD skip = 0, flags = 0;
    if (CryptStringToBinaryW(testB64, 0, CRYPT_STRING_BASE64,
            binBuf, &binLen, &skip, &flags)) {
        wchar_t decoded[64];
        MultiByteToWideChar(CP_UTF8, 0, (char*)binBuf, binLen, decoded, 64);
        decoded[binLen] = L'\0';
        OutFmt(L"CryptStringToBinary(\"%s\"): \"%s\"", testB64, decoded);
    }

    StatusSet(L"DPAPI — CryptProtectData(user+machine) + CryptProtectMemory + Base64");
}

// ── Mode 6: Random & HMAC ─────────────────────────────────────────────────────

void ModeRandom() {
    OutClear();
    OutLine(L"=== Random Number Generation & HMAC (BCrypt) ===");
    OutLine(L"");

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, nullptr, 0);

    // Generate random bytes
    const DWORD sizes[] = {16, 32, 64};
    for (DWORD sz : sizes) {
        BYTE buf[64] = {};
        BCryptGenRandom(hAlg, buf, sz, 0);
        wchar_t hexBuf[256];
        BytesToHex(buf, sz, hexBuf, 256);
        OutFmt(L"BCryptGenRandom(%2u bytes): %s", sz, hexBuf);
    }

    // Throughput test: 10000 x 16-byte random
    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);
    BYTE rndBuf[16];
    for (int i = 0; i < 10000; i++) BCryptGenRandom(hAlg, rndBuf, 16, 0);
    QueryPerformanceCounter(&t2);
    double ms = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
    OutFmt(L"\n10000 x BCryptGenRandom(16B): %.2f ms = %.0f calls/sec",
        ms, 10000.0 / (ms / 1000.0));

    BCryptCloseAlgorithmProvider(hAlg, 0);

    OutLine(L"");
    OutLine(L"--- HMAC-SHA256 ---");

    // HMAC-SHA256
    BCRYPT_ALG_HANDLE hHmacAlg = nullptr;
    BCryptOpenAlgorithmProvider(&hHmacAlg, BCRYPT_SHA256_ALGORITHM,
        nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);

    BYTE key[32]; DWORD keyLen = 32;
    // Fixed key for reproducibility
    for (int i = 0; i < 32; i++) key[i] = (BYTE)(0xA0 + i);

    BYTE data[1024]; DWORD dataLen = GetInputBytes(data, sizeof(data));

    // Create HMAC hash
    BCRYPT_HASH_HANDLE hHmac = nullptr;
    BCryptCreateHash(hHmacAlg, &hHmac, nullptr, 0, key, keyLen, 0);
    BCryptHashData(hHmac, data, dataLen, 0);
    BYTE hmac1[32];
    BCryptFinishHash(hHmac, hmac1, 32, 0);
    BCryptDestroyHash(hHmac);

    wchar_t hexBuf[128];
    BytesToHex(hmac1, 32, hexBuf, 128);
    OutFmt(L"HMAC-SHA256 (first run): %s", hexBuf);

    // Second run — must match
    BCryptCreateHash(hHmacAlg, &hHmac, nullptr, 0, key, keyLen, 0);
    BCryptHashData(hHmac, data, dataLen, 0);
    BYTE hmac2[32];
    BCryptFinishHash(hHmac, hmac2, 32, 0);
    BCryptDestroyHash(hHmac);

    BOOL match = (memcmp(hmac1, hmac2, 32) == 0);
    OutFmt(L"HMAC-SHA256 (second run matches): %s", match ? L"YES" : L"NO");

    BCryptCloseAlgorithmProvider(hHmacAlg, 0);

    OutLine(L"");
    OutLine(L"--- HMAC-SHA512 ---");

    BCRYPT_ALG_HANDLE hHmac512 = nullptr;
    BCryptOpenAlgorithmProvider(&hHmac512, BCRYPT_SHA512_ALGORITHM,
        nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);

    BCRYPT_HASH_HANDLE hH512 = nullptr;
    BCryptCreateHash(hHmac512, &hH512, nullptr, 0, key, keyLen, 0);
    BCryptHashData(hH512, data, dataLen, 0);
    BYTE hmac512[64];
    BCryptFinishHash(hH512, hmac512, 64, 0);
    BCryptDestroyHash(hH512);

    BytesToHex(hmac512, 32, hexBuf, 128);
    OutFmt(L"HMAC-SHA512 (first 32B): %s...", hexBuf);

    BCryptCloseAlgorithmProvider(hHmac512, 0);

    StatusSet(L"Random & HMAC — BCryptGenRandom + BCryptCreateHash(HMAC_FLAG) HMAC-SHA256/512");
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"Hash Lab",  IDC_BTN_HASH},
            {L"AES",       IDC_BTN_AES},
            {L"RSA",       IDC_BTN_RSA},
            {L"ECDSA",     IDC_BTN_ECDSA},
            {L"DPAPI",     IDC_BTN_DPAPI2},
            {L"Rnd+HMAC",  IDC_BTN_RANDOM},
        };
        for (int i = 0; i < 6; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 130, 8, 120, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // Input row
        HWND hLbl = CreateWindowW(L"STATIC", L"Input:",
            WS_CHILD | WS_VISIBLE, 10, 46, 48, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_LBL_INPUT, g_hInst, nullptr);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEditIn = CreateWindowW(L"EDIT",
            L"Hello, Win32 Cryptography World!",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            62, 45, 720, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_IN, g_hInst, nullptr);
        SendMessage(g_hEditIn, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Output
        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 76, 800, 400, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_OUT, g_hInst, nullptr);
        SendMessage(g_hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Status
        g_hStatus = CreateWindowW(STATUSCLASSNAMEW, L"Ready — pick a mode",
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd,
            (HMENU)(UINT_PTR)IDC_STATUS, g_hInst, nullptr);
        int parts[] = {600, -1};
        SendMessage(g_hStatus, SB_SETPARTS, 2, (LPARAM)parts);
        SendMessage(g_hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
        return 0;
    }

    case WM_SIZE: {
        int W = LOWORD(lp), H = HIWORD(lp);
        SendMessage(g_hStatus, WM_SIZE, 0, 0);
        RECT rs; GetWindowRect(g_hStatus, &rs);
        int sh = rs.bottom - rs.top;
        int parts[] = {W - 200, -1};
        SendMessage(g_hStatus, SB_SETPARTS, 2, (LPARAM)parts);
        SetWindowPos(g_hEditIn, nullptr, 62, 45, W - 80, 22, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 76, W, H - 76 - sh, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_HASH:   ModeHash();   break;
        case IDC_BTN_AES:    ModeAES();    break;
        case IDC_BTN_RSA:    ModeRSA();    break;
        case IDC_BTN_ECDSA:  ModeECDSA();  break;
        case IDC_BTN_DPAPI2: ModeDPAPI();  break;
        case IDC_BTN_RANDOM: ModeRandom(); break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) ModeHash();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    g_hInst = hInst;
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"Crypto27Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Crypto27Class",
        L"27 — Cryptography: CNG + DPAPI + Legacy CryptoAPI Workbench",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 640,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
