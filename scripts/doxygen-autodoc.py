#!/usr/bin/env python3
"""
Doxygen input filter: auto-generates brief descriptions for undocumented
C/C++ declarations based on their signatures.

Usage in Doxyfile:
    INPUT_FILTER = "python3 scripts/doxygen-autodoc.py"

Reads a source file from argv[1], writes annotated source to stdout.
Only injects doc comments where none exist (won't overwrite manual docs).
"""

import re
import sys


def has_doc_comment(lines: list[str], idx: int) -> bool:
    """Check if the line at idx already has a Doxygen comment above it."""
    i = idx - 1
    while i >= 0 and lines[i].strip() == "":
        i -= 1
    if i < 0:
        return False
    stripped = lines[i].strip()
    return stripped.endswith("*/") or stripped.startswith("///") or stripped.startswith("//!")


def snake_to_words(name: str) -> str:
    """Convert snake_case name to readable words: 'mesh_create_cube' -> 'create cube'."""
    parts = name.split("_")
    # Drop first part as subsystem prefix (mesh_, aabb_, shader_, etc.)
    if len(parts) > 1:
        parts = parts[1:]
    return " ".join(parts)


def get_subject(name: str) -> str:
    """Extract the subsystem/subject prefix: 'aabb_slide' -> 'AABB', 'mesh_draw' -> 'mesh'."""
    prefix = name.split("_")[0]
    # Known acronyms that should stay uppercase
    acronyms = {"aabb", "fbo", "vao", "vbo", "ebo", "fps", "hud", "fov"}
    if prefix.lower() in acronyms:
        return prefix.upper()
    return prefix


def camel_to_words(name: str) -> str:
    """Convert PascalCase to words: 'PlayerHealth' -> 'player health'."""
    words = re.sub(r"([A-Z])", r" \1", name).strip().lower()
    return words


def article(word: str) -> str:
    """Return 'a' or 'an' depending on word."""
    return "an" if word[0] in "AEIOUaeiou" else "a"


def guess_function_brief(return_type: str, name: str, params: str) -> str:
    """Generate a brief description from function signature."""
    words = snake_to_words(name)
    subject = get_subject(name)

    # Detect common patterns
    if name.startswith("is_") or name.startswith("has_"):
        rest = name[3:] if name.startswith("is_") else name[4:]
        return f"Check if {rest.replace('_', ' ')}."

    # *_create* -> Create a ...
    if "_create" in name:
        suffix = name.split("_create")[1].lstrip("_").replace("_", " ")
        if suffix:
            return f"Create {article(suffix)} {suffix} {subject}."
        return f"Create {article(subject)} new {subject}."

    # *_destroy* -> Destroy/free a ...
    if "_destroy" in name:
        return f"Destroy {article(subject)} {subject} and free its resources."

    # *_init* -> Initialize ...
    if "_init" in name:
        return f"Initialize the {subject} subsystem."

    # *_load* -> Load a ...
    if "_load" in name:
        return f"Load {article(subject)} {subject} from file."

    # *_bind* / *_unbind*
    if "_bind" in name and "_unbind" not in name:
        return f"Bind the {subject} for use."
    if "_unbind" in name:
        return f"Unbind the {subject}."

    # *_set_* -> Set ...
    if "_set_" in name:
        prop = name.split("_set_")[1].replace("_", " ")
        return f"Set the {prop} on the {subject}."

    # *_get_* -> Get ...
    if "_get_" in name:
        prop = name.split("_get_")[1].replace("_", " ")
        return f"Get the {prop} from the {subject}."

    # *_update* -> Update ...
    if "_update" in name:
        return f"Update the {subject} state."

    # *_draw* -> Draw ...
    if "_draw" in name:
        return f"Draw the {subject}."

    # *_process_* -> Process ...
    if "_process_" in name:
        action = name.split("_process_")[1].replace("_", " ")
        return f"Process {action} for the {subject}."

    # *_from_* -> Construct from ...
    if "_from_" in name:
        source = name.split("_from_")[1].replace("_", " ")
        return f"Create {article(subject)} {subject} from {source}."

    # *_begin_* / *_end_*
    if "_begin_" in name:
        action = name.split("_begin_")[1].replace("_", " ")
        return f"Begin {action} on the {subject}."
    if "_end_" in name:
        action = name.split("_end_")[1].replace("_", " ")
        return f"End {action} on the {subject}."

    # *_present* -> Present ...
    if "_present" in name:
        return f"Present the {subject} output to screen."

    # *_should_* -> Query ...
    if "_should_" in name:
        action = name.split("_should_")[1].replace("_", " ")
        return f"Check if the {subject} should {action}."

    # *_swap_* -> Swap ...
    if "_swap_" in name:
        what = name.split("_swap_")[1].replace("_", " ")
        return f"Swap {what} on the {subject}."

    # bool return -> Check/query
    if return_type.strip() == "bool":
        return f"Check if {subject} {words}."

    # void return with no params -> generic action
    if return_type.strip() == "void" and params.strip() == "":
        return f"Perform {words} on the {subject}."

    # Non-void return with noun-like words (no verb) -> getter
    # Heuristic: if the action words don't start with a common verb, treat as getter
    nouns_hint = {"forward", "right", "up", "down", "left", "position", "rotation",
                  "size", "width", "height", "count", "length", "matrix", "view",
                  "projection", "direction", "normal", "delta", "elapsed", "fps",
                  "color", "name", "type", "index", "data", "value", "state"}
    first_word = words.split()[0] if words.strip() else ""
    if first_word in nouns_hint or (return_type.strip() != "void" and "_" not in words.strip()):
        return f"Get the {words} of the {subject}."

    # Fallback: verb + subject
    words = words.strip()
    if words:
        return f"{words[0].upper()}{words[1:]} the {subject}."
    return ""


