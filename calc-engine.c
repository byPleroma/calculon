#include "calc-engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <glib.h>
#include <errno.h>
#include <float.h>

/* --- Helper: Inicialização DRY de Result --- */
static inline void init_result_state(CalcOperationResult *out_result) {
    out_result->type = CALC_SUCCESS;
    out_result->result_value = 0.0;
    out_result->has_history = false;
    out_result->display_ptr = NULL;
    out_result->history_ptr = NULL;
}

/* --- Macro: Commit result pointer to state buffer (eliminates strncpy boilerplate) --- */
#define COMMIT_RESULT(engine_state, result_struct) do { \
    (result_struct)->display_ptr = (engine_state)->input_buffer; \
    (result_struct)->history_ptr = (engine_state)->history_buffer; \
    (result_struct)->type = CALC_SUCCESS; \
} while(0)

/* --- Helper: Format double to buffer (O(1) direct formatting) --- */
static inline void format_double_to_buffer(double val, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%.10g", val);
}

/* --- Helper: Símbolo Visual para UI (O(1) lookup table) --- */
static inline const char* get_op_symbol(CalcOperator op) {
    static const char* op_symbols[] = {
        [OP_NONE] = "", [OP_ADD] = "+", [OP_SUB] = "−",
        [OP_MUL] = "×", [OP_DIV] = "÷", [OP_POW] = "^",
        [OP_RECIPROCAL] = "1/x", [OP_SQUARE] = "x²",
        [OP_SQRT] = "√", [OP_PERCENT] = "%"
    };
    return (op < sizeof(op_symbols)/sizeof(op_symbols[0])) ? op_symbols[op] : "";
}

/* --- Helper: g_ascii_strtod for locale-independent parsing --- */
static double safe_strtod(const char *str) {
    if (str == NULL || str[0] == '\0') return 0.0;
    if (str[0] == '-' && str[1] == '\0') return 0.0;
    char *endptr;
    double result = g_ascii_strtod(str, &endptr);
    return (str == endptr) ? 0.0 : result;
}

void calc_engine_init(CalcEngineState *state) {
    state->input_buffer[0] = '0';
    state->input_buffer[1] = '\0';
    state->buffer_len = 1;
    state->history_len = 0;
    state->current_operator = OP_NONE;
    state->memory_value = 0.0;
    state->stored_value = 0.0;
    state->current_value = 0.0;
    state->flags = 0;
    state->history_buffer[0] = '\0';
}

void calc_engine_reset(CalcEngineState *state) {
    state->stored_value = 0.0;
    state->current_value = 0.0;
    state->current_operator = OP_NONE;
    state->flags = 0;
    state->memory_value = 0.0;
    state->input_buffer[0] = '0';
    state->input_buffer[1] = '\0';
    state->buffer_len = 1;
    state->history_len = 0;
    state->history_buffer[0] = '\0';
}

void calc_engine_append_digit(CalcEngineState *state, char digit, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (state->flags & FLAG_HAS_RESULT) calc_engine_reset(state);

    bool is_zero_state = (state->buffer_len == 1 && state->input_buffer[0] == '0');

    if (digit == '.') {
        if (state->flags & FLAG_HAS_DECIMAL) {
            COMMIT_RESULT(state, out_result);
            return;
        }
        if (!(state->flags & FLAG_IS_INPUTTING) || is_zero_state) {
            state->input_buffer[0] = '0';
            state->input_buffer[1] = '.';
            state->input_buffer[2] = '\0';
            state->buffer_len = 2;
            state->current_value = 0.0;
        } else {
            state->input_buffer[state->buffer_len++] = '.';
            state->input_buffer[state->buffer_len] = '\0';
        }
        state->flags |= FLAG_HAS_DECIMAL;
        state->flags |= FLAG_IS_INPUTTING;
        state->flags &= ~FLAG_HAS_RESULT;
        COMMIT_RESULT(state, out_result);
        return;
    }

    if (!(state->flags & FLAG_IS_INPUTTING) || is_zero_state) {
        int num_digit = digit - '0';
        state->input_buffer[0] = digit;
        state->input_buffer[1] = '\0';
        state->buffer_len = 1;
        state->flags &= ~FLAG_HAS_DECIMAL;
        state->current_value = (double)num_digit;
    } else {
        if (state->buffer_len >= MAX_INPUT_DIGITS) {
            COMMIT_RESULT(state, out_result);
            return;
        }
        state->input_buffer[state->buffer_len++] = digit;
        state->input_buffer[state->buffer_len] = '\0';

        state->current_value = safe_strtod(state->input_buffer);
    }

    state->flags |= FLAG_IS_INPUTTING;
    state->flags &= ~FLAG_HAS_RESULT;
    COMMIT_RESULT(state, out_result);
}

