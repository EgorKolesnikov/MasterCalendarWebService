## Либы

cppCMS

mongoDB 3.6. Install: https://docs.mongodb.com/v3.6/tutorial/install-mongodb-enterprise-on-ubuntu/

mongocxx 3.3.0. Install: http://mongocxx.org/mongocxx-v3/installation/

## Что где лежит

### src/
1) util.h/.cpp - вспомогательные функции
2) urls.h/.cpp - реализация поведения ручек
3) server.cpp - main

### run/
1) server.sh - сборка и запуск сервера. Указывается необходимый протокол: http или fastcgi
2) configs/ - кофиги сервера на разных протоколах

### test/
1) get/ - генерация и запуск нагрузочного тестирования с ammo только GET запросов
2) post/ - генерация и запуск нагрузочного тестирования с ammo только POST запросов
3) [not done] mixed/ - генерация и запуск нагрузочного тестирования с ammo GET и POST запросов вместе
4) tank.sh - запуск нагрузочного тестирования указанного типа (get|post|mixed)

Тестирование поведения ручек реализовано в test/api.py (проверки на формат JSON, формат даты, возвращаемые коды и т.д.)
