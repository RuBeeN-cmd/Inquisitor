#!/bin/ash

CLIENT_HOST=${CLIENT_HOST:-client}
SERVER_HOST=${SERVER_HOST:-server}

resolve() {
	host="$1"
	i=0
	while [ "$i" -lt 30 ]; do
		ip=$(getent hosts "$host" | awk '{print $1}')
		[ -n "$ip" ] && { echo "$ip"; return 0; }
		sleep 1
		i=$((i + 1))
	done
	return 1
}

CLIENT_IP=$(resolve "$CLIENT_HOST") || { echo "cannot resolve $CLIENT_HOST" >&2; exit 1; }
SERVER_IP=$(resolve "$SERVER_HOST") || { echo "cannot resolve $SERVER_HOST" >&2; exit 1; }

ping -c1 -W1 "$CLIENT_IP" >/dev/null 2>&1
ping -c1 -W1 "$SERVER_IP" >/dev/null 2>&1

CLIENT_MAC=$(ip neigh show "${CLIENT_IP}" | awk '{print $5}')
SERVER_MAC=$(ip neigh show "${SERVER_IP}" | awk '{print $5}')

echo "inquisitor: ${CLIENT_HOST}(${CLIENT_IP}/${CLIENT_MAC}) <-> ${SERVER_HOST}(${SERVER_IP}/${SERVER_MAC})"

exec ./inquisitor "${CLIENT_IP}" "${CLIENT_MAC}" "${SERVER_IP}" "${SERVER_MAC}" "$@"
