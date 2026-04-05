#pragma once

#include <engine/core/types.h>

namespace qp {

enum class ConsoleMsgLevel : u8
{
    Info,
    Warn,
    Error,
    Cmd
};

// Initialize console (call once at startup)
void console_init();

// Push a message into the console buffer
void console_push(ConsoleMsgLevel level, const char *fmt, ...);

// Clear all messages
void console_clear();

// Draw the ImGui console window. Returns true if the console is visible.
// toggle_key_pressed: whether the toggle key was just pressed this frame
void console_draw(bool *open);

// Register a command callback: console_register("noclip", noclip_fn)
// The callback receives the full argument string after the command name.
using ConsoleCmdFn = void (*)(const char *args);
void console_register(const char *name, ConsoleCmdFn fn);

} // namespace qp
