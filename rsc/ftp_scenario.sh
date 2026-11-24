#!/bin/ash

SERVER_HOST=${SERVER_HOST:-server}
FTP_USER=${FTP_USER:-user}
FTP_PASS=${FTP_PASS:-pass}
ITERATIONS=${ITERATIONS:-0}

CURL="curl -s --ftp-port - --user ${FTP_USER}:${FTP_PASS}"

echo "[tester] waiting for ${SERVER_HOST}:21 ..."
until $CURL "ftp://${SERVER_HOST}/" >/dev/null 2>&1; do
	sleep 1
done
echo "[tester] server is up; letting inquisitor settle..."
sleep 5

work=/tmp/ftp_scenario
mkdir -p "$work"
head -c 4096 /dev/urandom | base64 > "$work/upload.txt"

i=0
while [ "$ITERATIONS" -eq 0 ] || [ "$i" -lt "$ITERATIONS" ]; do
	i=$((i + 1))
	echo "[tester] --- round $i ---"

	echo "[tester] STOR upload.txt"
	$CURL -T "$work/upload.txt" "ftp://${SERVER_HOST}/upload.txt"

	echo "[tester] LIST"
	$CURL "ftp://${SERVER_HOST}/" >/dev/null

	echo "[tester] RETR upload.txt"
	$CURL "ftp://${SERVER_HOST}/upload.txt" -o "$work/download.txt"

	sleep 4
done

echo "[tester] done."
