# NetServer

A C++ High Performance NetServer 

## Introduction  

本项目为C++编写的基于epoll的多线程网络服务器框架，应用层实现了简单的HTTP服务器HttpServer，其中HTTP服务器实现了HTTP的解析和Get方法请求，目前支持静态资源访问，支持HTTP长连接。

## Origin and purpose of the project
* 项目起源：我大学时间主要用于打 ACM，现在大三下学期打算春招，找一下暑假实习，自己大学期间几乎全部空余时间都用于刷算法题目，但找工作时发现很多算法工程师都偏于机器学习、人工智能方面，而自己并未正式学习该方面知识。没有了算法岗，三月谷歌面试的失利，突然让我一时间找不到自己适合的工作。Java, Python自己只会一点点，系统地去学习感觉来不及。比赛时一直使用C++，所以打算将自己方向转向偏熟悉C++、后端、网络服务器开发。

  确定方向后自己便开始了一些相关知识的学习，本以为要学不多的我突然发现了要学习量的恐怖。原以为自己会用C++，没想到自己会的只不过是最最最基础的一部分，想起了当初老师说的那句我们写的C++是“Your grandfather's C++”。差不多整个三月我都活在恐慌之中，有时不分昼夜地疯狂地补习 C++语法知识、网络知识、Linux网络编程，悔恨着大一大二大把时间的荒废，“恨不能系长绳于青天，系此西飞之白日”。几次连续熬夜通宵让我身体有些吃不消，但我也于此中吮吸着学习知识所带来的快乐，心里充溢着成长所带来的满足。终于抓住了三月的尾巴，完成了这个HTTP 服务器的项目。坦白这个项目很多的代码借鉴（使用、抄袭）了《Linux 高性能服务器编程》中的代码片段（因为自己学习Linux网络编程即是看的这本书），以及MarvinLe的 Web 服务器[A C++ Lightweight Web Server based on Linux epoll](https://github.com/MarvinLe/WebServer)

  今天毫无心理准备地收到了阿里的面试通知，一下想起一月份的时候ACM校队的学长内推了我，但那时简历比较老了，感觉现在有了很多提升的地方要反应在简历上，于是决定把该HTTP项目上传到 Github 上。

* 项目目的：C++知识、部分C++11的语法和编码规范、学习巩固网络编程、网络IO模型、多线程、Linux命令、TCP/IP、HTTP协议等知识

## Envoirment  
* OS: Ubuntu 18.04
* kernel: 4.13.0-46-generic

## Build

	$ make
	$ make clean

## Run
	$ ./httpserver [port] [iothreadnum] [workerthreadnum]
	
	例：$ ./httpserver 80 4 2
	表示开启80端口，采用4个IO线程、2个工作线程的方式 
	一般情况下，业务处理简单的话，工作线程数设为0即可

## Tech
 * 基于epoll的IO复用机制实现Reactor模式，采用边缘触发（ET）模式，和非阻塞模式
 * 由于采用ET模式，read、write和accept的时候必须采用循环的方式，直到error==EAGAIN为止，防止漏读等清况，这样的效率会比LT模式高很多，减少了触发次数
 * 使用RAII手法封装互斥器(pthrea_mutex_t)、 条件变量(pthread_cond_t)等线程同步互斥机制，使用RAII管理文件描述符等资源
 * 使用线程池提高并发度，并降低频繁创建线程的开销
 * 使用零拷贝优化性能，减少拷贝次数
 * 线程模型将划分为主线程、IO线程和worker线程，主线程接收客户端连接（accept），并通过Round-Robin策略分发给IO线程，IO线程负责连接管理（即事件监听和读写操作），worker线程负责业务计算任务（即对数据进行处理，应用层处理复杂的时候可以开启）
 * 支持HTTP长连接
 * 支持优雅关闭连接
   * 通常情况下，由客户端主动发起FIN关闭连接
   * 客户端发送FIN关闭连接后，服务器把数据发完才close，而不是直接暴力close
   * 如果连接出错，则服务器可以直接close
