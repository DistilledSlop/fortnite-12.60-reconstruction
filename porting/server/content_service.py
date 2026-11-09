"""Lightswitch, version check, storefront, calendar and content endpoints (local stand-in).

Evidence:
  observed  GET /lightswitch/api/service/bulk/status (earlier login capture) and the
            status field names in the binary (serviceInstanceId, status, message,
            allowedActions, banned, overrideCatalogIds, launcherInfoDTO); the version
            check answer type NO_UPDATE; GET /fortnite/api/game/v2/enabled_features
            answering []; POST /datarouter/api/v1/public/data answering 204.
  inferred  the single-service lightswitch route /lightswitch/api/service/{id}/status
            (answers one object, not a list), and the status values UP, DOWN and
            MAINTENANCE.
  reflection the calendar timeline (CalendarDownload, EventsTimeline) and the storefront catalog
            (CatalogDownload) member names and kinds; see reflection_schemas.json.
  guess     the keychain body (storefront) and the content page. These are the earlier stub's answers. The keychain
            entry is 32 zero bytes as a placeholder. It is not a real key.
"""

from catalog import storefronts

LIGHTSWITCH_STATUSES = ("UP", "DOWN", "MAINTENANCE")  # inferred values
CATALOG_OVERRIDE_ID = "a7f138b2e51945ffbfdacc1af0541053"  # from the earlier stub
LAUNCHER_INFO = {"appName": "Fortnite", "catalogItemId": "4fe75bbc5a674f4f9b356b5c90567da5", "namespace": "fn"}
KEYCHAIN_PLACEHOLDER = "00000000000000000000000000000000:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=:/Game/Local/Unused"


class ContentService:
    def __init__(self, ctx):
        self.ctx = ctx
        self._status = {}

    def set_service_status(self, service_id, status, message=""):
        """Set one service's lightswitch state (tests and maintenance drills)."""
        if status not in LIGHTSWITCH_STATUSES:
            raise ValueError(f"status must be one of {LIGHTSWITCH_STATUSES}")
        self._status[service_id] = (status, message)

    def _entry(self, service_id):
        status, message = self._status.get(service_id, ("UP", "Fortnite is online" if service_id == "fortnite" else ""))
        return {
            "serviceInstanceId": service_id,
            "status": status,
            "message": message,
            "maintenanceUri": None,
            "overrideCatalogIds": [CATALOG_OVERRIDE_ID],
            "allowedActions": ["PLAY", "DOWNLOAD"],
            "banned": False,
            "launcherInfoDTO": LAUNCHER_INFO,
        }

    # Handlers ----------------------------------------------------------

    def bulk_status(self, req, match):
        service_ids = req.query_all.get("serviceId") or ["fortnite"]
        return 200, [self._entry(service_id) for service_id in service_ids]

    def single_status(self, req, match):
        return 200, self._entry(match.group(1))

    def version_check(self, req, match):
        return 200, {"type": "NO_UPDATE"}

    def enabled_features(self, req, match):
        return 200, []

    def timeline(self, req, match):
        now = self.ctx.iso()
        state = {
            "activeStorefronts": ["BRDailyStorefront"], "eventNamedWeights": {}, "seasonNumber": 12,
            "seasonTemplateId": "AthenaSeason:athenaseason12", "matchXpBonusPoints": 0,
            "seasonBegin": "2020-06-02T00:00:00Z", "seasonEnd": "2099-01-01T00:00:00.000Z",
            "seasonDisplayedEnd": "2099-01-01T00:00:00.000Z", "weeklyStoreEnd": "2099-01-01T00:00:00.000Z",
            "stwEventStoreEnd": "2099-01-01T00:00:00.000Z", "stwWeeklyStoreEnd": "2099-01-01T00:00:00.000Z",
            "dailyStoreEnd": "2099-01-01T00:00:00.000Z",
        }
        far = "2099-01-01T00:00:00.000Z"
        return 200, {"channels": {
            "client-matchmaking": {"states": [], "cacheExpire": far},
            "client-events": {"states": [{"validFrom": "2020-01-01T00:00:00.000Z",
                                          "activeEvents": [], "state": state}], "cacheExpire": far},
        }, "cacheIntervalMins": 10.0, "currentTime": now}  # CalendarDownload members (reflection)

    def catalog(self, req, match):
        far = "2099-01-01T00:00:00.000Z"
        return 200, {"refreshIntervalHrs": 1, "dailyPurchaseHrs": 24, "expiration": far, "storefronts": storefronts()}

    def keychain(self, req, match):
        return 200, [KEYCHAIN_PLACEHOLDER]

    def content_page(self, req, match):
        now = self.ctx.iso()
        return 200, {"_title": "Fortnite Game", "_activeDate": now, "lastModified": now, "_locale": "en-US"}

    def datarouter(self, req, match):
        return 204, None

    def register(self, router):
        router.add("GET", r"/lightswitch/api/service/bulk/status", self.bulk_status,
                   name="lightswitch.bulk", status="observed")
        router.add("GET", r"/lightswitch/api/service/([^/]+)/status", self.single_status,
                   name="lightswitch.single", status="inferred")
        router.add("GET", r"/fortnite/api/v2/versioncheck/[^/]+", self.version_check,
                   name="versioncheck", status="observed")
        router.add("GET", r"/fortnite/api/game/v2/enabled_features", self.enabled_features,
                   name="enabled_features", status="observed")
        router.add("GET", r"/fortnite/api/calendar/v1/timeline", self.timeline,
                   name="calendar.timeline", status="guess")
        router.add("GET", r"/fortnite/api/storefront/v2/catalog", self.catalog,
                   name="storefront.catalog", status="guess")
        router.add("GET", r"/fortnite/api/storefront/v2/keychain", self.keychain,
                   name="storefront.keychain", status="guess")
        router.add("GET", r"/content/api/pages/fortnite-game", self.content_page,
                   name="content.page", status="guess")
        router.add("POST", r"/datarouter/api/v1/public/data", self.datarouter,
                   name="datarouter", status="observed")
