#!/usr/bin/env bash

key="00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF"
INBOX="/tmp/inbox"
OUTBOX="/tmp/outbox"

mkdir -p $INBOX
mkdir -p $OUTBOX
rm -rf $INBOX/*
rm -rf $OUTBOX/*

./udp-file enc $key img1.jpeg < test-data/IMG_20240101_160447_HDR.jpg > $INBOX/test.bin &&
./udp-file enc $key img2.mp4 < test-data/IMG_1095-1.MP4 >> $INBOX/test.bin &&

ls -lh $INBOX &&

./udp-file shuffle $INBOX/test.bin &&

./udp-file dump < $INBOX/test.bin &&

./udp-file dec $key $OUTBOX < $INBOX/test.bin &&

ls -lh $OUTBOX &&

diff test-data/IMG_20240101_160447_HDR.jpg $OUTBOX/img1.jpeg &&
diff test-data/IMG_1095-1.MP4 $OUTBOX/img2.mp4 &&

echo PASSED &&

rm -rf $INBOX $OUTBOX
