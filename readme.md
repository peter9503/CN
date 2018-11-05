server:普通的server，收到訊息之後發一模一樣的訊息出去

client:會發送一個隨機整數的hex字串出去，並確認收到的訊息是否和發出去的一模一樣，不一樣的話不做回應，一樣的話則印出作業要求的文字

使用方法
``
	./server [port]
	./client [-t timeout(ms)] [-n ping time] [IP:port]
``
