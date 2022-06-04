#!/bin/bash

set -e

make
exec ./webserv "$@"
