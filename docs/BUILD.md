How to build on Linux based on Debian
=======

Install packages:

- sudo apt-get install --quiet --assume-yes git build-essential cmake python3 libcapstone-dev

git clone --recursive https://github.com/horsicq/HexViewer.git

cd HexViewer

Run build script: bash -x build_linux.sh
