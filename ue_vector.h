#ifndef UE_VECTOR_H
#define UE_VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct ue_vector {
    size_t length;
    size_t capacity;
    size_t data_size;
    void** data;
} ue_vector;

ue_vector*  ue_vector_start         (size_t init_capacity, size_t data_size);
void        ue_vector_finish        (ue_vector* vect);

size_t      ue_vector_length        (const ue_vector* const vect);
size_t      ue_vector_capacity      (const ue_vector* const vect);
size_t      ue_vector_data_size     (const ue_vector* const vect);
bool        ue_vector_is_empty      (const ue_vector* const vect);

void        ue_vector_resize        (ue_vector* vect);
void        ue_vector_shrink_to_fit (ue_vector* vect);

void        ue_vector_add_back      (ue_vector* vect, const void* const to_be_added);
void        ue_vector_add_front     (ue_vector* vect, const void* const to_be_added);
void        ue_vector_add_in        (ue_vector* vect, const void* const to_be_added, size_t pos);

void*       ue_vector_get_back      (const ue_vector* const vect);
void*       ue_vector_get_front     (const ue_vector* const vect);
void*       ue_vector_get_in        (const ue_vector* const vect, size_t pos);

void        ue_vector_delete_back   (ue_vector* vect);
void        ue_vector_delete_front  (ue_vector* vect);
void        ue_vector_delete_in     (ue_vector* vect, size_t pos);

#endif // UE_VECTOR_H
