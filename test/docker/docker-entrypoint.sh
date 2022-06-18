#!/bin/bash

set -eux

make
exec $@
