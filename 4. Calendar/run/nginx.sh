sudo rm /etc/nginx/sites-enabled/cppcms.calendar
sudo ln -s "$(pwd)/configs/nginx.conf" /etc/nginx/sites-enabled/cppcms.calendar
sudo service nginx restart
