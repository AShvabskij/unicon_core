#!/bin/bash

# Get WiFi interface name (usually wlan0 or similar)
WIFI_IFACE=$(ip link | grep -oP 'wlan[0-9]+' | head -1)

if [ -z "$WIFI_IFACE" ]; then
    echo "Error: WiFi interface not found!"
    exit 1
fi

echo "Disabling WiFi ($WIFI_IFACE)..."
sudo ip link set "$WIFI_IFACE" down

# echo "Waiting 30 seconds..."
# sleep 30

# echo "Re-enabling WiFi ($WIFI_IFACE)..."
# sudo ip link set "$WIFI_IFACE" up

echo "Done!"

exit 0
