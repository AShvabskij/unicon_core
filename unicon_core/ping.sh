#!/bin/bash

set +x

echo "ping to main vpn server..."
ping 10.9.0.1 -c 1

echo "ping to reservred vpn server..."
ping 10.8.0.1 -c 1

exit 0
