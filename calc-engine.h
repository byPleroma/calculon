#ifndef CALC_ENGINE_H
#define CALC_ENGINE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_DISPLAY_LEN 32
#define MAX_HISTORY_LEN 256
#define MAX_INPUT_DIGITS 15

#define ERR_MSG "Erro"
#define ERR_LEN (sizeof(ERR_MSG) - 1)

/* --- Resultado de Operação --- */
typedef enum {
    CALC_SUCCESS,
    CALC_ERROR_DIVISION_BY_ZERO,
    CALC_ERROR_INVALID_OPERATOR,
    CALC_ERROR_MATH_INVALID
} CalcResultType;

typedef struct {
    double result_value;
    CalcResultType type;
    bool has_history;
    const char *display_ptr;
    const char *history_ptr;
} CalcOperationResult;

/* --- Enum para Operadores (Desacoplamento UI/Core) --- */
typedef enum {
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_RECIPROCAL,
    OP_SQUARE,
    OP_SQRT,
    OP_PERCENT
} CalcOperator;

typedef enum {
    MEM_CLEAR,
    MEM_RECALL,
    MEM_ADD,
    MEM_SUBTRACT,
    MEM_STORE
} CalcMemOp;

/* --- Estado do Motor de Cálculo (Sem GTK) --- */
typedef struct {
    double stored_value;
    double memory_value;
    double current_value;
    size_t buffer_len;
    size_t history_len;
    CalcOperator current_operator;
    uint8_t flags;
    char input_buffer[MAX_DISPLAY_LEN];
    char history_buffer[MAX_HISTORY_LEN];
} CalcEngineState;

/* Flag bit definitions for CalcEngineState.flags */
#define FLAG_IS_INPUTTING   (1 << 0)
#define FLAG_HAS_RESULT     (1 << 1)
#define FLAG_HAS_DECIMAL    (1 << 2)
#define FLAG_MEMORY_IN_USE  (1 << 3)
#define FLAG_IS_FROZEN      (1 << 4)

/* --- API do Motor de Cálculo --- */
void calc_engine_init(CalcEngineState *state);
void calc_engine_reset(CalcEngineState *state);
void calc_engine_append_digit(CalcEngineState *state, char digit, CalcOperationResult *out_result);
void calc_engine_set_operator(CalcEngineState *state, CalcOperator op, CalcOperationResult *out_result);
void calc_engine_calculate(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_equal(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_unary_op(CalcEngineState *state, CalcOperator op, CalcOperationResult *out_result);
void calc_engine_backspace(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_flip_sign(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_memory_clear(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_memory_recall(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_memory_add(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_memory_subtract(CalcEngineState *state, CalcOperationResult *out_result);
void calc_engine_memory_store(CalcEngineState *state, CalcOperationResult *out_result);

#endif /* CALC_ENGINE_H */
