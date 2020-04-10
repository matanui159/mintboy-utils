#!/bin/sh
set -e
IMAGE=docker.pkg.github.com/matanui159/mintboy-utils/mintboy-docker
docker build . -t $IMAGE
docker push $IMAGE
