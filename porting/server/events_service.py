"""Competition download: the local server has no configured tournaments."""


class EventsService:
    def __init__(self, oauth):
        self.oauth = oauth

    def download(self, req, match):
        account_id = match.group(1)
        self.oauth.authenticate(req, account_id=account_id)
        return 200, {
            "player": {"gameId": "Fortnite", "accountId": account_id, "tokens": [],
                       "persistentScores": {}, "pendingPayouts": [], "teams": {}},
            "events": [], "templates": [], "scores": [],
        }

    def register(self, router):
        router.add("GET", r"/api/v1/events/Fortnite/download/([^/]+)", self.download,
                   name="events.download", status="observed")
