#include "io_compat.h"
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>

// Internal structure to hold search state for each handle.
struct find_handle_t {
    DIR* dir;
    std::string pattern;
    std::string directory;
};

// We need a way to map long handles to our internal structure.
// A simple vector works for this project's needs.
// Note: This is a simple implementation and assumes _findclose is always called.
static std::vector<find_handle_t*> g_find_handles;

static long add_handle(find_handle_t* h) {
    auto it = std::find(g_find_handles.begin(), g_find_handles.end(), nullptr);
    if (it != g_find_handles.end()) {
        *it = h;
        return std::distance(g_find_handles.begin(), it) + 1;
    }
    g_find_handles.push_back(h);
    return g_find_handles.size();
}

static find_handle_t* get_handle(long l) {
    if (l > 0 && (size_t)l <= g_find_handles.size()) {
        return g_find_handles[l - 1];
    }
    return nullptr;
}

static void remove_handle(long l) {
    if (l > 0 && (size_t)l <= g_find_handles.size()) {
        delete g_find_handles[l - 1];
        g_find_handles[l - 1] = nullptr;
    }
}

extern "C" {

long _findfirst(const char *filespec, struct _finddata_t *fileinfo) {
    if (!filespec || !fileinfo) {
        return -1L;
    }

    // RERUN: Convert backslashes to forward slashes for Linux compatibility
    std::string spec = filespec;
    std::replace(spec.begin(), spec.end(), '\\', '/');

    const char* last_slash = strrchr(spec.c_str(), '/');
    std::string dir_path;
    std::string pattern;

    if (last_slash) {
        size_t pos = last_slash - spec.c_str();
        dir_path = spec.substr(0, pos);
        pattern = spec.substr(pos + 1);
    } else {
        dir_path = ".";
        pattern = spec;
    }
    
    if (dir_path.empty()) {
        dir_path = ".";
    }

    DIR* dir = opendir(dir_path.c_str());
    if (!dir) {
        return -1L;
    }

    find_handle_t* handle_info = new find_handle_t;
    handle_info->dir = dir;
    handle_info->pattern = pattern;
    handle_info->directory = dir_path;

    long handle = add_handle(handle_info);

    if (_findnext(handle, fileinfo) == 0) {
        return handle;
    }

    _findclose(handle);
    return -1L;
}

int _findnext(long handle, struct _finddata_t *fileinfo) {
    find_handle_t* handle_info = get_handle(handle);
    if (!handle_info || !fileinfo) {
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(handle_info->dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (fnmatch(handle_info->pattern.c_str(), entry->d_name, FNM_CASEFOLD) == 0) {
            strncpy(fileinfo->name, entry->d_name, _MAX_PATH);
            fileinfo->name[_MAX_PATH - 1] = '\0';

            std::string full_path = handle_info->directory + "/" + entry->d_name;
            struct stat st;
            if (stat(full_path.c_str(), &st) == 0) {
                fileinfo->attrib = (S_ISDIR(st.st_mode) ? _A_SUBDIR : 0) | (!(st.st_mode & S_IWUSR) ? _A_RDONLY : 0) | (entry->d_name[0] == '.' ? _A_HIDDEN : 0);
                if (fileinfo->attrib == 0) fileinfo->attrib = _A_NORMAL;
                fileinfo->size = st.st_size;
                fileinfo->time_create = st.st_ctime; // Best available approximation for creation time
                fileinfo->time_access = st.st_atime;
                fileinfo->time_write = st.st_mtime;
            }
            return 0; // Success
        }
    }

    return -1; // No more files
}

int _findclose(long handle) {
    find_handle_t* handle_info = get_handle(handle);
    if (!handle_info) {
        return -1;
    }
    closedir(handle_info->dir);
    remove_handle(handle);
    return 0;
}

// Case-insensitive fopen. Windows file APIs are case-insensitive; the game
// data on disk is a mix of cases (PILOT1.X8, b26eng.x8, ...) while stored
// names come back as e.g. "imagemap/PILOT1.x8". On failure we walk each
// path component and match it case-insensitively in its directory.
FILE* fopen_ci(const char* path, const char* mode) {
    if (!path || !*path) {
        return nullptr; // fopen("") fails; an empty path must not resolve to "."
    }
    FILE* f = fopen(path, mode);
    if (f || !strchr(mode, 'r')) {
        return f; // literal hit, or a write-mode failure we shouldn't fix
    }

    // Resolve component-by-component under a case-insensitive match.
    std::string resolved;
    std::string input = path;
    std::replace(input.begin(), input.end(), '\\', '/');

    size_t pos = 0;
    if (!input.empty() && input[0] == '/') {
        resolved = "/";
        pos = 1;
    } else {
        resolved = ".";
    }

    while (pos < input.size()) {
        size_t slash = input.find('/', pos);
        std::string comp = (slash == std::string::npos)
            ? input.substr(pos)
            : input.substr(pos, slash - pos);
        pos = (slash == std::string::npos) ? input.size() : slash + 1;
        if (comp.empty() || comp == ".") {
            if (comp == ".") resolved += "/.";
            continue;
        }

        DIR* dir = opendir(resolved.c_str());
        if (!dir) {
            return nullptr;
        }
        std::string found;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcasecmp(comp.c_str(), entry->d_name) == 0) {
                found = entry->d_name;
                break;
            }
        }
        closedir(dir);
        if (found.empty()) {
            return nullptr;
        }
        resolved += "/" + found;
    }

    return fopen(resolved.c_str(), mode);
}

} // extern "C"