void calc_engine_set_operator(CalcEngineState *state, CalcOperator op, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (!(state->flags & FLAG_IS_INPUTTING) && op == OP_SUB) {
        state->input_buffer[0] = '-';
        state->input_buffer[1] = '\0';
        state->buffer_len = 1;
        state->flags |= FLAG_IS_INPUTTING;
        state->flags &= ~FLAG_HAS_RESULT;
        COMMIT_RESULT(state, out_result);
        return;
    }

    if (state->flags & FLAG_IS_INPUTTING || state->flags & FLAG_HAS_RESULT) {
        if ((state->flags & FLAG_IS_INPUTTING) && state->current_operator != OP_NONE) {
            calc_engine_calculate(state, out_result);
            if (out_result->type != CALC_SUCCESS) return;
        }
        state->stored_value = state->current_value;
        state->flags &= ~FLAG_IS_INPUTTING;
        state->flags &= ~FLAG_HAS_RESULT;
        state->flags &= ~FLAG_HAS_DECIMAL;
    }

    state->current_operator = op;

    format_double_to_buffer(state->stored_value, state->history_buffer, sizeof(state->history_buffer));
    state->history_len = strlen(state->history_buffer);
    int space_left = sizeof(state->history_buffer) - state->history_len;
    int written = snprintf(state->history_buffer + state->history_len,
                          space_left,
                          " %s", get_op_symbol(op));
    if (written > 0) {
        state->history_len += (written < space_left) ? written : (space_left - 1);
    }
    out_result->history_ptr = state->history_buffer;
    out_result->has_history = true;

    COMMIT_RESULT(state, out_result);
}

void calc_engine_calculate(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    double current = state->current_value;
    double calc_result = 0.0;
    const char *sym = "";

    errno = 0;
    switch (state->current_operator) {
        case OP_MUL: calc_result = state->stored_value * current; sym = "×"; break;
        case OP_SUB: calc_result = state->stored_value - current; sym = "−"; break;
        case OP_DIV:
            if (fabs(current) < DBL_EPSILON) {
                out_result->type = CALC_ERROR_DIVISION_BY_ZERO;
                memcpy(state->input_buffer, ERR_MSG, ERR_LEN + 1);
                state->current_operator = OP_NONE;
                state->flags |= FLAG_HAS_RESULT;
                state->flags |= FLAG_IS_FROZEN;
                state->buffer_len = ERR_LEN;
                COMMIT_RESULT(state, out_result);
                return;
            }
            calc_result = state->stored_value / current;
            sym = "÷"; break;
        case OP_ADD: calc_result = state->stored_value + current; sym = "+"; break;
        case OP_POW: 
            calc_result = pow(state->stored_value, current); 
            sym = "^"; 
            break;
        default:
            assert(0 && "Unreachable");
            out_result->type = CALC_ERROR_INVALID_OPERATOR;
            state->current_operator = OP_NONE;
            state->input_buffer[0] = '0';
            state->input_buffer[1] = '\0';
            state->buffer_len = 1;
            state->flags &= ~FLAG_HAS_DECIMAL;
            COMMIT_RESULT(state, out_result);
            return;
    }

    if (errno == ERANGE || isnan(calc_result) || isinf(calc_result)) {
        out_result->type = CALC_ERROR_MATH_INVALID;
        memcpy(state->input_buffer, ERR_MSG, ERR_LEN + 1);
        state->flags |= FLAG_IS_FROZEN;
        state->buffer_len = ERR_LEN;
        COMMIT_RESULT(state, out_result);
        return;
    }

    state->current_value = calc_result;
    format_double_to_buffer(calc_result, state->input_buffer, sizeof(state->input_buffer));
    state->buffer_len = strlen(state->input_buffer);
    
    int written = snprintf(state->history_buffer, sizeof(state->history_buffer),
                           "%.10g %s %.10g =", state->stored_value, sym, current);
    state->history_len = (written > 0 && written < (int)sizeof(state->history_buffer))
                         ? (size_t)written
                         : sizeof(state->history_buffer) - 1;
    
    out_result->history_ptr = state->history_buffer;
    out_result->has_history = true;
    out_result->result_value = calc_result;
    state->flags |= FLAG_HAS_RESULT;
    COMMIT_RESULT(state, out_result);
}

void calc_engine_equal(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (state->current_operator != OP_NONE) {
        calc_engine_calculate(state, out_result);
        state->current_operator = OP_NONE;
    }
}

