#include "ue_vector.h"

ue_vector* ue_vector_start(size_t init_capacity, size_t data_size) {
    if (init_capacity == 0)
        init_capacity = 4;

    ue_vector* new_vect = (ue_vector*)malloc(sizeof(ue_vector));
    assert(new_vect != NULL);

    new_vect->data = (void**)malloc(sizeof(void*) * init_capacity);
    assert(new_vect->data != NULL);

    new_vect->data_size = data_size;
    new_vect->capacity = init_capacity;
    new_vect->length = 0;

    return new_vect;
}

void ue_vector_finish(ue_vector* vect) {
    for (size_t i = 0; i < ue_vector_length(vect); ++i)
        free(vect->data[i]);
    free(vect->data);
    free(vect);
}

size_t ue_vector_length(const ue_vector* const vect) {
    assert(vect != NULL);
    return vect->length;
}

size_t ue_vector_capacity(const ue_vector* const vect) {
    assert(vect != NULL);
    return vect->capacity;
}

size_t ue_vector_data_size(const ue_vector* const vect) {
    assert(vect != NULL);
    return vect->data_size;
}

bool ue_vector_is_empty(const ue_vector* const vect) {
    assert(vect != NULL);
    return ue_vector_length(vect) == 0;
}

void ue_vector_resize(ue_vector* vect) {
    assert(vect != NULL);
    if (ue_vector_length(vect) == ue_vector_capacity(vect)) {
        vect->capacity += ue_vector_capacity(vect) / 2;
        vect->data = (void**)realloc(vect->data, sizeof(void*) * ue_vector_capacity(vect));
    }
}

void ue_vector_shrink_to_fit(ue_vector* vect) {
    assert(vect != NULL);
    assert(!ue_vector_is_empty(vect));
    vect->capacity = ue_vector_length(vect) + 1;
    vect->data = (void**)realloc(vect->data, sizeof(void*) * ue_vector_capacity(vect));
}

void ue_vector_add_back(ue_vector* vect, const void* const to_be_added) {
    assert(vect != NULL);
    assert(to_be_added != NULL);
    if (!ue_vector_is_empty(vect))
        ue_vector_add_in(vect, to_be_added, ue_vector_length(vect));
    else
        ue_vector_add_in(vect, to_be_added, 0);
}

void ue_vector_add_front(ue_vector* vect, const void* const to_be_added) {
    assert(vect != NULL);
    assert(to_be_added != NULL);
    ue_vector_add_in(vect, to_be_added, 0);
}

void ue_vector_add_in(ue_vector* vect, const void* const to_be_added, size_t pos) {
    assert(vect != NULL);
    assert(to_be_added != NULL);
    assert(pos <= ue_vector_length(vect));

    ue_vector_resize(vect);
    vect->data[ue_vector_length(vect)] = (void*)malloc(ue_vector_data_size(vect));

    // moving elements
    for (size_t i = ue_vector_length(vect); i > pos; --i) {
        memcpy(vect->data[i], vect->data[i - 1], ue_vector_data_size(vect));
    }

    memcpy(vect->data[pos], to_be_added, ue_vector_data_size(vect));
    ++(vect->length);
}

