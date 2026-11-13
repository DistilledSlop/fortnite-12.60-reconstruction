"""Frontend-only answers that keep the menu moving (local stand-in). No backend state.

These routes come from the earlier stub. Their paths appear in the 12.60 binary or in
the client's frontend refresh pass, but their bodies were never checked against a
client capture. Every answer is an empty or default shape. A wrong 200 here can hide a
real gap, so each route is labelled guess and must be replaced once a capture exists.
"""

class FrontendStubs:
    def __init__(self, ctx):
        self.ctx = ctx

    def empty_list(self, req, match):
        return 200, []

    def socialban(self, req, match):
        return 200, {"bans": [], "warnings": []}

    def notifications_count(self, req, match):
        return 200, {"pings": 0, "invites": 0}

    def privacy(self, req, match):
        return 200, {"acceptInvites": "public"}

    def empty_object(self, req, match):
        return 200, {}

    def receipts(self, req, match):
        return 200, []

    def billing_accounts(self, req, match):
        return 200, []

    def store_access(self, req, match):
        return 204, None

    def setting_nudged(self, req, match):
        return 200, []

    def register(self, router):
        g = "guess"
        router.add("GET", r"/socialban/api/public/v1/[^/]+", self.socialban, name="socialban", status=g)
        router.add("GET", r"/party/api/v1/Fortnite/user/[^/]+/notifications/undelivered/count",
                   self.notifications_count, name="party.notifications_count", status=g)
        router.add("GET", r"/fortnite/api/game/v2/privacy/account/[^/]+", self.privacy,
                   name="privacy.account", status=g)
        router.add("GET", r"/fortnite/api/game/v2/clientfeaturekeys/[^/]+", self.empty_object,
                   name="clientfeaturekeys", status=g)
        router.add("GET", r"/fortnite/api/receipts/v1/account/[^/]+/receipts", self.receipts,
                   name="receipts", status=g)
        router.add("GET", r"/api/public/payment/accounts/[^/]+/billingaccounts", self.billing_accounts,
                   name="billing.accounts", status=g)
        router.add("POST", r"/fortnite/api/storeaccess/v1/request_access/[^/]+", self.store_access,
                   name="storeaccess", status=g)
        router.add("GET", r"/presence/api/v1/_/[^/]+/settings/subscriptions", self.empty_list,
                   name="presence.settings", status=g)
        router.add("GET", r"/presence/api/v1/Fortnite/[^/]+/subscriptions/nudged", self.setting_nudged,
                   name="presence.nudged", status=g)
