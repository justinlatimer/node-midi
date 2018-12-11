#!/usr/bin/env bash
set -e -u

# test installing from source
echo "building binaries for publishing";
npm install --build-from-source;
# npm test;

node-pre-gyp package testpackage;
node-pre-gyp publish;
node-pre-gyp info;
npm run clean;