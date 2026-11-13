#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* All paths in the configuration are relative to the launcher directory. */
#define CAP 32768
static wchar_t dir[CAP], ini[CAP], game[CAP], cert[CAP], dll[CAP], log_path[CAP];
static wchar_t server[64], account[64], cmd[CAP];

#define WM_STATUS (WM_APP + 1)
#define WM_FINISHED (WM_APP + 2)
static HWND window, fields[4], play_button, status_label;
static int headless;
static void status(const wchar_t *message) {
    if (headless) {
        fwprintf(stderr, L"%ls\n", message);
        fflush(stderr);
    } else if (window) {
        size_t bytes = (wcslen(message) + 1) * sizeof(wchar_t);
        wchar_t *copy = (wchar_t *)HeapAlloc(GetProcessHeap(), 0, bytes);
        if (copy) { memcpy(copy, message, bytes); if (!PostMessageW(window, WM_STATUS, 0, (LPARAM)copy)) HeapFree(GetProcessHeap(), 0, copy); }
    }
}
static void error(const wchar_t *stage) {
    wchar_t message[256];
    swprintf(message, 256, L"%ls failed (Windows error %lu).", stage, GetLastError());
    status(message);
}
static int full_path(const wchar_t *name, wchar_t *out) {
    wchar_t tmp[CAP]; DWORD n;
    if (name[0] == L'\\' || (name[0] && name[1] == L':'))
        n = (DWORD)swprintf(tmp, CAP, L"%ls", name);
    else n = (DWORD)swprintf(tmp, CAP, L"%ls\\%ls", dir, name);
    if (n >= CAP) return 0;
    n = GetFullPathNameW(tmp, CAP, out, NULL);
    return n && n < CAP;
}
static int config_path(const wchar_t *key, const wchar_t *fallback, wchar_t *out) {
    wchar_t value[CAP];
    DWORD n = GetPrivateProfileStringW(L"LocalServer", key, fallback, value, CAP, ini);
    return n && n < CAP - 1 && full_path(value, out);
}
static int is_file(const wchar_t *path) {
    DWORD a = GetFileAttributesW(path);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}
static int valid_account(const wchar_t *s) {
    size_t i, n = wcslen(s);
    if (!n || n > 32) return 0;
    for (i = 0; i < n; ++i) {
        wchar_t c = s[i];
        if (!((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') ||
              (c >= L'0' && c <= L'9') || c == L'_' || c == L' ' || c == L'-')) return 0;
    }
    return 1;
}
static int valid_ipv4(const wchar_t *s) {
    unsigned part = 0, digits = 0, dots = 0, nonzero = 0; const wchar_t *p;
    for (p = s; ; ++p) {
        if (*p >= L'0' && *p <= L'9') {
            if (digits && !part) return 0;
            if (++digits > 3) return 0;
            part = part * 10 + (unsigned)(*p - L'0');
            if (part > 255) return 0;
        } else if (*p == L'.' || !*p) {
            if (!digits) return 0;
            if (part) nonzero = 1;
            if (!*p) return dots == 3 && nonzero;
            if (++dots > 3) return 0;
            part = digits = 0;
        } else return 0;
    }
}
/* Quote one Windows argv element following CommandLineToArgvW/CRT rules. */
static int append_arg(wchar_t *dst, size_t cap, const wchar_t *arg) {
    size_t len = wcslen(dst), slashes = 0; const wchar_t *p;
#define PUT(c) do { if (len + 1 >= cap) return 0; dst[len++] = (c); } while (0)
    if (len) PUT(L' ');
    PUT(L'"');
    for (p = arg; *p; ++p) {
        if (*p == L'\\') { ++slashes; continue; }
        if (*p == L'"') { while (slashes--) { PUT(L'\\'); PUT(L'\\'); } PUT(L'\\'); PUT(L'"'); }
        else { while (slashes--) PUT(L'\\'); PUT(*p); }
        slashes = 0;
    }
    while (slashes--) { PUT(L'\\'); PUT(L'\\'); }
    PUT(L'"'); dst[len] = 0; return 1;
#undef PUT
}
static int remote_module(DWORD pid, const wchar_t *name, MODULEENTRY32W *result) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    MODULEENTRY32W m; int found = 0;
    if (snap == INVALID_HANDLE_VALUE) return 0;
    m.dwSize = sizeof(m);
    if (Module32FirstW(snap, &m)) do {
        if (!_wcsicmp(m.szModule, name)) { *result = m; found = 1; break; }
    } while (Module32NextW(snap, &m));
    CloseHandle(snap); return found;
}
static int inject(PROCESS_INFORMATION *pi) {
    HMODULE owner = NULL; FARPROC local; MODULEENTRY32W remote, loaded;
    wchar_t owner_path[CAP], *base; HANDLE thread = NULL; void *memory = NULL;
    SIZE_T bytes = (wcslen(dll) + 1) * sizeof(wchar_t), written = 0;
    DWORD result = 0, wait, failure = ERROR_SUCCESS; int ok = 0; wchar_t message[192];
    local = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!local || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                      (LPCWSTR)(void *)local, &owner) ||
        !GetModuleFileNameW(owner, owner_path, CAP)) goto done;
    base = wcsrchr(owner_path, L'\\'); base = base ? base + 1 : owner_path;
    {
        DWORD start = GetTickCount();
        while (!remote_module(pi->dwProcessId, base, &remote)) {
            if (WaitForSingleObject(pi->hProcess, 50) != WAIT_TIMEOUT || (DWORD)(GetTickCount() - start) >= 10000) {
                status(L"Loader module unavailable."); goto done;
            }
        }
    }
    memory = VirtualAllocEx(pi->hProcess, NULL, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!memory || !WriteProcessMemory(pi->hProcess, memory, dll, bytes, &written) || written != bytes) goto done;
    thread = CreateRemoteThread(pi->hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)((BYTE *)remote.modBaseAddr + ((BYTE *)(void *)local - (BYTE *)owner)), memory, 0, NULL);
    if (!thread) goto done;
    /* The protected shipping image can retain the loader lock through a long
     * cold start under Wine. The UI worker stays responsive while it finishes. */
    wait = WaitForSingleObject(thread, 120000);
    if (wait == WAIT_TIMEOUT) {
        failure = ERROR_TIMEOUT;
        status(L"Remote helper load timed out after 120 seconds."); goto done;
    }
    if (wait == WAIT_FAILED) {
        failure = GetLastError();
        swprintf(message, 192, L"Waiting for remote helper load failed (Windows error %lu).", failure);
        status(message); goto done;
    }
    if (wait != WAIT_OBJECT_0) {
        failure = ERROR_GEN_FAILURE;
        swprintf(message, 192, L"Unexpected remote helper wait result: %lu.", wait);
        status(message); goto done;
    }
    if (!GetExitCodeThread(thread, &result)) {
        failure = GetLastError();
        swprintf(message, 192, L"Reading remote loader result failed (Windows error %lu).", failure);
        status(message); goto done;
    }
    if (!result) {
        failure = ERROR_DLL_INIT_FAILED;
        status(L"Remote LoadLibraryW returned zero."); goto done;
    }
    if (!remote_module(pi->dwProcessId, L"FortniteLocalHelper.dll", &loaded) || _wcsicmp(loaded.szExePath, dll)) {
        failure = ERROR_DLL_INIT_FAILED;
        /* LoadLibrary's successful return is a module address, truncated to DWORD.
           Report only recognizable exception statuses, never an address. */
        if (result == 0xc0000005UL || result == 0xc000001dUL || result == 0xc0000135UL || result == 0xc0000142UL || result == 0xc0000409UL) {
            swprintf(message, 192, L"Remote loader exited with exception status 0x%08lX; helper was not verified.", result);
            status(message);
        } else status(L"Remote loader exited nonzero, but helper module could not be verified.");
        goto done;
    }
    ok = 1;
