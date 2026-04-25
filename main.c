#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "calc-engine.h"

#define GRID_ROWS 6
#define GRID_COLS 4
#define BTN_MAIN_COUNT (GRID_ROWS * GRID_COLS)
#define BTN_MEM_COUNT 6
#define BTN_TOTAL_COUNT (BTN_MAIN_COUNT + BTN_MEM_COUNT)

/* --- Metadados de Botões (Desacoplamento UI/Core) --- */
typedef enum { BTN_DIGIT, BTN_OP, BTN_EQUAL, BTN_CLEAR, BTN_SIGN, BTN_UNARY, BTN_BACKSPACE, BTN_MEMORY } BtnType;

typedef struct {
    const char *label;
    BtnType type;
    CalcOperator op;
    CalcMemOp mem_op;
    const char *css_class;
} BtnDef;

/* --- Forward Declarations --- */
typedef struct CalcState CalcState;

/* --- Contexto para Dados de Botão --- */
typedef struct ButtonContext {
    CalcState *state;
    BtnDef def;
} ButtonContext;

/* --- Estrutura de Estado da UI (Apenas widgets) --- */
struct CalcState {
    GtkWidget *window;
    GtkWidget *display_main;
    GtkWidget *display_history;
    CalcEngineState engine_state;
    GtkWidget *buttons[BTN_MAIN_COUNT];
    ButtonContext button_contexts[BTN_TOTAL_COUNT];
};

/* --- Declarações de Callbacks --- */
static void on_destroy(GtkWidget *widget, gpointer data);
static void on_digit_clicked(GtkButton *button, gpointer data);
static void on_operator_clicked(GtkButton *button, gpointer data);
static void on_equal_clicked(GtkButton *button, gpointer data);
static void on_clear_clicked(GtkButton *button, gpointer data);
static void on_unary_clicked(GtkButton *button, gpointer data);
static void on_backspace_clicked(GtkButton *button, gpointer data);
static void on_memory_clicked(GtkButton *button, gpointer data);
static void on_sign_clicked(GtkButton *button, gpointer data);
static void update_display(CalcState *state, const char *text, const char *history_text);
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);

/* --- CSS Moderno (Estilo Windows 11 Fluent Design) --- */
static const char *fluent_css =
    "window { background-color: #F3F3F3; }" /* Fundo cinza claro */
    "#calc-grid { padding: 4px; }"
    "button {"
    "  background-color: #F9F9F9;"
    "  border: 1px solid #EAEAEA;"
    "  border-radius: 6px;"
    "  box-shadow: 0 1px 2px rgba(0,0,0,0.03);"
    "  color: #1A1A1A;"
    "  font-size: 16px;"
    "  margin: 2px;"
    "  min-height: 45px;"
    "}"
    "button:hover { background-color: #F0F0F0; }"
    "button:active { background-color: #E5E5E5; box-shadow: none; border-color: #D1D1D1; }"
    ""
    "#number-btn {"
    "  background-color: #FFFFFF;"
    "  font-weight: 600;"
    "  box-shadow: 0 1px 3px rgba(0,0,0,0.05);"
    "}"
    "#number-btn:hover { background-color: #FDFDFD; }"
    ""
    "#btn-equal {"
    "  background-color: #0c6aa7;"
    "  color: white;"
    "  border: none;"
    "}"
    "#btn-equal:hover { background-color: #0a5a8f; }"
    ""
    "#memory-btn {"
    "  background: transparent;"
    "  border: none;"
    "  box-shadow: none;"
    "  font-size: 12px;"
    "  font-weight: bold;"
    "  color: #555555;"
    "  min-height: 30px;"
    "}"
    "#memory-btn:hover { background-color: #E5E5E5; border-radius: 4px; }"
    ""
    "#display-main {"
    "  font-size: 48px;"
    "  font-weight: bold;"
    "  color: #000000;"
    "  padding-right: 16px;"
    "  padding-bottom: 8px;"
    "}"
    "#display-history {"
    "  font-size: 14px;"
    "  color: #666666;"
    "  padding-right: 16px;"
    "  padding-top: 16px;"
    "}"
    "#header-text { font-size: 18px; font-weight: bold; color: #1A1A1A; }";

