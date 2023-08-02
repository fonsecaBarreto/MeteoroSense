/******* WIFI *******/
const char *ssid = "Gabriel";
const char *password = "2014072276";

/******* MQTT Broker Connection Details *******/
const char *mqtt_server = "192.168.0.173"; // "telemetria.macae.ufrj.br"
const char *mqtt_username = "lucas";
const char *mqtt_password = "1234567";
const char *mqtt_topic = "test"; //  "/prefeituras/macae/estacoes/est001";
const int mqtt_port = 8883;

/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDaTCCAlGgAwIBAgIUcQZkqAKrBQMk4XmAUqSjiwOdOxEwDQYJKoZIhvcNAQEL
BQAwRDELMAkGA1UEBhMCQlIxDjAMBgNVBAcMBU1hY2FlMQ0wCwYDVQQKDAR1ZnJq
MRYwFAYDVQQDDA0xOTIuMTY4LjAuMTczMB4XDTIzMDczMDE3NDg0MloXDTIzMDky
ODE3NDg0MlowRDELMAkGA1UEBhMCQlIxDjAMBgNVBAcMBU1hY2FlMQ0wCwYDVQQK
DAR1ZnJqMRYwFAYDVQQDDA0xOTIuMTY4LjAuMTczMIIBIjANBgkqhkiG9w0BAQEF
AAOCAQ8AMIIBCgKCAQEAzliqbGb2uSlPmTbUYAOPBoXWHIoyogT26H7JR8KmY5NQ
3ph6IARXolnCTgkoCoWD8f8Bd6BkpQIOGyhYjS73DhcUyMmBqguo6K1bsGU2R+eg
rSS2xgaFvfFhKEAiKd4GoHwiMLIkDr44gkAY+bERaodGnkgskOe2OVKTn/05cYIc
yNM3smn2QuwkiXwYpTlNq2cH0wzQiBzKbTQUeoCR5MgOslDGlE/1EgV0mxXPPh8f
Ssr/xDXFnhJZJBFt9FMMTtolNNHyBMDrm/kb7Q9rc90FsFeKjoogweZdPRHtWYpK
G2tmlnSfJEVP+/8laP/Fz6sBh70zllbLoKseyMHE6QIDAQABo1MwUTAdBgNVHQ4E
FgQUTN3ovob1OSKatIqNeq/pFW1VAmEwHwYDVR0jBBgwFoAUTN3ovob1OSKatIqN
eq/pFW1VAmEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAmcyl
t3cWTMrW2wIFkC8EynS42sZpqyUivjNn0aizvqdhdqc+KKZnMpiScwmBLEzEGddG
cZmULcZgFBZ9t2uF37PG4rEjPzTbkZNhYS+NFdlkRa6FikRppShYiFb+jzt2XQIT
rM/4qExSnDYM6H3tfhYslZf/FY2+zZ1QvqB+z9PDTCqlm7U5JBuRSIVmIyTr+enO
EumFh6mVy7+hUW084rOM7buJDD4OJ7f0YkqLZRtb5Dd5JF35GzX+kfKPU3O0fhmY
/RVAQjHf6ENzxC+RNYLD82JFjYp7GENsmLZBumHRoUhtJCDb+bOmehh/7d19B5ry
CggHbmPo6ouMwWfA+g==
-----END CERTIFICATE-----
)EOF";