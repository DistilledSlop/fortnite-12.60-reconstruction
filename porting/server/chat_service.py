"""Game chat room reservation (local stand-in). Python standard library only.

Route: /api/game/v2/chat/{accountId}/reserveGeneralChatRooms/{subgame}/{platform}

Evidence:
  binary     the route format `/api/game/v2/chat/`accountId/reserveGeneralChatRooms/`subgame/`platform`
             is in the 12.60 exe (UTF-16) and one function loads it (see EVIDENCE.md).
  reflection the answer type is `GeneralChatReturn` (FounderChatRooms, GlobalChatRooms; each entry a
             `GeneralChatRoom`). The member names are in the reflection struct_blocks.json, copied to
             reflection_schemas.json. Linking the route to this struct is by name only, so that link is inferred.
  convention the verb (POST) is not confirmed. An empty room list is the answer used, because no room
             names or member counts are known. No rooms are invented.
"""

from api_common import ApiError, Err  # noqa: F401  (kept for symmetry with the other services)


class ChatService:
    def __init__(self, ctx, oauth):
        self.ctx = ctx
        self.oauth = oauth

    def reserve_general_rooms(self, req, match):
        account_id = match.group(1)
        self.oauth.authenticate(req, account_id=account_id)
        # GeneralChatReturn members. The five bools are reflected bitfields, so the engine reads them
        # too. "Not disabled, no paid access" is the default, and no rooms are invented.
        return 200, {
            "bIsSubGameGlobalChatDisabled": False,
            "bIsFounderChatDisabled": False,
            "bIsGlobalChatDisabled": False,
            "bNeedsPaidAccessForFounderChat": False,
            "bNeedsPaidAccessForGlobalChat": False,
            "founderChatRooms": [],
            "globalChatRooms": [],
        }

    def register(self, router):
        router.add("POST", r"(?:/fortnite)?/api/game/v2/chat/([^/]+)/reserveGeneralChatRooms/([^/]+)/([^/]+)",
                   self.reserve_general_rooms, name="chat.reserve_general_rooms", status="inferred")