/* --- Principal --- */
int main(int argc, char *argv[]) {
    CalcState state = {0};
    calc_engine_init(&state.engine_state);


    gtk_init(&argc, &argv);

    /* Gestão de Recurso: Carregar e injetar o CSS, depois liberar a memória do provedor */
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, fluent_css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider); /* Liberação imediata */

    /* Janela */
    state.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(state.window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(state.window), 320, 520);
    g_signal_connect(state.window, "destroy", G_CALLBACK(on_destroy), &state);
    g_signal_connect(state.window, "key-press-event", G_CALLBACK(on_key_press), &state);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(state.window), vbox);

    /* --- Cabeçalho (Menu, Título, Histórico) --- */
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_top(header_box, 10);
    gtk_widget_set_margin_start(header_box, 15);
    gtk_widget_set_margin_end(header_box, 15);
    
    GtkWidget *icon_menu = gtk_label_new("☰");
    GtkWidget *lbl_standard = gtk_label_new("Standard");
    gtk_widget_set_name(lbl_standard, "header-text");
    gtk_widget_set_hexpand(lbl_standard, TRUE);
    gtk_widget_set_halign(lbl_standard, GTK_ALIGN_START);
    GtkWidget *icon_hist = gtk_label_new("↺");

    gtk_box_pack_start(GTK_BOX(header_box), icon_menu, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_box), lbl_standard, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header_box), icon_hist, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), header_box, FALSE, FALSE, 0);

    /* --- Visores --- */
    state.display_history = gtk_label_new("");
    gtk_widget_set_name(state.display_history, "display-history");
    gtk_label_set_xalign(GTK_LABEL(state.display_history), 1.0);
    gtk_box_pack_start(GTK_BOX(vbox), state.display_history, FALSE, FALSE, 0);

    state.display_main = gtk_label_new("0");
    gtk_widget_set_name(state.display_main, "display-main");
    gtk_label_set_xalign(GTK_LABEL(state.display_main), 1.0);
    gtk_box_pack_start(GTK_BOX(vbox), state.display_main, FALSE, FALSE, 0);

    /* Atualizando estado inicial zerado */
    update_display(&state, "0", "");

    /* --- Fila de Memória --- */
    GtkWidget *mem_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(mem_box, 10);
    gtk_widget_set_margin_end(mem_box, 10);
    static const BtnDef mem_buttons[] = {
        {"MC", BTN_MEMORY, 0, MEM_CLEAR, "memory-btn"},
        {"MR", BTN_MEMORY, 0, MEM_RECALL, "memory-btn"},
        {"M+", BTN_MEMORY, 0, MEM_ADD, "memory-btn"},
        {"M-", BTN_MEMORY, 0, MEM_SUBTRACT, "memory-btn"},
        {"MS", BTN_MEMORY, 0, MEM_STORE, "memory-btn"},
        {"M⌄", BTN_MEMORY, 0, MEM_STORE, "memory-btn"}
    };
    for (int i = 0; i < BTN_MEM_COUNT; i++) {
        const BtnDef *def = &mem_buttons[i];
        GtkWidget *btn = gtk_button_new_with_label(def->label);
        gtk_widget_set_name(btn, def->css_class);
        gtk_widget_set_can_focus(btn, FALSE);
        state.button_contexts[i + BTN_MAIN_COUNT].state = &state;
        state.button_contexts[i + BTN_MAIN_COUNT].def = *def;
        g_signal_connect(btn, "clicked", G_CALLBACK(on_memory_clicked), &state.button_contexts[i + BTN_MAIN_COUNT]);
        gtk_box_pack_start(GTK_BOX(mem_box), btn, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), mem_box, FALSE, FALSE, 5);

    /* --- Teclado Principal (Grid) --- */
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_name(grid, "calc-grid");
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    static const BtnDef btn_matrix[6][4] = {
        {{"%", BTN_UNARY, OP_PERCENT, 0, "btn-op"}, {"CE", BTN_CLEAR, 0, 0, "btn-op"}, {"C", BTN_CLEAR, 0, 0, "btn-op"}, {"⌫", BTN_BACKSPACE, 0, 0, "btn-op"}},
        {{"¹/x", BTN_UNARY, OP_RECIPROCAL, 0, "btn-op"}, {"x²", BTN_OP, OP_POW, 0, "btn-op"}, {"²√x", BTN_UNARY, OP_SQRT, 0, "btn-op"}, {"÷", BTN_OP, OP_DIV, 0, "btn-op"}},
        {{"7", BTN_DIGIT, 0, 0, ""}, {"8", BTN_DIGIT, 0, 0, ""}, {"9", BTN_DIGIT, 0, 0, ""}, {"×", BTN_OP, OP_MUL, 0, "btn-op"}},
        {{"4", BTN_DIGIT, 0, 0, ""}, {"5", BTN_DIGIT, 0, 0, ""}, {"6", BTN_DIGIT, 0, 0, ""}, {"−", BTN_OP, OP_SUB, 0, "btn-op"}},
        {{"1", BTN_DIGIT, 0, 0, ""}, {"2", BTN_DIGIT, 0, 0, ""}, {"3", BTN_DIGIT, 0, 0, ""}, {"+", BTN_OP, OP_ADD, 0, "btn-op"}},
        {{"+/−", BTN_SIGN, 0, 0, ""}, {"0", BTN_DIGIT, 0, 0, ""}, {".", BTN_DIGIT, 0, 0, ""}, {"=", BTN_EQUAL, 0, 0, "btn-equal"}}
    };

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            const BtnDef *def = &btn_matrix[row][col];
            GtkWidget *btn = gtk_button_new_with_label(def->label);
            int idx = row * GRID_COLS + col;
            if (idx >= BTN_MAIN_COUNT) continue;
            gtk_widget_set_can_focus(btn, FALSE);
            
            state.button_contexts[idx].state = &state;
            state.button_contexts[idx].def = *def;

            if (def->css_class && def->css_class[0] != '\0') {
                gtk_widget_set_name(btn, def->css_class);
            }

            switch (def->type) {
                case BTN_EQUAL:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_equal_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_CLEAR:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_clear_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_OP:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_operator_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_DIGIT:
                    gtk_widget_set_name(btn, "number-btn");
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_digit_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_UNARY:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_unary_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_BACKSPACE:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_backspace_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_SIGN:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_sign_clicked), &state.button_contexts[idx]);
                    break;
                case BTN_MEMORY:
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_memory_clicked), &state.button_contexts[idx]);
                    break;
            }

            state.buttons[idx] = btn;
            gtk_grid_attach(GTK_GRID(grid), btn, col, row, 1, 1);
        }
    }

    gtk_widget_show_all(state.window);
    gtk_main();

    return 0;
}

