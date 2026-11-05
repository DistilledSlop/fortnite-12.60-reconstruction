"""Compatibility facade for probe_server.py --stub. The logic lives in backend.py and the service modules.

Keeps the module-level respond() and websocket_route() that probe_server.py and the earlier
smoke tests call. configure() replaces the default backend, for example to set a
cloud storage directory before the server starts.

The default backend has process-wide state. Configure state_dir to persist profiles
and user cloud files across restarts; tokens, parties and matchmaking sessions remain in memory.
"""

from api_common import LOCAL_ACCOUNT_ID as ACCOUNT_ID
from api_common import LOCAL_DISPLAY_NAME as DISPLAY_NAME
from api_common import RawText  # noqa: F401  (re-exported for callers of the earlier stub)
from backend import Backend

DEFAULT = Backend()


def configure(**options):
    """Replace the default backend, for example configure(cloudstorage_dir=...)."""
    global DEFAULT
    DEFAULT = Backend(**options)
    return DEFAULT


def respond(method, path, query, form, body=b"", headers=None, query_all=None):
    return DEFAULT.respond(method, path, query=query, form=form, body=body,
                           headers=headers, query_all=query_all)


def websocket_route(path, query):
    return DEFAULT.websocket_route(path, query)
