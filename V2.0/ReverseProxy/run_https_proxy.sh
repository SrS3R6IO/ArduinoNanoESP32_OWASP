# This Bash script starts the reverse proxy using Caddy

#!/bin/bash

ESP32_IP="192.168.1.25"
ESP32_PORT=80
DOMAIN="esp32.localhost"
CADDYFILE=Caddyfile

cat > $CADDYFILE <<EOF
https://$DOMAIN {
    reverse_proxy $ESP32_IP:$ESP32_PORT
}
EOF
