# docker run -v "$(pwd):/var/loadtest" -v $HOME/.ssh:/root/.ssh -it direvius/yandex-tank
yandex-tank -c ../configs/tank.conf.yaml
