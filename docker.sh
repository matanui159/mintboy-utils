#!/bin/sh
set -e
TAG=docker.pkg.github.com/matanui159/mintboy-utils/mintboy-docker:$1
docker build . -t $TAG
docker push $TAG