void calc_engine_backspace(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if ((state->flags & FLAG_IS_INPUTTING) && !(state->flags & FLAG_HAS_RESULT)) {
        if (state->buffer_len > 1) {
            if (state->input_buffer[state->buffer_len - 1] == '.') {
                state->flags &= ~FLAG_HAS_DECIMAL;
            }
            state->buffer_len--;
            state->input_buffer[state->buffer_len] = '\0';
        } else {
            state->input_buffer[0] = '0';
            state->input_buffer[1] = '\0';
            state->buffer_len = 1;
            state->flags &= ~FLAG_IS_INPUTTING;
        }
    }
    COMMIT_RESULT(state, out_result);
}

void calc_engine_unary_op(CalcEngineState *state, CalcOperator op, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    double current = state->current_value;
    double calc_result = current;

    errno = 0;
    switch (op) {
        case OP_RECIPROCAL:
            if (fabs(current) < DBL_EPSILON) {
                out_result->type = CALC_ERROR_DIVISION_BY_ZERO;
                memcpy(state->input_buffer, ERR_MSG, ERR_LEN + 1);
                state->flags |= FLAG_HAS_RESULT;
                state->flags |= FLAG_IS_FROZEN;
                state->buffer_len = ERR_LEN;
                COMMIT_RESULT(state, out_result);
                return;
            }
            calc_result = 1.0 / current;
            break;
        case OP_SQUARE:
            calc_result = current * current;
            break;
        case OP_SQRT:
            if (current < 0) {
                out_result->type = CALC_ERROR_INVALID_OPERATOR;
                memcpy(state->input_buffer, ERR_MSG, ERR_LEN + 1);
                state->flags |= FLAG_HAS_RESULT;
                state->flags |= FLAG_IS_FROZEN;
                state->buffer_len = ERR_LEN;
                COMMIT_RESULT(state, out_result);
                return;
            }
            calc_result = sqrt(current);
            break;
        case OP_PERCENT:
            if (state->current_operator == OP_ADD || state->current_operator == OP_SUB) {
                calc_result = state->stored_value * (current / 100.0);
            } else {
                calc_result = current / 100.0;
            }
            break;
        default:
            assert(0 && "Unreachable");
            return;
    }

    if (errno == ERANGE || isnan(calc_result) || isinf(calc_result)) {
        out_result->type = CALC_ERROR_MATH_INVALID;
        memcpy(state->input_buffer, ERR_MSG, ERR_LEN + 1);
        state->flags |= FLAG_IS_FROZEN;
        state->buffer_len = ERR_LEN;
        COMMIT_RESULT(state, out_result);
        return;
    }

    state->current_value = calc_result;
    format_double_to_buffer(calc_result, state->input_buffer, sizeof(state->input_buffer));
    state->buffer_len = strlen(state->input_buffer);

    out_result->has_history = false;
    out_result->result_value = calc_result;
    state->flags |= FLAG_HAS_RESULT;
    state->flags &= ~FLAG_IS_INPUTTING;
    COMMIT_RESULT(state, out_result);
}

void calc_engine_flip_sign(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    state->current_value = -state->current_value;
    format_double_to_buffer(state->current_value, state->input_buffer, sizeof(state->input_buffer));
    state->buffer_len = strlen(state->input_buffer);
    COMMIT_RESULT(state, out_result);
}

void calc_engine_memory_clear(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    state->memory_value = 0.0;
    state->flags &= ~FLAG_MEMORY_IN_USE;
    COMMIT_RESULT(state, out_result);
}

void calc_engine_memory_recall(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (state->flags & FLAG_MEMORY_IN_USE) {
        format_double_to_buffer(state->memory_value, state->input_buffer, sizeof(state->input_buffer));
        state->buffer_len = strlen(state->input_buffer);
        state->flags |= FLAG_IS_INPUTTING;
        state->flags &= ~FLAG_HAS_RESULT;
    }
    COMMIT_RESULT(state, out_result);
}

void calc_engine_memory_add(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (state->flags & FLAG_MEMORY_IN_USE) {
        state->memory_value += state->current_value;
    } else {
        state->memory_value = state->current_value;
        state->flags |= FLAG_MEMORY_IN_USE;
    }
    COMMIT_RESULT(state, out_result);
}

void calc_engine_memory_subtract(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    if (state->flags & FLAG_MEMORY_IN_USE) {
        state->memory_value -= state->current_value;
    } else {
        state->memory_value = -state->current_value;
        state->flags |= FLAG_MEMORY_IN_USE;
    }
    COMMIT_RESULT(state, out_result);
}

void calc_engine_memory_store(CalcEngineState *state, CalcOperationResult *out_result) {
    init_result_state(out_result);

    if (state->flags & FLAG_IS_FROZEN) return;

    state->memory_value = state->current_value;
    state->flags |= FLAG_MEMORY_IN_USE;
    COMMIT_RESULT(state, out_result);
}
