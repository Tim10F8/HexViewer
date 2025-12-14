# How to build on Linux based on Debian 
  
#### Install packages:

```bash
sudo apt-get install git build-essential libcapstone-dev -y
```

#### Clone this repo recursively:

```bash
git clone --recursive https://github.com/horsicq/HexViewer.git
cd HexViewer
```

#### Run build script:

```bash
chmod a+x build_linux.sh
bash -x build_linux.sh
```