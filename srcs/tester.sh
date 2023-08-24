#!/bin/bash

mkdir YoupiBanane
cd YoupiBanane
touch youpi.bad_extension
touch youpi.bla
mkdir nop
cd nop
touch youpi.bad_extension
touch other.pouic
cd ..
mkdir Yeah
cd Yeah
touch not_happy.bad_extension
cd ../..

./tester http://localhost:8000
rm -rf YoupiBanane
