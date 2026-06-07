#include <Windows.h>
#include <OleAuto.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    VARIANT value;
    VariantInit(&value);
    value.vt = VT_BSTR;
    value.bstrVal = SysAllocString(L"Automation BSTR value");

    SAFEARRAY* array = SafeArrayCreateVector(VT_I4, 0, 3);
    LONG* data{};
    SafeArrayAccessData(array, reinterpret_cast<void**>(&data));
    data[0] = 10; data[1] = 20; data[2] = 30;
    SafeArrayUnaccessData(array);

    wchar_t msg[512]{};
    StringCchPrintfW(msg, ARRAYSIZE(msg), L"19_Automation\r\nVARIANT vt: %u\r\nBSTR: %s\r\nSAFEARRAY created with 3 integers.", value.vt, value.bstrVal);
    VariantClear(&value);
    SafeArrayDestroy(array);
    MessageBoxW(nullptr, msg, L"Automation", MB_OK);
    return 0;
}
