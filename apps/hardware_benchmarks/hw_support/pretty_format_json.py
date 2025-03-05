import json

def pretty_format_json(obj, indent=0, indent_step=2, inline_threshold=180):
    """
    Recursively format a JSON object with pretty printing similar to PrettyWriter.

    Parameters:
      obj: The JSON-serializable Python object (dict, list, etc.)
      indent: Current indentation level (number of spaces).
      indent_step: Number of spaces to add for each nesting level.
      inline_threshold: If a list of primitives formatted inline is shorter than this,
                        it will be output on a single line. Otherwise, we chunk lines.

    Returns:
      A formatted JSON string.
    """
    spacing = " " * indent

    if isinstance(obj, dict):
        if not obj:
            return "{}"
        result = "{\n"
        inner_items = []
        for key, value in obj.items():
            formatted_value = pretty_format_json(
                value, indent + indent_step, indent_step, inline_threshold
            )
            inner_items.append(
                " " * (indent + indent_step) + json.dumps(key) + ": " + formatted_value
            )
        result += ",\n".join(inner_items)
        result += "\n" + spacing + "}"
        return result

    elif isinstance(obj, list):
        if not obj:
            return "[]"

        # If all items are primitive, we handle them differently than nested objects
        all_primitives = all(not isinstance(item, (dict, list)) for item in obj)

        if all_primitives:
            # Try to inline the entire list
            inline = json.dumps(obj)
            if len(inline) <= inline_threshold:
                # Fits in one line nicely
                return inline
            else:
                # Otherwise, chunk the items across multiple lines
                return _format_long_primitive_list(obj, indent, indent_step, inline_threshold)
        else:
            # Otherwise, fallback to the old multi-line approach for nested lists/dicts
            result = "[\n"
            inner_items = []
            for item in obj:
                formatted_item = pretty_format_json(
                    item, indent + indent_step, indent_step, inline_threshold
                )
                inner_items.append(" " * (indent + indent_step) + formatted_item)
            result += ",\n".join(inner_items)
            result += "\n" + spacing + "]"
            return result

    else:
        # For primitive types, use standard JSON string formatting
        return json.dumps(obj)


def _format_long_primitive_list(lst, indent, indent_step, inline_threshold):
    """
    Helper to chunk large lists of primitives so that each line doesn't exceed inline_threshold.
    """
    spacing = " " * indent
    indent_str = " " * (indent + indent_step)

    # Convert each item to its JSON string
    item_strs = [json.dumps(item) for item in lst]

    # We'll accumulate items into lines so that each line does not exceed inline_threshold
    lines = []
    current_line = item_strs[0]
    for s in item_strs[1:]:
        # +2 for ", " if we put them on the same line
        if len(current_line) + len(s) + 2 <= inline_threshold:
            current_line += ", " + s
        else:
            lines.append(current_line)
            current_line = s
    lines.append(current_line)  # Add the last line

    # If we only have one line, just wrap in [ ... ]
    if len(lines) == 1:
        return "[" + lines[0] + "]"

    # Otherwise, build a multi-line string, each line at the next-level indentation
    result = "[\n"
    for i, line in enumerate(lines):
        # Add a comma at the end of each line except the last
        suffix = "," if i < len(lines) - 1 else ""
        result += f"{indent_str}{line}{suffix}\n"
    result += spacing + "]"
    return result
