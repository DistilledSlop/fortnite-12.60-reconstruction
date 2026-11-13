/* Exercise the production reader against real shared Windows files. */
#include "../helper.c"

static const char token[] = "-ProjectChronoSession=0123456789ABCDEF0123456789ABCDEF";
static const char other_token[] = "-ProjectChronoSession=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char frontend[] = "LogLoad: LoadMap: /Game/Maps/Frontend";
static WCHAR path[MAX_PATH];
static int failures;

static void check(int condition, const char *label) {
    if (!condition) { fprintf(stderr, "FAIL: %s (Windows error %lu)\n", label, GetLastError()); ++failures; }
}
static int write_bytes(const void *bytes, DWORD length, DWORD disposition) {
    DWORD written = 0;
    HANDLE file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    int ok;
    if (file == INVALID_HANDLE_VALUE) return 0;
    if (disposition == OPEN_EXISTING && SetFilePointer(file, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        CloseHandle(file); return 0;
    }
    ok = WriteFile(file, bytes, length, &written, NULL) && written == length;
    CloseHandle(file); return ok;
}
static int write_text(const char *text, DWORD disposition) {
    return write_bytes(text, (DWORD)strlen(text), disposition);
}
static void close_reader(FrontendLog *reader) {
    if (reader->file != INVALID_HANDLE_VALUE) CloseHandle(reader->file);
}
static int poll(FrontendLog *reader) { return frontend_in_log(reader, path, token); }

int wmain(void) {
    WCHAR temp[MAX_PATH]; DWORD n = GetTempPathW(MAX_PATH, temp);
    check(n && n < MAX_PATH && GetTempFileNameW(temp, L"pct", 0, path), "create temporary path");
    if (failures) return 1;
    {
        FrontendLog reader = {.file = INVALID_HANDLE_VALUE};
        check(DeleteFileW(path), "remove initial file");
        check(poll(&reader) == 0, "missing log stays pending");
        close_reader(&reader);
    }
    {
        FrontendLog reader = {.file = INVALID_HANDLE_VALUE};
        check(write_text(frontend, CREATE_ALWAYS), "write stale frontend");
        check(poll(&reader) == 0, "stale frontend without token rejected");
        check(write_text(token, OPEN_EXISTING), "append token after stale frontend");
        check(poll(&reader) == 0, "frontend before token rejected in same scan");
        check(write_text(frontend, OPEN_EXISTING), "append current frontend");
        check(poll(&reader) == 1, "frontend after token accepted");
        close_reader(&reader);
    }
    {
        FrontendLog reader = {.file = INVALID_HANDLE_VALUE};
        char *data = (char *)malloc(131072 + sizeof(frontend)); size_t pos;
        check(data != NULL, "allocate boundary fixture");
        if (data) {
            memset(data, 'x', 131072 + sizeof(frontend));
            pos = 65536 - strlen(token) / 2; memcpy(data + pos, token, strlen(token));
            pos = 131072 - strlen(frontend) / 2; memcpy(data + pos, frontend, strlen(frontend));
            check(write_bytes(data, (DWORD)(pos + strlen(frontend)), CREATE_ALWAYS), "write split markers");
            check(poll(&reader) == 1, "token and frontend across read boundaries accepted");
            free(data);
        }
        close_reader(&reader);
    }
    {
        FrontendLog reader = {.file = INVALID_HANDLE_VALUE};
        check(write_text(frontend, CREATE_ALWAYS), "write old log before replacement");
        check(poll(&reader) == 0, "old log pending before replacement");
        check(DeleteFileW(path), "remove old log");
        check(write_text(token, CREATE_ALWAYS) && write_text(frontend, OPEN_EXISTING), "create replacement log");
        check(poll(&reader) == 1, "replacement before token is read");
        close_reader(&reader);
    }
    {
        FrontendLog reader = {.file = INVALID_HANDLE_VALUE};
        check(write_text(token, CREATE_ALWAYS), "write current token");
        check(poll(&reader) == 0, "current token alone stays pending");
        check(write_text(other_token, CREATE_ALWAYS) && write_text(frontend, OPEN_EXISTING), "truncate and regrow different generation");
        check(poll(&reader) == 0, "different generation frontend rejected");
        check(write_text(token, OPEN_EXISTING) && write_text(frontend, OPEN_EXISTING), "append current generation");
        check(poll(&reader) == 1, "current generation recovered after truncation");
        close_reader(&reader);
    }
    check(DeleteFileW(path), "delete temporary log");
    if (!failures) puts("frontend log tests passed");
    return failures ? 1 : 0;
}
