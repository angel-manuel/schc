#include "type.h"

const char *type_to_str(core_type_t type) {
    switch (type) {
    case TYPE_NONE:
        return "TYPE_NONE";
    case TYPE_INT:
        return "TYPE_INT";
    case TYPE_STR:
        return "TYPE_STR";
    }

    return "TYPE_UNKNOWN";
}