done:
    if (!ok && !failure) {
        failure = GetLastError(); if (!failure) failure = ERROR_GEN_FAILURE;
        swprintf(message, 192, L"Loading local helper failed (Windows error %lu).", failure);
        status(message);
    }
    if (thread) CloseHandle(thread);
    /* A timed-out thread may still use this buffer; child termination owns cleanup. */
    if (memory && (!thread || ok)) VirtualFreeEx(pi->hProcess, memory, 0, MEM_RELEASE);
    if (!ok) SetLastError(failure);
    return ok;
}
static int launch(void) {
    DWORD n, type, exitcode, wait, create_error; wchar_t *slash; wchar_t password[128], game_directory[CAP];
    wchar_t ready_name[128], failed_name[128], client_log[CAP], session_id[33], session_arg[64]; GUID session_guid; HANDLE ready = NULL, failed = NULL, waits[3]; BOOL created;
    STARTUPINFOW si; PROCESS_INFORMATION pi;
    static const wchar_t *args[] = { L"-nobe", L"-noeac", L"-NOEPICPORTAL", L"-epicapp=Fortnite", L"-epicenv=Prod", L"-epiclocale=en-us", L"-skippatchcheck", L"-fromfl=none", L"-AUTH_LOGIN=unused", L"-AUTH_TYPE=exchangecode" };
    size_t i;
    n = GetModuleFileNameW(NULL, dir, CAP);
    if (!n || n >= CAP || !(slash = wcsrchr(dir, L'\\'))) { error(L"Launcher path"); return 1; }
    *slash = 0;
    if (!full_path(L"local-server.ini", ini) ||
        !config_path(L"GameExecutable", L"FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping.exe", game) ||
        !config_path(L"Certificate", L"server-ca.pem", cert) || !full_path(L"FortniteLocalHelper.dll", dll) || !full_path(L"ProjectChrono-helper.log", log_path)) {
        status(L"Invalid or excessively long path in configuration."); return 1;
    }
    n = GetPrivateProfileStringW(L"LocalServer", L"ServerAddress", L"127.0.0.1", server, 64, ini);
    if (!n || n >= 63 || !valid_ipv4(server)) { status(L"Server address must be an IPv4 address."); return 1; }
    n = GetPrivateProfileStringW(L"LocalServer", L"Account", L"LocalPlayer", account, 64, ini);
    if (!n || n >= 63 || !valid_account(account)) { status(L"Account: 1–32 letters, digits, spaces, underscores or hyphens."); return 1; }
    if (!is_file(game) || !is_file(cert) || !is_file(dll)) { status(L"Game executable, public CA certificate, or helper DLL is missing."); return 1; }
    if (!GetBinaryTypeW(game, &type) || type != SCS_64BIT_BINARY) { status(L"Game executable must be a 64-bit Windows executable."); return 1; }
    wcscpy(game_directory, game);
    slash = wcsrchr(game_directory, L'\\');
    if (!slash) { status(L"Game executable directory is invalid."); return 1; }
    *slash = 0;
    if (!SetEnvironmentVariableW(L"FN_LOCAL_CA_CERT", cert) || !SetEnvironmentVariableW(L"FN_LOCAL_SERVER", server) || !SetEnvironmentVariableW(L"FN_LOCAL_ACCOUNT", account) || !SetEnvironmentVariableW(L"FN_LOCAL_HELPER_LOG", log_path)) { error(L"Setting child environment"); return 1; }
    {
        HMODULE helper; BOOL prepared = FALSE; DWORD failure = ERROR_GEN_FAILURE;
        typedef BOOL (WINAPI *prepare_fn)(LPCWSTR);
        prepare_fn prepare;
        status(L"Preparing secure connection…");
        if (!SetEnvironmentVariableW(L"FN_LOCAL_PREPARE_ONLY", L"1")) { error(L"Preparing certificate"); return 1; }
        helper = LoadLibraryW(dll);
        if (helper) {
            prepare = (prepare_fn)(void *)GetProcAddress(helper, "ProjectChronoPrepareCertificate");
            if (prepare) prepared = prepare(cert);
            failure = GetLastError();
            FreeLibrary(helper);
        } else failure = GetLastError();
        if (!SetEnvironmentVariableW(L"FN_LOCAL_PREPARE_ONLY", NULL)) { error(L"Clearing certificate preparation mode"); return 1; }
        if (!prepared) { SetLastError(failure); error(L"Preparing certificate"); return 1; }
    }
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, client_log))) {
        status(L"Could not locate the client log directory."); return 1;
    }
    if (wcslen(client_log) + wcslen(L"\\FortniteGame\\Saved\\Logs\\FortniteGame.log") >= CAP) {
        status(L"Client log path is too long."); return 1;
    }
    wcscat(client_log, L"\\FortniteGame\\Saved\\Logs\\FortniteGame.log");
    if (FAILED(CoCreateGuid(&session_guid))) { status(L"Could not create a unique session ID."); return 1; }
    swprintf(session_id, 33, L"%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
        (unsigned long)session_guid.Data1, (unsigned)session_guid.Data2, (unsigned)session_guid.Data3,
        (unsigned)session_guid.Data4[0], (unsigned)session_guid.Data4[1],
        (unsigned)session_guid.Data4[2], (unsigned)session_guid.Data4[3],
        (unsigned)session_guid.Data4[4], (unsigned)session_guid.Data4[5],
        (unsigned)session_guid.Data4[6], (unsigned)session_guid.Data4[7]);
    swprintf(session_arg, 64, L"-ProjectChronoSession=%ls", session_id);
    cmd[0] = 0;
    if (!append_arg(cmd, CAP, game)) goto command_failure;
    for (i = 0; i < sizeof(args)/sizeof(args[0]); ++i) if (!append_arg(cmd, CAP, args[i])) goto command_failure;
    /* Keep this switch unquoted so the client's command-line log includes it verbatim. */
    if (wcslen(cmd) + 1 + wcslen(session_arg) >= CAP) goto command_failure;
    wcscat(cmd, L" "); wcscat(cmd, session_arg);
    if (swprintf(password, 128, L"-AUTH_PASSWORD=local-username:%ls", account) < 0 ||
        !append_arg(cmd, CAP, password)) goto command_failure;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si); ZeroMemory(&pi, sizeof(pi));
    swprintf(ready_name, 128, L"Local\\ProjectChrono.%lu.%llu.Ready", GetCurrentProcessId(), (unsigned long long)GetTickCount64());
    /* Use the same launch identifier for both events. */
    wcscpy(failed_name, ready_name);
    wcscpy(wcsrchr(failed_name, L'.'), L".Failed");
    ready = CreateEventW(NULL, TRUE, FALSE, ready_name);
    if (!ready || GetLastError() == ERROR_ALREADY_EXISTS) { status(L"Could not create unique helper ready event."); goto event_failure; }
    failed = CreateEventW(NULL, TRUE, FALSE, failed_name);
    if (!failed || GetLastError() == ERROR_ALREADY_EXISTS) { status(L"Could not create unique helper failure event."); goto event_failure; }
    if (!SetEnvironmentVariableW(L"FN_LOCAL_READY_EVENT", ready_name) || !SetEnvironmentVariableW(L"FN_LOCAL_FAILED_EVENT", failed_name) ||
        !SetEnvironmentVariableW(L"FN_LOCAL_CLIENT_LOG", client_log) ||
        !SetEnvironmentVariableW(L"FN_LOCAL_SESSION_ID", session_id)) {
        error(L"Setting helper event environment"); goto event_failure;
    }
    status(L"Starting game process…");
    created = CreateProcessW(game, cmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, game_directory, &si, &pi);
    create_error = GetLastError();
    SetEnvironmentVariableW(L"FN_LOCAL_READY_EVENT", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_FAILED_EVENT", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_CLIENT_LOG", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_SESSION_ID", NULL);
    if (!created) { SetLastError(create_error); error(L"Starting game"); goto event_failure; }
    if (ResumeThread(pi.hThread) == (DWORD)-1) { error(L"Starting game thread"); goto child_failure; }
    status(L"Loading local helper…");
    if (!inject(&pi)) goto child_failure;
    status(L"Initializing local helper…");
    waits[0] = pi.hProcess; waits[1] = ready; waits[2] = failed;
    wait = WaitForMultipleObjects(3, waits, FALSE, 185000);
    if (wait == WAIT_OBJECT_0) {
        if (!GetExitCodeProcess(pi.hProcess, &exitcode) || !exitcode) exitcode = 1;
        status(L"Game process exited before the local helper was ready.");
        goto child_done;
    }
    if (wait != WAIT_OBJECT_0 + 1) {
        if (wait == WAIT_OBJECT_0 + 2) status(L"Local helper initialization failed.");
        else if (wait == WAIT_TIMEOUT) status(L"Local helper initialization timed out.");
        else error(L"Waiting for local helper");
        goto child_failure;
    }
    status(L"Local helper ready. Game process running.");
    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, &exitcode)) exitcode = 1;
    if (exitcode) { wchar_t message[128]; swprintf(message, 128, L"Game process exited with code %lu.", exitcode); status(message); }
    goto child_done;
