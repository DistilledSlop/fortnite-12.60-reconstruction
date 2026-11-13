"""Athena hardcore modifier updates.

The 12.60 client reflects ModifierId and bEnabled in each update, and
UFortHardcoreModifierItem has the profile attribute is_enabled. Field casing,
ownership checks and duplicate rejection for non-empty updates are local
policies; only an empty `updates` request has been observed on the wire.
"""

from api_common import ApiError, Err


def _invalid(message):
    return ApiError(400, Err.INVALID_PAYLOAD, message)


def set_hardcore_modifier(ctx):
    updates = ctx.body.get("updates")
    if not isinstance(updates, list):
        raise _invalid("updates must be an array.")
    tx = ctx.tx("athena")
    seen = set()
    for update in updates:
        if not isinstance(update, dict):
            raise _invalid("Each modifier update must be an object.")
        modifier_id = update.get("modifierId")
        enabled = update.get("bEnabled")
        if not isinstance(modifier_id, str) or not modifier_id:
            raise _invalid("modifierId must be a non-empty string.")
        if not isinstance(enabled, bool):
            raise _invalid("bEnabled must be true or false.")
        if modifier_id in seen:
            raise _invalid("A modifier may only appear once in updates.")
        seen.add(modifier_id)
        item = tx.item(modifier_id)
        if not item.get("templateId", "").casefold().startswith("hardcoremodifier:"):
            raise _invalid("modifierId must identify an owned hardcore modifier.")
        tx.set_item_attr(modifier_id, "is_enabled", enabled)


def register(command):
    command("SetHardcoreModifier", ("athena",), "observed",
            "updates: array of modifierId and bEnabled (non-empty schema inferred).")(
                set_hardcore_modifier)
