#!/bin/ash

CLIENT_IP=$(getent hosts client | awk '{print $1}')
SERVER_IP=$(getent hosts server | awk '{print $1}')

ping -c1 -W1 "$CLIENT_IP" >/dev/null
ping -c1 -W1 "$SERVER_IP" >/dev/null

CLIENT_MAC=$(ip neigh show ${CLIENT_IP} | awk '{print $5}')
SERVER_MAC=$(ip neigh show ${SERVER_IP} | awk '{print $5}')

./inquisitor "${CLIENT_IP}" "${CLIENT_MAC}" "${SERVER_IP}" "${SERVER_MAC}" "$@"