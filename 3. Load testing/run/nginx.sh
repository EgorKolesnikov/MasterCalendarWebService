sudo rm /etc/nginx/sites-enabled/cppcms.hello-world.loadtest
sudo ln -s "$(pwd)/../configs/nginx.conf" /etc/nginx/sites-enabled/cppcms.hello-world.loadtest
sudo service nginx restart
