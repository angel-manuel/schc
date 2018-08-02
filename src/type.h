#ifndef SCHC_CORE_TYPE_H_
#define SCHC_CORE_TYPE_H_

typedef enum core_type_ {
    TYPE_NONE = 0,
    TYPE_INT,
    TYPE_STR,
} core_type_t;

const char *type_to_str(core_type_t type);

#endif /*SCHC_CORE_TYPE_H_*/
