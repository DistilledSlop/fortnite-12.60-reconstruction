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
    if (headless) fwprintf(stderr, L"%ls\n", message);
    else if (window) {
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
static HFONT font, title_font, brand_font, small_font, button_font;
static HBRUSH background, card_background, edit_background;
static HWND browse[2], title_label, tag_label, brand_label, subtitle_label, note_label;
static int initialize_paths(void) {
    DWORD n = GetModuleFileNameW(NULL, dir, CAP); wchar_t *slash;
    if (!n || n >= CAP || !(slash = wcsrchr(dir, L'\\'))) return 0;
    *slash = 0; return full_path(L"local-server.ini", ini);
}
static HWND control(const wchar_t *cls, const wchar_t *text, DWORD style, int x, int y, int w, int h, int id) {
    HWND hnd = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, window, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(hnd, WM_SETFONT, (WPARAM)font, TRUE); return hnd;
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
static void rounded(HDC dc, int x, int y, int w, int h, int radius, COLORREF fill, COLORREF edge) {
    HBRUSH brush = CreateSolidBrush(fill); HPEN pen = CreatePen(PS_SOLID, 1, edge);
    HGDIOBJ oldbrush = SelectObject(dc, brush), oldpen = SelectObject(dc, pen);
    RoundRect(dc, x, y, x + w, y + h, radius, radius);
    SelectObject(dc, oldbrush); SelectObject(dc, oldpen); DeleteObject(brush); DeleteObject(pen);
}
static void line(HDC dc, int x1, int y1, int x2, int y2, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 1, color); HGDIOBJ old = SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, NULL); LineTo(dc, x2, y2); SelectObject(dc, old); DeleteObject(pen);
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
        font = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        small_font = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        button_font = CreateFontW(-15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        background = CreateSolidBrush(RGB(15, 20, 33)); card_background = CreateSolidBrush(RGB(27, 34, 51)); edit_background = CreateSolidBrush(RGB(39, 48, 68));
        brand_label = control(L"STATIC", L"PROJECTCHRONO", 0, 48, 32, 240, 26, 0); SendMessageW(brand_label, WM_SETFONT, (WPARAM)brand_font, TRUE);
        tag_label = control(L"STATIC", L"FORTNITE 12.60   /   CHAPTER 2  SEASON 2", 0, 48, 75, 420, 22, 0); SendMessageW(tag_label, WM_SETFONT, (WPARAM)small_font, TRUE);
        title_label = control(L"STATIC", L"Back to the island.", 0, 48, 105, 550, 48, 0); SendMessageW(title_label, WM_SETFONT, (WPARAM)title_font, TRUE);
        subtitle_label = control(L"STATIC", L"Set up your local session and make your return.", 0, 50, 157, 570, 24, 0);
        control(L"STATIC", L"YOUR SESSION", 0, 74, 229, 300, 24, 0);
        control(L"STATIC", L"Server IPv4 address", 0, 74, 270, 320, 22, 0);
        fields[2] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 84, 300, 354, 23, ID_SERVER);
        control(L"STATIC", L"Account name", 0, 74, 351, 320, 22, 0);
        fields[3] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 84, 381, 354, 23, ID_ACCOUNT);
        control(L"STATIC", L"GAME FILES", 0, 518, 229, 300, 24, 0);
        control(L"STATIC", L"Game executable", 0, 518, 270, 320, 22, 0);
        fields[0] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 528, 300, 257, 23, ID_GAME);
        browse[0] = control(L"BUTTON", L"Browse", WS_TABSTOP | BS_OWNERDRAW, 800, 292, 92, 39, ID_GAME_BROWSE);
        control(L"STATIC", L"Public CA certificate", 0, 518, 351, 320, 22, 0);
        fields[1] = control(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 528, 381, 257, 23, ID_CERT);
        browse[1] = control(L"BUTTON", L"Browse", WS_TABSTOP | BS_OWNERDRAW, 800, 373, 92, 39, ID_CERT_BROWSE);
        for (i = 0; i < 4; ++i) { GetPrivateProfileStringW(L"LocalServer", keys[i], defaults[i], value, CAP, ini); SetWindowTextW(fields[i], value); }
        SendMessageW(fields[2], EM_SETLIMITTEXT, 63, 0); SendMessageW(fields[3], EM_SETLIMITTEXT, 32, 0);
        play_button = control(L"BUTTON", L"Play  \x2192", WS_TABSTOP | BS_OWNERDRAW, 48, 483, 190, 52, ID_PLAY);
        control(L"BUTTON", L"Open log", WS_TABSTOP | BS_OWNERDRAW, 249, 483, 120, 52, ID_LOG);
        status_label = control(L"STATIC", L"Ready to configure your local session.", 0, 48, 557, 850, 38, 0);
        note_label = control(L"STATIC", L"On Play, the selected public CA is trusted for this Windows account.", 0, 48, 608, 850, 24, 0); SendMessageW(note_label, WM_SETFONT, (WPARAM)small_font, TRUE);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_GAME_BROWSE: choose_file(0); return 0;
        case ID_CERT_BROWSE: choose_file(1); return 0;
        case ID_LOG: {
            wchar_t path[CAP];
            if (!full_path(L"ProjectChrono-helper.log", path) || !is_file(path) || (INT_PTR)ShellExecuteW(hwnd, L"open", path, NULL, dir, SW_SHOWNORMAL) <= 32)
                SetWindowTextW(status_label, L"No helper log is available yet.");
            return 0;
        }
        case ID_PLAY: {
            HANDLE thread;
            if (!IsWindowEnabled(play_button)) return 0;
            if (!save_settings()) { SetWindowTextW(status_label, L"Could not save settings beside the launcher."); return 0; }
            set_enabled(FALSE); SetWindowTextW(status_label, L"Preparing local session…");
            thread = CreateThread(NULL, 0, worker, NULL, 0, NULL);
            if (thread) CloseHandle(thread); else { set_enabled(TRUE); SetWindowTextW(status_label, L"Could not start launcher worker."); }
            return 0;
        }} break;
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lp;
        BOOL primary = draw->CtlID == ID_PLAY;
        BOOL disabled = (draw->itemState & ODS_DISABLED) != 0;
        COLORREF fill = disabled ? RGB(49, 53, 68) : primary ? RGB(126, 98, 224) : RGB(48, 58, 80);
        COLORREF edge = (draw->itemState & ODS_FOCUS) ? RGB(102, 221, 208) : fill;
        wchar_t label[80]; RECT r = draw->rcItem;
        FillRect(draw->hDC, &r, (draw->CtlID == ID_GAME_BROWSE || draw->CtlID == ID_CERT_BROWSE) ? card_background : background);
        if (draw->itemState & ODS_SELECTED) fill = primary ? RGB(105, 79, 196) : RGB(60, 72, 96);
        rounded(draw->hDC, r.left, r.top, r.right - r.left, r.bottom - r.top, 12, fill, edge);
        GetWindowTextW(draw->hwndItem, label, 80);
        SetBkMode(draw->hDC, TRANSPARENT); SetTextColor(draw->hDC, disabled ? RGB(148, 153, 170) : RGB(246, 247, 252));
        SelectObject(draw->hDC, button_font); DrawTextW(draw->hDC, label, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return TRUE;
    }
    case WM_STATUS: SetWindowTextW(status_label, (wchar_t *)lp); HeapFree(GetProcessHeap(), 0, (void *)lp); return 0;
    case WM_FINISHED: if (!wp) SetWindowTextW(status_label, L"Game process exited."); set_enabled(TRUE); return 0;
    case WM_CTLCOLORSTATIC: {
        HWND child = (HWND)lp; HDC dc = (HDC)wp;
        BOOL on_background;
        if (child == fields[0] || child == fields[1] || child == fields[2] || child == fields[3]) {
            SetTextColor(dc, RGB(148, 153, 170)); SetBkColor(dc, RGB(39, 48, 68)); return (LRESULT)edit_background;
        }
        on_background = child == title_label || child == tag_label || child == brand_label || child == subtitle_label || child == status_label || child == note_label;
        SetTextColor(dc, child == tag_label ? RGB(102, 214, 204) : child == subtitle_label || child == note_label ? RGB(152, 164, 187) : child == brand_label ? RGB(190, 175, 255) : RGB(232, 237, 247));
        SetBkColor(dc, on_background ? RGB(15, 20, 33) : RGB(27, 34, 51)); return (LRESULT)(on_background ? background : card_background);
    }
    case WM_CTLCOLOREDIT: SetTextColor((HDC)wp, RGB(240, 243, 250)); SetBkColor((HDC)wp, RGB(39, 48, 68)); return (LRESULT)edit_background;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps); RECT r; HPEN pen; HGDIOBJ oldpen, oldbrush;
        GetClientRect(hwnd, &r); FillRect(dc, &r, background);
        /* A quiet clock motif gives the header a sense of place. */
        pen = CreatePen(PS_SOLID, 1, RGB(51, 59, 86)); oldpen = SelectObject(dc, pen); oldbrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        Ellipse(dc, 720, -65, 1020, 235); Ellipse(dc, 747, -38, 993, 208); Ellipse(dc, 812, 27, 928, 143);
        SelectObject(dc, oldbrush); SelectObject(dc, oldpen); DeleteObject(pen);
        line(dc, 870, 85, 870, 43, RGB(102, 214, 204)); line(dc, 870, 85, 902, 102, RGB(140, 117, 216));
        rounded(dc, 48, 205, 416, 245, 18, RGB(27, 34, 51), RGB(44, 54, 75));
        rounded(dc, 492, 205, 440, 245, 18, RGB(27, 34, 51), RGB(44, 54, 75));
        rounded(dc, 74, 292, 374, 39, 9, RGB(39, 48, 68), RGB(52, 64, 88));
        rounded(dc, 74, 373, 374, 39, 9, RGB(39, 48, 68), RGB(52, 64, 88));
        rounded(dc, 518, 292, 277, 39, 9, RGB(39, 48, 68), RGB(52, 64, 88));
        rounded(dc, 518, 373, 277, 39, 9, RGB(39, 48, 68), RGB(52, 64, 88));
        line(dc, 48, 590, 932, 590, RGB(42, 51, 70));
        EndPaint(hwnd, &ps); return 0;
    }
    case WM_DESTROY: window = NULL; PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command, int show) {
    WNDCLASSW wc; MSG message; RECT size = {0, 0, 980, 650}; (void)previous;
    if (!initialize_paths()) return 1;
    if (command && !_wcsicmp(command, L"--headless")) { headless = 1; return launch(); }
    ZeroMemory(&wc, sizeof(wc)); wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = L"ProjectChronoWindow"; wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) return 1;
    AdjustWindowRect(&size, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    window = CreateWindowExW(0, wc.lpszClassName, L"ProjectChrono  |  12.60", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top, NULL, NULL, instance, NULL);
    if (!window) return 1;
    ShowWindow(window, show); UpdateWindow(window);
    while (GetMessageW(&message, NULL, 0, 0) > 0) { if (!IsDialogMessageW(window, &message)) { TranslateMessage(&message); DispatchMessageW(&message); } }
    DeleteObject(font); DeleteObject(title_font); DeleteObject(brand_font); DeleteObject(small_font); DeleteObject(button_font); DeleteObject(background); DeleteObject(card_background); DeleteObject(edit_background);
    return 0;
}
