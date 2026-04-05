#include <engine/core/console.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

#include "imgui.h"

namespace qp {

// ============================================================================
// Internal state
// ============================================================================

static constexpr u32 MAX_MESSAGES   = 512;
static constexpr u32 MAX_MSG_LEN    = 256;
static constexpr u32 MAX_COMMANDS   = 64;
static constexpr u32 MAX_HISTORY    = 32;
static constexpr u32 MAX_CMD_NAME   = 32;

struct ConsoleMsg
{
    char            text[MAX_MSG_LEN];
    ConsoleMsgLevel level;
};

struct ConsoleCommand
{
    char         name[MAX_CMD_NAME];
    ConsoleCmdFn fn;
};

static ConsoleMsg     g_messages[MAX_MESSAGES];
static u32            g_msg_count = 0;
static u32            g_msg_head  = 0; // ring buffer write index

static ConsoleCommand g_commands[MAX_COMMANDS];
static u32            g_cmd_count = 0;

static char           g_input_buf[MAX_MSG_LEN] = {};
static bool           g_scroll_to_bottom        = true;
static bool           g_reclaim_focus            = false;

// Command history
static char           g_history[MAX_HISTORY][MAX_MSG_LEN] = {};
static u32            g_history_count = 0;
static u32            g_history_head  = 0;
static i32            g_history_pos   = -1; // -1 = not browsing

// ============================================================================
// Built-in commands
// ============================================================================

static void cmd_clear(const char *)
{
    console_clear();
}

static void cmd_help(const char *)
{
    console_push(ConsoleMsgLevel::Info, "--- Commands ---");
    for (u32 i = 0; i < g_cmd_count; i++) {
        console_push(ConsoleMsgLevel::Info, "  %s", g_commands[i].name);
    }
}

// ============================================================================
// Public API
// ============================================================================

void console_init()
{
    g_msg_count = 0;
    g_msg_head  = 0;
    g_cmd_count = 0;
    g_history_count = 0;
    g_history_head  = 0;
    g_history_pos   = -1;

    console_register("clear", cmd_clear);
    console_register("help", cmd_help);
}

void console_push(ConsoleMsgLevel level, const char *fmt, ...)
{
    ConsoleMsg &msg = g_messages[g_msg_head];
    msg.level       = level;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.text, MAX_MSG_LEN, fmt, args);
    va_end(args);

    g_msg_head = (g_msg_head + 1) % MAX_MESSAGES;
    if (g_msg_count < MAX_MESSAGES) {
        g_msg_count++;
    }

    g_scroll_to_bottom = true;
}

void console_clear()
{
    g_msg_count = 0;
    g_msg_head  = 0;
}

void console_register(const char *name, ConsoleCmdFn fn)
{
    if (g_cmd_count >= MAX_COMMANDS) {
        return;
    }
    ConsoleCommand &cmd = g_commands[g_cmd_count++];
    snprintf(cmd.name, MAX_CMD_NAME, "%s", name);
    cmd.fn = fn;
}

// ============================================================================
// Internal helpers
// ============================================================================

static void history_add(const char *line)
{
    // Don't add duplicates of the last entry
    if (g_history_count > 0) {
        u32 last = (g_history_head + MAX_HISTORY - 1) % MAX_HISTORY;
        if (strcmp(g_history[last], line) == 0) {
            return;
        }
    }

    snprintf(g_history[g_history_head], MAX_MSG_LEN, "%s", line);
    g_history_head = (g_history_head + 1) % MAX_HISTORY;
    if (g_history_count < MAX_HISTORY) {
        g_history_count++;
    }
}

