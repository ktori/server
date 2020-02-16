#!/bin/bash

set -e

build/server &

[[ "200" = $(curl -sL -w "%{http_code}\\n" "http://127.0.0.1:8080" -o /dev/null) ]]
[[ "200" = $(curl --insecure -sL -w "%{http_code}\\n" "https://127.0.0.1:8081" -o /dev/null) ]]

kill %1