def guess_struct_brief(name: str) -> str:
    """Generate a brief description from struct/class name."""
    # Known acronyms - keep as-is
    acronyms = {"AABB", "FBO", "VAO", "VBO", "EBO", "HUD"}
    if name in acronyms:
        return f"Represents an {name}." if name[0] in "AEIOU" else f"Represents a {name}."

    words = camel_to_words(name)
    if "config" in words:
        return f"Configuration for {words.replace('config', '').strip()}."
    if "info" in words:
        return f"Information about {words.replace('info', '').strip()}."
    vowels = "aeiou"
    article = "an" if words.lstrip()[0] in vowels else "a"
    return f"Represents {article} {words}."


def guess_enum_brief(name: str) -> str:
    """Generate a brief for an enum."""
    words = camel_to_words(name)
    return f"Enumeration of {words} values."


def guess_typedef_brief(name: str) -> str:
    """Generate a brief for a typedef."""
    words = camel_to_words(name) if name[0].isupper() else snake_to_words(name)
    return f"Type alias for {words}."


def guess_define_brief(name: str) -> str:
    """Generate a brief for a #define macro."""
    words = name.lower().replace("_", " ")
    # Strip common prefixes
    for prefix in ("qp ",):
        if words.startswith(prefix):
            words = words[len(prefix):]
    return f"Macro: {words}."


# Regex patterns for declarations
RE_FUNC = re.compile(
    r"^(\w[\w\s\*&:<>]*?)\s+"  # return type
    r"(\w+)"                    # function name
    r"\s*\(([^)]*)\)"           # params
    r"\s*[;{]",                 # end
)
RE_STRUCT = re.compile(r"^(?:typedef\s+)?struct\s+(\w+)")
RE_ENUM = re.compile(r"^(?:typedef\s+)?enum(?:\s+class)?\s+(\w+)")
RE_TYPEDEF = re.compile(r"^typedef\s+.+\s+(\w+)\s*;")
RE_DEFINE = re.compile(r"^#define\s+(\w+)(?:\(.*?\))?\s+")
RE_NAMESPACE = re.compile(r"^namespace\s+\w+")
RE_USING = re.compile(r"^using\s+")

# Lines that look like declarations but aren't worth documenting
SKIP_NAMES = {"if", "else", "while", "for", "switch", "return", "case", "do", "namespace", "using"}


def process(source: str) -> str:
    lines = source.splitlines(keepends=True)
    output: list[str] = []
    i = 0

    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Skip empty, preprocessor guards, includes, already-documented
        if (
            not stripped
            or stripped.startswith("#include")
            or stripped.startswith("#pragma")
            or stripped.startswith("#ifndef")
            or stripped.startswith("#define") and stripped.endswith("_H")
            or stripped.startswith("#endif")
            or stripped.startswith("//")
            or stripped.startswith("/*")
        ):
            output.append(line)
            i += 1
            continue

        # Get indentation
        indent = line[: len(line) - len(line.lstrip())]

        # Only document top-level or namespace-level (indent <= 1 tab/4 spaces)
        indent_level = len(indent.replace("\t", "    "))
        if indent_level > 4:
            output.append(line)
            i += 1
            continue

        brief = ""

        # Check for #define macros
        m = RE_DEFINE.match(stripped)
        if m and not has_doc_comment(output, len(output)):
            name = m.group(1)
            if not name.startswith("_") and name not in SKIP_NAMES:
                brief = guess_define_brief(name)

        # Check for struct
        if not brief:
            m = RE_STRUCT.match(stripped)
            if m and not has_doc_comment(output, len(output)):
                brief = guess_struct_brief(m.group(1))

        # Check for enum
        if not brief:
            m = RE_ENUM.match(stripped)
            if m and not has_doc_comment(output, len(output)):
                brief = guess_enum_brief(m.group(1))

        # Check for typedef (but not struct/enum typedefs already caught)
        if not brief:
            m = RE_TYPEDEF.match(stripped)
            if m and not has_doc_comment(output, len(output)):
                brief = guess_typedef_brief(m.group(1))

        # Check for function declaration/definition
        if not brief and not RE_NAMESPACE.match(stripped) and not RE_USING.match(stripped):
            # Join with next lines if params span multiple lines
            joined = stripped
            j = i + 1
            while j < len(lines) and ")" not in joined and j < i + 5:
                joined += " " + lines[j].strip()
                j += 1

            m = RE_FUNC.match(joined)
            if m and not has_doc_comment(output, len(output)):
                ret_type = m.group(1).strip()
                name = m.group(2)
                params = m.group(3)
                if name not in SKIP_NAMES and not name.startswith("_"):
                    brief = guess_function_brief(ret_type, name, params)

        if brief:
            output.append(f"{indent}/** @brief {brief} */\n")

        output.append(line)
        i += 1

    return "".join(output)


def main():
    if len(sys.argv) < 2:
        print("Usage: doxygen-autodoc.py <source-file>", file=sys.stderr)
        sys.exit(1)

    filepath = sys.argv[1]
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        source = f.read()

    result = process(source)
    sys.stdout.write(result)


if __name__ == "__main__":
    main()