static void execute_input(const char *input)
{
    // Skip whitespace
    while (*input == ' ') {
        input++;
    }
    if (*input == '\0') {
        return;
    }

    history_add(input);
    console_push(ConsoleMsgLevel::Cmd, "> %s", input);

    // Extract command name
    char cmd_name[MAX_CMD_NAME] = {};
    const char *p = input;
    u32 i = 0;
    while (*p && *p != ' ' && i < MAX_CMD_NAME - 1) {
        cmd_name[i++] = *p++;
    }
    cmd_name[i] = '\0';

    // Skip space between command and args
    while (*p == ' ') {
        p++;
    }

    // Find and execute
    for (u32 c = 0; c < g_cmd_count; c++) {
        if (strcmp(g_commands[c].name, cmd_name) == 0) {
            g_commands[c].fn(p);
            return;
        }
    }

    console_push(ConsoleMsgLevel::Error, "Unknown command: '%s'. Type 'help' for list.", cmd_name);
}

static int input_callback(ImGuiInputTextCallbackData *data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
        if (g_history_count == 0) {
            return 0;
        }

        if (data->EventKey == ImGuiKey_UpArrow) {
            if (g_history_pos == -1) {
                g_history_pos = (i32)g_history_count - 1;
            } else if (g_history_pos > 0) {
                g_history_pos--;
            }
        } else if (data->EventKey == ImGuiKey_DownArrow) {
            if (g_history_pos != -1) {
                g_history_pos++;
                if (g_history_pos >= (i32)g_history_count) {
                    g_history_pos = -1;
                }
            }
        }

        if (g_history_pos >= 0) {
            // Map history_pos to ring buffer index
            u32 start = (g_history_count >= MAX_HISTORY) ? g_history_head : 0;
            u32 idx   = (start + (u32)g_history_pos) % MAX_HISTORY;
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, g_history[idx]);
        } else {
            data->DeleteChars(0, data->BufTextLen);
        }
    }
    return 0;
}

static ImVec4 level_color(ConsoleMsgLevel level)
{
    switch (level) {
        case ConsoleMsgLevel::Info:  return {0.80f, 0.80f, 0.80f, 1.0f}; // light gray
        case ConsoleMsgLevel::Warn:  return {1.0f, 0.8f, 0.2f, 1.0f};  // yellow
        case ConsoleMsgLevel::Error: return {1.0f, 0.3f, 0.3f, 1.0f};  // red
        case ConsoleMsgLevel::Cmd:   return {0.7f, 0.7f, 0.9f, 1.0f};  // light blue (user input)
    }
    return {1.0f, 1.0f, 1.0f, 1.0f};
}

// ============================================================================
// Draw
// ============================================================================

void console_draw(bool *open)
{
    if (!*open) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(620, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Console", open)) {
        ImGui::End();
        return;
    }

    // Messages area
    f32 footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollRegion", ImVec2(0, -footer_height), ImGuiChildFlags_None,
                       ImGuiWindowFlags_HorizontalScrollbar);

    // Render messages from ring buffer
    u32 start = 0;
    if (g_msg_count >= MAX_MESSAGES) {
        start = g_msg_head; // oldest message
    }
    for (u32 i = 0; i < g_msg_count; i++) {
        u32 idx = (start + i) % MAX_MESSAGES;
        ImGui::PushStyleColor(ImGuiCol_Text, level_color(g_messages[idx].level));
        ImGui::TextUnformatted(g_messages[idx].text);
        ImGui::PopStyleColor();
    }

    if (g_scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
        g_scroll_to_bottom = false;
    }

    ImGui::EndChild();

    // Input line
    ImGui::Separator();
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                ImGuiInputTextFlags_CallbackHistory;

    if (g_reclaim_focus) {
        ImGui::SetKeyboardFocusHere();
        g_reclaim_focus = false;
    }

    if (ImGui::InputText("##input", g_input_buf, MAX_MSG_LEN, flags, input_callback)) {
        execute_input(g_input_buf);
        g_input_buf[0]  = '\0';
        g_history_pos    = -1;
        g_reclaim_focus  = true;
    }

    // Auto-focus on appearing
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

} // namespace qp
