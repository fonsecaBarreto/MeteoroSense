#pragma once
/******* Configuração de ambiente *******/

struct Config {
  char station_uid[64];
  char station_name[64];
  char wifi_ssid[64];
  char wifi_password[64];
  char mqtt_server[64]; 
  char mqtt_username[64]; 
  char mqtt_password[64]; 
  char mqtt_topic[64]; 
  int mqtt_port;
  int interval;
} ;

struct Config config;

const char *configFileName = "/config.txt";

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