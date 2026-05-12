# yatop

Монитор ресурсов Linux с бэкендом на C++ и WebUI-фронтендом.

## Архитектура

Программа работает в 2 потока: один регулярно читает `/proc`, другой обслуживает HTTP сервер с WebUI на localhost. Выбранный способ IPC с процессом бразуера - Server-Sent Events.

Поток информации:
- /proc -> ResourceSampler (I/O операции)
- ResourceSampler -> HttpServer (общее адресное пространство)
- HttpServer -> frontend (HTTP, SSE)

## Требования

- Linux
- компилятор C++20
- CMake
- npm

## Сборка

Сборка фронтенда:

```bash
npm install && npm run build
```

Сборка бэкенда:

```bash
cmake -S . -B build && cmake --build build
```

## Запуск

```bash
./build/yatop
```

Открыть в браузере:

```text
http://127.0.0.1:8080
```

## Опции

```text
--host 127.0.0.1
--port 8080
--frontend-root frontend/dist
--interval-ms 1000
```

## Текущие метрики

- общая загрузка CPU
- использование RAM
- таблица процессов с PID, именем, состоянием, использованием CPU и RAM

Бэкенд возвращает все процессы, которые может прочитать из `/proc`. Сортировка выполняется в браузере.
