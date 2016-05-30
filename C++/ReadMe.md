# How to register service 
1 setup console windows
2 locate in the exe path
3 enter: appname /registerService

# How to unregistered service 
```
appname /registerService
```

# setup service
```
net start appname

```

# shutdown service
```
net stop appname
```

# how to test 
this is UDP echo server which bind  ip: 12.0.0.1 port 8080
use the udp tool to send data to this address , message would echo 