## Assumptions

- Whenever Redis is up Mysql will be up, so no need to maintain queue
- When redis and mysql both down , no service will be provided
- When redis is down and mysql is up only get queries will be served.

## Commands Supported

- set key value
- expire key time 
- get key



##To compile execute following command.

```c
gcc proxy.c `mysql_config --cflags --libs` libhiredis.a
```

## Files to look

- proxy.c
- proxy.h
