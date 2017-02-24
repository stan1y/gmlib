#!/bin/bash
source buildenv && make static && cd src/demo && make demo && make linklibs