child_failure:
    TerminateProcess(pi.hProcess, 1);
    WaitForSingleObject(pi.hProcess, INFINITE);
    exitcode = 1;
child_done:
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    CloseHandle(failed); CloseHandle(ready);
    return (int)exitcode;
command_failure:
    status(L"Game command line is too long.");
    return 1;
event_failure:
    SetEnvironmentVariableW(L"FN_LOCAL_READY_EVENT", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_FAILED_EVENT", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_CLIENT_LOG", NULL);
    SetEnvironmentVariableW(L"FN_LOCAL_SESSION_ID", NULL);
    if (failed) CloseHandle(failed);
    if (ready) CloseHandle(ready);
    return 1;
}

#define ID_GAME 101
#define ID_CERT 102
#define ID_SERVER 103
#define ID_ACCOUNT 104
#define ID_GAME_BROWSE 201
#define ID_CERT_BROWSE 202
#define ID_PLAY 203
#define ID_LOG 204
/* Minimal GDI+ flat C API declarations for anti-aliased cards, inputs, buttons and clock motif. */
typedef struct GpGraphics GpGraphics;
typedef struct GpPen GpPen;
typedef struct GpBrush GpBrush;
typedef struct GpSolidFill GpSolidFill;
typedef struct GpPath GpPath;
typedef DWORD ARGB;
typedef float REAL;
typedef int GpStatus;
typedef struct { UINT32 GdiplusVersion; void *DebugEventCallback; BOOL SuppressBackgroundThread; BOOL SuppressExternalCodecs; } GdiplusStartupInput;
GpStatus WINAPI GdiplusStartup(ULONG_PTR *, const GdiplusStartupInput *, void *);
void WINAPI GdiplusShutdown(ULONG_PTR);
GpStatus WINAPI GdipCreateFromHDC(HDC, GpGraphics **);
GpStatus WINAPI GdipDeleteGraphics(GpGraphics *);
GpStatus WINAPI GdipSetSmoothingMode(GpGraphics *, int);
GpStatus WINAPI GdipSetPixelOffsetMode(GpGraphics *, int);
GpStatus WINAPI GdipCreatePen1(ARGB, REAL, int, GpPen **);
GpStatus WINAPI GdipSetPenStartCap(GpPen *, int);
GpStatus WINAPI GdipSetPenEndCap(GpPen *, int);
GpStatus WINAPI GdipDeletePen(GpPen *);
GpStatus WINAPI GdipCreateSolidFill(ARGB, GpSolidFill **);
GpStatus WINAPI GdipDeleteBrush(GpBrush *);
GpStatus WINAPI GdipCreatePath(int, GpPath **);
GpStatus WINAPI GdipDeletePath(GpPath *);
GpStatus WINAPI GdipAddPathArc(GpPath *, REAL, REAL, REAL, REAL, REAL, REAL);
GpStatus WINAPI GdipClosePathFigure(GpPath *);
GpStatus WINAPI GdipFillPath(GpGraphics *, GpBrush *, GpPath *);
GpStatus WINAPI GdipDrawPath(GpGraphics *, GpPen *, GpPath *);
GpStatus WINAPI GdipDrawEllipse(GpGraphics *, GpPen *, REAL, REAL, REAL, REAL);
GpStatus WINAPI GdipFillEllipse(GpGraphics *, GpBrush *, REAL, REAL, REAL, REAL);
GpStatus WINAPI GdipDrawLine(GpGraphics *, GpPen *, REAL, REAL, REAL, REAL);
static ARGB argb(BYTE a, COLORREF c) {
    return ((ARGB)a << 24) | ((ARGB)GetRValue(c) << 16) | ((ARGB)GetGValue(c) << 8) | (ARGB)GetBValue(c);
}