/* --- Lógica Básica Otimizada --- */

static void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

static void on_digit_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    const char *digit = gtk_button_get_label(button);
    CalcOperationResult result;
    calc_engine_append_digit(&state->engine_state, (char)digit[0], &result);
    update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
}

static void on_operator_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcOperator op = ctx->def.op;
    CalcOperationResult result;
    calc_engine_set_operator(&state->engine_state, op, &result);
    update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
}

static void on_equal_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcOperationResult result;
    calc_engine_equal(&state->engine_state, &result);
    update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
}

static void on_clear_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    calc_engine_reset(&state->engine_state);
    update_display(state, state->engine_state.input_buffer, "");
}

static void update_display(CalcState *state, const char *text, const char *history_text) {
    if (text) gtk_label_set_text(GTK_LABEL(state->display_main), text);
    if (history_text) gtk_label_set_text(GTK_LABEL(state->display_history), history_text);
}

static void on_unary_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcOperator op = ctx->def.op;
    if (op == 0) return;
    CalcOperationResult result;
    calc_engine_unary_op(&state->engine_state, op, &result);
    update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
}

static void on_backspace_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcOperationResult result;
    calc_engine_backspace(&state->engine_state, &result);
    update_display(state, result.display_ptr, NULL);
}

static void on_memory_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcMemOp mem_op = ctx->def.mem_op;
    CalcOperationResult result;

    switch (mem_op) {
        case MEM_CLEAR: calc_engine_memory_clear(&state->engine_state, &result); break;
        case MEM_RECALL: calc_engine_memory_recall(&state->engine_state, &result); break;
        case MEM_ADD: calc_engine_memory_add(&state->engine_state, &result); break;
        case MEM_SUBTRACT: calc_engine_memory_subtract(&state->engine_state, &result); break;
        case MEM_STORE: calc_engine_memory_store(&state->engine_state, &result); break;
        default: return;
    }

    update_display(state, result.display_ptr, NULL);
}

