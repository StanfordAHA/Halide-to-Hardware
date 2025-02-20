import json
def pretty_format_json(obj, indent=0, indent_step=2, inline_threshold=200):
    """
    Recursively format a JSON object with pretty printing similar to PrettyWriter.

    Parameters:
    obj: The JSON-serializable Python object (dict, list, etc.)
    indent: Current indentation level (number of spaces).
    indent_step: Number of spaces to add for each nesting level.
    inline_threshold: If a list of primitives formatted inline is shorter than this,
                        it will be output on a single line.

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
        # If all items are primitive (no dicts or lists), try to inline them.
        if all(not isinstance(item, (dict, list)) for item in obj):
            inline = json.dumps(obj)
            if len(inline) <= inline_threshold:
                return inline
        # Otherwise, format each item on its own line.
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
        # For primitive types, use the standard json.dumps formatting.
        return json.dumps(obj)