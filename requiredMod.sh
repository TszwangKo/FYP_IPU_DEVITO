sudo apt-get update
sudo apt-get install software-properties-common protobuf-compiler  -y
sudo apt-get install build-essential g++ python3-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev -y
sudo apt-get install python3.8-venv screen -y
pip install panel

echo -ne "
For the Wave Equation, 2 additional packeges are needed. Follow the link below to complete installation

Devito Compiler:        https://www.devitoproject.org/devito/download.html
nlohmann JSON library:  https://github.com/nlohmann/json\n\n"