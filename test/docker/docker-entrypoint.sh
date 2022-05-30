#!/bin/bash

make
exec ./webserv "$@"
