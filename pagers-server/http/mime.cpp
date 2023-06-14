#include <cstring>
#include <cstdio>
#include "mime.hpp"

// taken from
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
const char* MIME_JS   = "text/javascript";
const char* MIME_CSS  = "text/css";
const char* MIME_HTML = "text/html";

// returned when no match
const char* MIME_UNKNOWN = "application/octet-stream";

const char* last_dot(const char* str) {
    for (int i=strlen(str)-1; i>=0; i--) {
        if (str[i] == '.')
            return str + i;
    }

    return nullptr;
}

const char* content_type_for(const char* filename) {
    const char* dot = last_dot(filename);
    if (!dot) {
        printf("no file extension in filename: '%s'\n", filename);
        return MIME_UNKNOWN;
    }

    dot++; // skip dot

    if (strcmp(dot, "js") == 0)
        return MIME_JS;
    if (strcmp(dot, "css") == 0)
        return MIME_CSS;
    if (strcmp(dot, "html") == 0)
        return MIME_HTML;

    printf("unknown file extension: '%s'\n", dot);
    return MIME_UNKNOWN;
}