static void on_sign_clicked(GtkButton *button, gpointer data) {
    ButtonContext *ctx = (ButtonContext *)data;
    CalcState *state = ctx->state;
    CalcOperationResult result;
    calc_engine_flip_sign(&state->engine_state, &result);
    update_display(state, result.display_ptr, NULL);
}

/* --- Integração de Teclado --- */
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    CalcState *state = (CalcState *)data;
    CalcOperationResult result = {0};

    /* Shift + 6 ou Shift + ~ para potência, incluindo dead keys */
    if (event->keyval == GDK_KEY_caret ||
        event->keyval == GDK_KEY_dead_circumflex ||
        event->keyval == GDK_KEY_dead_tilde ||
        ((event->state & GDK_SHIFT_MASK) && event->keyval == GDK_KEY_6)) {
        calc_engine_set_operator(&state->engine_state, OP_POW, &result);
        update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
        return TRUE;
    }

    gboolean handled = TRUE;
    switch (event->keyval) {
        /* Dígitos (Teclado principal e numérico) - Range O(1) */
        case GDK_KEY_0 ... GDK_KEY_9:
            calc_engine_append_digit(&state->engine_state, (char)('0' + (event->keyval - GDK_KEY_0)), &result);
            break;
        case GDK_KEY_KP_0 ... GDK_KEY_KP_9:
            calc_engine_append_digit(&state->engine_state, (char)('0' + (event->keyval - GDK_KEY_KP_0)), &result);
            break;

        /* Ponto decimal (Ponto ou vírgula) */
        case GDK_KEY_period: case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal: case GDK_KEY_KP_Separator:
            calc_engine_append_digit(&state->engine_state, (char)'.' , &result);
            break;

        /* Operadores Básicos */
        case GDK_KEY_plus: case GDK_KEY_KP_Add:
            calc_engine_set_operator(&state->engine_state, OP_ADD, &result);
            break;
        case GDK_KEY_minus: case GDK_KEY_KP_Subtract:
            calc_engine_set_operator(&state->engine_state, OP_SUB, &result);
            break;
        case GDK_KEY_asterisk: case GDK_KEY_KP_Multiply:
            calc_engine_set_operator(&state->engine_state, OP_MUL, &result);
            break;
        case GDK_KEY_slash: case GDK_KEY_KP_Divide:
            calc_engine_set_operator(&state->engine_state, OP_DIV, &result);
            break;

        /* Igual (Enter, KP_Enter ou =) */
        case GDK_KEY_Return: case GDK_KEY_KP_Enter: case GDK_KEY_equal:
            calc_engine_equal(&state->engine_state, &result);
            break;

        /* Controles: Backspace e Clear */
        case GDK_KEY_BackSpace:
            calc_engine_backspace(&state->engine_state, &result);
            break;
        case GDK_KEY_Escape:
            calc_engine_reset(&state->engine_state);
            update_display(state, state->engine_state.input_buffer, "");
            return TRUE;

        /* Operadores Unários Específicos */
        case GDK_KEY_percent:
            calc_engine_unary_op(&state->engine_state, OP_PERCENT, &result);
            break;
        case GDK_KEY_r: case GDK_KEY_R:
            calc_engine_unary_op(&state->engine_state, OP_RECIPROCAL, &result);
            break;
        case GDK_KEY_s: case GDK_KEY_S:
            calc_engine_unary_op(&state->engine_state, OP_SQRT, &result);
            break;
        case GDK_KEY_q: case GDK_KEY_Q:
            calc_engine_unary_op(&state->engine_state, OP_SQUARE, &result);
            break;
        case GDK_KEY_F9:
            calc_engine_flip_sign(&state->engine_state, &result);
            break;

        default:
            handled = FALSE;
            break;
    }

    if (handled) update_display(state, result.display_ptr, result.has_history ? result.history_ptr : NULL);
    return handled;
}