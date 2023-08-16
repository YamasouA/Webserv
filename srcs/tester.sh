#!/bin/bash

mkdir YoupiBanane
cd YoupiBanane
touch youpi.bad_extension
touch youpi.bla
mkdir nop
cd nop
touch youpi.bad_extension
touch other.public
cd ..
mkdir Yeah
touch Yeah/not_happy.bad_extension
cd ..

./tester http://localhost:8000
rm -rf YoupiBanane
