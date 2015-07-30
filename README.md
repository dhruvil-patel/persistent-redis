## Assumptions

- Whenever Redis is up Mysql will be up, so no need to maintain queue
- When redis and mysql both down , no service will be provided
- When redis is down and mysql is up only get queries will be served.

## Commands Supported

- set key value
- expire key time 
- get key

## Overview of approach

Main thread continously accepts commands , and serves depending on the status of Redis and MySQL.Whenever Redis or MySQL goes down corresponding status flag is set to DOWN, and new thread starts which tries to reconnect to Redis or MySQL. After successfully reconnecting this thread stops. 

##To compile execute following command.

```c
gcc proxy.c `mysql_config --cflags --libs` libhiredis.a
```

## Files to look

- proxy.c
- proxy.h
