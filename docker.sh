#!/bin/sh
set -e
IMAGE=matanui159/mintboy-docker
docker build . -t $IMAGE
docker push $IMAGE