static HFONT font, title_font, brand_font, section_font, label_font, small_font, button_font;
static HBRUSH background, card_background, edit_background;
static HWND browse[2], log_button, title_label, tag_label, brand_label, subtitle_label, note_label, hovered_button;
static HWND section_labels[2], field_labels[4];
static WNDPROC orig_button_proc;
static int status_kind;
static int initialize_paths(void) {
    DWORD n = GetModuleFileNameW(NULL, dir, CAP); wchar_t *slash;
    if (!n || n >= CAP || !(slash = wcsrchr(dir, L'\\'))) return 0;
    *slash = 0; return full_path(L"local-server.ini", ini);
}
static HWND control(const wchar_t *cls, const wchar_t *text, DWORD style, int x, int y, int w, int h, int id) {
    HWND hnd = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, window, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)font, TRUE); return hnd;
}
static int classify_status(const wchar_t *msg) {
    if (!msg || !*msg) return 0;
    if (wcsstr(msg, L"Local helper ready")) return 2;
    if (wcsstr(msg, L"failed") || wcsstr(msg, L"missing") || wcsstr(msg, L"must be") ||
        wcsstr(msg, L"Invalid") || wcsstr(msg, L"Could not") || wcsstr(msg, L"timed out") ||
        wcsstr(msg, L"unavailable") || wcsstr(msg, L"No helper log") || wcsstr(msg, L"exited with code") ||
        wcsstr(msg, L"1\x2013" L"32") || wcsstr(msg, L"exited before")) return 1;
    if (wcsstr(msg, L"\x2026") || wcsstr(msg, L"...") || wcsstr(msg, L"Preparing") ||
        wcsstr(msg, L"Starting") || wcsstr(msg, L"Loading") || wcsstr(msg, L"Initializing")) return 3;
    return 0;
}
static COLORREF status_dot_color(int kind) {
    if (kind == 1) return RGB(242, 139, 130);
    if (kind == 2) return RGB(102, 214, 204);
    if (kind == 3) return RGB(190, 175, 255);
    return RGB(92, 106, 135);
}
static void set_ui_status(const wchar_t *msg) {
    RECT dot = {46, 558, 62, 574};
    status_kind = classify_status(msg);
    SetWindowTextW(status_label, msg);
    InvalidateRect(window, &dot, FALSE);
    InvalidateRect(status_label, NULL, TRUE);
}
static void set_enabled(BOOL enabled) {
    int i; for (i = 0; i < 4; ++i) EnableWindow(fields[i], enabled);
    for (i = 0; i < 2; ++i) EnableWindow(browse[i], enabled);
    EnableWindow(play_button, enabled);
}
static void choose_file(int index) {
    OPENFILENAMEW ofn; wchar_t path[CAP];
    GetWindowTextW(fields[index], path, CAP);
    ZeroMemory(&ofn, sizeof(ofn)); ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = window;
    ofn.lpstrFile = path; ofn.nMaxFile = CAP; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrFilter = index ? L"PEM certificates\0*.pem;*.crt\0All files\0*.*\0\0" : L"Windows applications\0*.exe\0All files\0*.*\0\0";
    if (GetOpenFileNameW(&ofn)) SetWindowTextW(fields[index], path);
}
static int save_settings(void) {
    static const wchar_t *keys[] = {L"GameExecutable", L"Certificate", L"ServerAddress", L"Account"};
    wchar_t value[CAP]; int i;
    for (i = 0; i < 4; ++i) {
        GetWindowTextW(fields[i], value, CAP);
        if (!WritePrivateProfileStringW(L"LocalServer", keys[i], value, ini)) return 0;
    }
    return 1;
}
static DWORD WINAPI worker(LPVOID unused) {
    int result; (void)unused; result = launch();
    if (window) PostMessageW(window, WM_FINISHED, (WPARAM)result, 0);
    return 0;
}
static LRESULT CALLBACK button_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    if (message == WM_MOUSEMOVE) {
        if (hovered_button != hwnd) {
            TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
            hovered_button = hwnd;
            TrackMouseEvent(&tme);
            InvalidateRect(hwnd, NULL, FALSE);
        }
    } else if (message == WM_MOUSELEAVE) {
        if (hovered_button == hwnd) { hovered_button = NULL; InvalidateRect(hwnd, NULL, FALSE); }
    } else if (message == WM_SETCURSOR && IsWindowEnabled(hwnd)) {
        SetCursor(LoadCursorW(NULL, IDC_HAND));
        return TRUE;
    }
    return CallWindowProcW(orig_button_proc, hwnd, message, wp, lp);
}
static void gdip_round_rect(GpGraphics *g, REAL x, REAL y, REAL w, REAL h, REAL r, ARGB fill, ARGB edge, REAL stroke_w) {
    GpPath *path = NULL; GpSolidFill *brush = NULL; GpPen *pen = NULL;
    REAL d = r * 2.0f;
    if (GdipCreatePath(0, &path) != 0 || !path) return;
    GdipAddPathArc(path, x, y, d, d, 180.0f, 90.0f);
    GdipAddPathArc(path, x + w - d, y, d, d, 270.0f, 90.0f);
    GdipAddPathArc(path, x + w - d, y + h - d, d, d, 0.0f, 90.0f);
    GdipAddPathArc(path, x, y + h - d, d, d, 90.0f, 90.0f);
    GdipClosePathFigure(path);
    if ((fill >> 24) && GdipCreateSolidFill(fill, &brush) == 0 && brush) {
        GdipFillPath(g, (GpBrush *)brush, path);
        GdipDeleteBrush((GpBrush *)brush);
    }
    if ((edge >> 24) && stroke_w > 0.0f && GdipCreatePen1(edge, stroke_w, 2, &pen) == 0 && pen) {
        GdipDrawPath(g, pen, path);
        GdipDeletePen(pen);
    }
    GdipDeletePath(path);
}
static void gdip_line(GpGraphics *g, REAL x1, REAL y1, REAL x2, REAL y2, ARGB color, REAL w, int cap) {
    GpPen *pen = NULL;
    if (GdipCreatePen1(color, w, 2, &pen) == 0 && pen) {
        if (cap) { GdipSetPenStartCap(pen, cap); GdipSetPenEndCap(pen, cap); }
        GdipDrawLine(g, pen, x1, y1, x2, y2);
        GdipDeletePen(pen);
    }
}
static void gdip_circle(GpGraphics *g, REAL x, REAL y, REAL d, ARGB fill, ARGB edge, REAL stroke_w) {
    GpSolidFill *brush = NULL; GpPen *pen = NULL;
    if ((fill >> 24) && GdipCreateSolidFill(fill, &brush) == 0 && brush) {
        GdipFillEllipse(g, (GpBrush *)brush, x, y, d, d);
        GdipDeleteBrush((GpBrush *)brush);
    }
    if ((edge >> 24) && stroke_w > 0.0f && GdipCreatePen1(edge, stroke_w, 2, &pen) == 0 && pen) {
        GdipDrawEllipse(g, pen, x, y, d, d);
        GdipDeletePen(pen);
    }
}
static void hook_button(HWND btn) {
    WNDPROC prev = (WNDPROC)SetWindowLongPtrW(btn, GWLP_WNDPROC, (LONG_PTR)button_proc);
    if (!orig_button_proc) orig_button_proc = prev;
}
static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
    case WM_CREATE: {
        wchar_t value[CAP]; int i;
        const wchar_t *keys[] = {L"GameExecutable", L"Certificate", L"ServerAddress", L"Account"};
        const wchar_t *defaults[] = {L"FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping.exe", L"server-ca.pem", L"127.0.0.1", L"LocalPlayer"};
        window = hwnd;
        title_font = CreateFontW(-34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        brand_font = CreateFontW(-19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        section_font = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        label_font = CreateFontW(-13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        small_font = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        button_font = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        background = CreateSolidBrush(RGB(15, 20, 33)); card_background = CreateSolidBrush(RGB(27, 34, 51)); edit_background = CreateSolidBrush(RGB(39, 48, 68));
        brand_label = control(L"STATIC", L"PROJECTCHRONO", 0, 48, 30, 240, 26, 0); SendMessageW(brand_label, WM_SETFONT, (WPARAM)brand_font, TRUE);
        tag_label = control(L"STATIC", L"FORTNITE 12.60   /   CHAPTER 2 \x00b7 SEASON 2", 0, 48, 72, 420, 20, 0); SendMessageW(tag_label, WM_SETFONT, (WPARAM)section_font, TRUE);
        title_label = control(L"STATIC", L"Back to the island.", 0, 48, 100, 550, 46, 0); SendMessageW(title_label, WM_SETFONT, (WPARAM)title_font, TRUE);
        subtitle_label = control(L"STATIC", L"Set up your local session and make your return.", 0, 49, 154, 570, 22, 0); SendMessageW(subtitle_label, WM_SETFONT, (WPARAM)font, TRUE);
        section_labels[0] = control(L"STATIC", L"YOUR SESSION", 0, 74, 228, 300, 20, 0); SendMessageW(section_labels[0], WM_SETFONT, (WPARAM)section_font, TRUE);
        field_labels[0] = control(L"STATIC", L"Server IPv4 address", 0, 74, 266, 320, 20, 0); SendMessageW(field_labels[0], WM_SETFONT, (WPARAM)label_font, TRUE);
        fields[2] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 86, 299, 340, 21, ID_SERVER);
        field_labels[1] = control(L"STATIC", L"Account name", 0, 74, 348, 320, 20, 0); SendMessageW(field_labels[1], WM_SETFONT, (WPARAM)label_font, TRUE);
        fields[3] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 86, 381, 340, 21, ID_ACCOUNT);
        section_labels[1] = control(L"STATIC", L"GAME FILES", 0, 518, 228, 300, 20, 0); SendMessageW(section_labels[1], WM_SETFONT, (WPARAM)section_font, TRUE);
        field_labels[2] = control(L"STATIC", L"Game executable", 0, 518, 266, 320, 20, 0); SendMessageW(field_labels[2], WM_SETFONT, (WPARAM)label_font, TRUE);
        fields[0] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 530, 299, 264, 21, ID_GAME);
        browse[0] = control(L"BUTTON", L"Browse", WS_TABSTOP | BS_OWNERDRAW, 814, 290, 92, 39, ID_GAME_BROWSE); hook_button(browse[0]);
        field_labels[3] = control(L"STATIC", L"Public CA certificate", 0, 518, 348, 320, 20, 0); SendMessageW(field_labels[3], WM_SETFONT, (WPARAM)label_font, TRUE);
        fields[1] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 530, 381, 264, 21, ID_CERT);
        browse[1] = control(L"BUTTON", L"Browse", WS_TABSTOP | BS_OWNERDRAW, 814, 372, 92, 39, ID_CERT_BROWSE); hook_button(browse[1]);
        for (i = 0; i < 4; ++i) { GetPrivateProfileStringW(L"LocalServer", keys[i], defaults[i], value, CAP, ini); SetWindowTextW(fields[i], value); }
        SendMessageW(fields[2], EM_SETLIMITTEXT, 63, 0); SendMessageW(fields[3], EM_SETLIMITTEXT, 32, 0);
        play_button = control(L"BUTTON", L"Play  \x2192", WS_TABSTOP | BS_OWNERDRAW, 48, 483, 190, 50, ID_PLAY); hook_button(play_button);
        log_button = control(L"BUTTON", L"Open log", WS_TABSTOP | BS_OWNERDRAW, 250, 483, 120, 50, ID_LOG); hook_button(log_button);
        status_label = control(L"STATIC", L"Ready to configure your local session.", 0, 64, 553, 868, 24, 0);
        note_label = control(L"STATIC", L"On Play, the selected public CA is trusted for this Windows account.", 0, 48, 610, 884, 22, 0); SendMessageW(note_label, WM_SETFONT, (WPARAM)small_font, TRUE);
        return 0;
    }
    case WM_COMMAND:
        if (HIWORD(wp) == EN_SETFOCUS || HIWORD(wp) == EN_KILLFOCUS) {
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        switch (LOWORD(wp)) {
        case ID_GAME_BROWSE: choose_file(0); return 0;
        case ID_CERT_BROWSE: choose_file(1); return 0;
        case ID_LOG: {
            wchar_t path[CAP];
            if (!full_path(L"ProjectChrono-helper.log", path) || !is_file(path) || (INT_PTR)ShellExecuteW(hwnd, L"open", path, NULL, dir, SW_SHOWNORMAL) <= 32)
                set_ui_status(L"No helper log is available yet.");
            return 0;
        }
        case ID_PLAY: {
            HANDLE thread;
            if (!IsWindowEnabled(play_button)) return 0;
            if (!save_settings()) { set_ui_status(L"Could not save settings beside the launcher."); return 0; }
            set_enabled(FALSE); set_ui_status(L"Preparing local session\x2026");
            thread = CreateThread(NULL, 0, worker, NULL, 0, NULL);
            if (thread) CloseHandle(thread); else { set_enabled(TRUE); set_ui_status(L"Could not start launcher worker."); }
            return 0;
        }} break;
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lp;
        BOOL primary = draw->CtlID == ID_PLAY;
        BOOL disabled = (draw->itemState & ODS_DISABLED) != 0;
        BOOL hovered = !disabled && hovered_button == draw->hwndItem;
        BOOL pressed = !disabled && (draw->itemState & ODS_SELECTED) != 0;
        BOOL in_card = draw->CtlID == ID_GAME_BROWSE || draw->CtlID == ID_CERT_BROWSE;
        COLORREF bg = in_card ? RGB(27, 34, 51) : RGB(15, 20, 33);
        COLORREF fill = disabled ? RGB(49, 53, 68)
            : primary ? (pressed ? RGB(105, 79, 196) : hovered ? RGB(141, 115, 234) : RGB(126, 98, 224))
                      : (pressed ? RGB(60, 72, 96) : hovered ? RGB(58, 70, 95) : RGB(48, 58, 80));
        COLORREF edge = (draw->itemState & ODS_FOCUS) ? RGB(102, 221, 208)
            : primary ? fill : disabled ? fill : RGB(62, 75, 102);
        RECT r = draw->rcItem;
        int w = r.right - r.left, h = r.bottom - r.top;
        HDC mem = CreateCompatibleDC(draw->hDC);
        HBITMAP bmp = CreateCompatibleBitmap(draw->hDC, w, h);
        HGDIOBJ old = SelectObject(mem, bmp);
        HBRUSH bg_brush = CreateSolidBrush(bg);
        RECT mr = {0, 0, w, h};
        GpGraphics *g = NULL;
        wchar_t label[80];
        FillRect(mem, &mr, bg_brush); DeleteObject(bg_brush);
        if (GdipCreateFromHDC(mem, &g) == 0 && g) {
            GdipSetSmoothingMode(g, 4);
            gdip_round_rect(g, 0.5f, 0.5f, (REAL)(w - 1), (REAL)(h - 1), in_card ? 10.0f : 12.0f, argb(255, fill), argb(255, edge), 1.0f);
            GdipDeleteGraphics(g);
        }
        GetWindowTextW(draw->hwndItem, label, 80);
        SetBkMode(mem, TRANSPARENT);
        SetTextColor(mem, disabled ? RGB(148, 153, 170) : primary ? RGB(255, 255, 255) : RGB(242, 245, 252));
        SelectObject(mem, button_font);
        DrawTextW(mem, label, -1, &mr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        BitBlt(draw->hDC, r.left, r.top, w, h, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem);
        return TRUE;
    }
    case WM_STATUS: set_ui_status((wchar_t *)lp); HeapFree(GetProcessHeap(), 0, (void *)lp); return 0;
    case WM_FINISHED: if (!wp) set_ui_status(L"Game process exited."); set_enabled(TRUE); return 0;
    case WM_CTLCOLORSTATIC: {
        HWND child = (HWND)lp; HDC dc = (HDC)wp;
        BOOL on_background;
        if (child == fields[0] || child == fields[1] || child == fields[2] || child == fields[3]) {
            SetTextColor(dc, RGB(148, 153, 170)); SetBkColor(dc, RGB(39, 48, 68)); return (LRESULT)edit_background;
        }
        on_background = child == title_label || child == tag_label || child == brand_label || child == subtitle_label || child == status_label || child == note_label;
        if (child == tag_label) SetTextColor(dc, RGB(102, 214, 204));
        else if (child == brand_label) SetTextColor(dc, RGB(190, 175, 255));
        else if (child == title_label) SetTextColor(dc, RGB(238, 242, 251));
        else if (child == subtitle_label) SetTextColor(dc, RGB(152, 164, 187));
        else if (child == note_label) SetTextColor(dc, RGB(139, 152, 178));
        else if (child == section_labels[0] || child == section_labels[1]) SetTextColor(dc, RGB(143, 159, 192));
        else if (child == field_labels[0] || child == field_labels[1] || child == field_labels[2] || child == field_labels[3]) SetTextColor(dc, RGB(212, 220, 237));
        else if (child == status_label) {
            SetTextColor(dc, status_kind == 1 ? RGB(242, 139, 130) : status_kind == 2 ? RGB(102, 214, 204) : RGB(232, 237, 247));
        } else SetTextColor(dc, RGB(232, 237, 247));
        SetBkColor(dc, on_background ? RGB(15, 20, 33) : RGB(27, 34, 51));
        return (LRESULT)(on_background ? background : card_background);
    }
    case WM_CTLCOLOREDIT: SetTextColor((HDC)wp, RGB(240, 243, 250)); SetBkColor((HDC)wp, RGB(39, 48, 68)); return (LRESULT)edit_background;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps); RECT r;
        HDC mem; HBITMAP bmp; HGDIOBJ old; GpGraphics *g = NULL; HWND focused;
        GetClientRect(hwnd, &r);
        mem = CreateCompatibleDC(dc);
        bmp = CreateCompatibleBitmap(dc, r.right, r.bottom);
        old = SelectObject(mem, bmp);
        FillRect(mem, &r, background);
        if (GdipCreateFromHDC(mem, &g) == 0 && g) {
            ARGB input_fill = argb(255, RGB(39, 48, 68));
            ARGB input_edge = argb(255, RGB(52, 64, 88));
            ARGB focus_edge = argb(255, RGB(102, 221, 208));
            focused = GetFocus();
            GdipSetSmoothingMode(g, 4);
            GdipSetPixelOffsetMode(g, 4);
            /* Subtle ambient clock motif in the header. */
            gdip_circle(g, 720.5f, -64.5f, 300.0f, argb(14, RGB(126, 98, 224)), argb(255, RGB(51, 59, 86)), 1.0f);
            gdip_circle(g, 747.5f, -37.5f, 246.0f, 0, argb(255, RGB(51, 59, 86)), 1.0f);
            gdip_circle(g, 812.5f, 27.5f, 116.0f, 0, argb(255, RGB(58, 68, 98)), 1.0f);
            gdip_line(g, 870.5f, 85.5f, 870.5f, 43.5f, argb(255, RGB(102, 214, 204)), 1.8f, 2);
            gdip_line(g, 870.5f, 85.5f, 902.5f, 102.5f, argb(255, RGB(140, 117, 216)), 1.8f, 2);
            gdip_circle(g, 868.0f, 83.0f, 5.0f, argb(255, RGB(190, 175, 255)), 0, 0.0f);
            /* Cards and input boxes. */
            gdip_round_rect(g, 48.5f, 205.5f, 416.0f, 245.0f, 12.0f, argb(255, RGB(27, 34, 51)), argb(255, RGB(44, 54, 75)), 1.0f);
            gdip_round_rect(g, 492.5f, 205.5f, 440.0f, 245.0f, 12.0f, argb(255, RGB(27, 34, 51)), argb(255, RGB(44, 54, 75)), 1.0f);
            gdip_round_rect(g, 74.5f, 290.5f, 364.0f, 39.0f, 9.0f, input_fill, focused == fields[2] ? focus_edge : input_edge, 1.0f);
            gdip_round_rect(g, 74.5f, 372.5f, 364.0f, 39.0f, 9.0f, input_fill, focused == fields[3] ? focus_edge : input_edge, 1.0f);
            gdip_round_rect(g, 518.5f, 290.5f, 288.0f, 39.0f, 9.0f, input_fill, focused == fields[0] ? focus_edge : input_edge, 1.0f);
            gdip_round_rect(g, 518.5f, 372.5f, 288.0f, 39.0f, 9.0f, input_fill, focused == fields[1] ? focus_edge : input_edge, 1.0f);
            /* Status indicator dot and footer rule. */
            gdip_circle(g, 49.0f, 560.0f, 8.0f, argb(255, status_dot_color(status_kind)), 0, 0.0f);
            gdip_line(g, 48.0f, 594.5f, 932.0f, 594.5f, argb(255, RGB(42, 51, 70)), 1.0f, 0);
            GdipDeleteGraphics(g);
        }
        BitBlt(dc, 0, 0, r.right, r.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem);
        EndPaint(hwnd, &ps); return 0;
    }
    case WM_DESTROY: window = NULL; PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command, int show) {
    WNDCLASSW wc; MSG message; RECT size = {0, 0, 980, 650};
    GdiplusStartupInput gdip_in = {1, NULL, FALSE, FALSE}; ULONG_PTR gdip_token = 0; (void)previous;
    if (!initialize_paths()) return 1;
    if (command && !_wcsicmp(command, L"--headless")) { headless = 1; return launch(); }
    GdiplusStartup(&gdip_token, &gdip_in, NULL);
    ZeroMemory(&wc, sizeof(wc)); wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = L"ProjectChronoWindow"; wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) { GdiplusShutdown(gdip_token); return 1; }
    AdjustWindowRect(&size, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    window = CreateWindowExW(0, wc.lpszClassName, L"ProjectChrono  |  12.60", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top, NULL, NULL, instance, NULL);
    if (!window) { GdiplusShutdown(gdip_token); return 1; }
    ShowWindow(window, show); UpdateWindow(window);
    while (GetMessageW(&message, NULL, 0, 0) > 0) { if (!IsDialogMessageW(window, &message)) { TranslateMessage(&message); DispatchMessageW(&message); } }
    DeleteObject(font); DeleteObject(title_font); DeleteObject(brand_font); DeleteObject(section_font); DeleteObject(label_font); DeleteObject(small_font); DeleteObject(button_font);
    DeleteObject(background); DeleteObject(card_background); DeleteObject(edit_background);
    GdiplusShutdown(gdip_token);
    return 0;
}
