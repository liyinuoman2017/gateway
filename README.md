**如何设计一个好的软件架构，如何提高软件的扩展性，移植性，复用性和可读性？**

很多做嵌入式开发的朋友经常会遇到这种情况：一个项目软件设计完成了，客户提出了一些新的功能需求。这时侯如果客户新需求不多，软件增加一些新功能即可，但是增加新功能后程序容易出现各种异常问题；这时侯如果客户新需求很多，导致软件全局很多地方需要修改，甚至有可能导致软件重写。造成这种结果的原因是，软件设计没有遵循软件设计原则，没有使用正确的设计模式和正确的软件架构。

软件设计五大原则：单一原则 ，开闭原则，里氏代换原则，接口隔离原则，依赖倒置原则。

27种设计模式：适配器模式，装饰模式，观察者模式，迭代器模式，拜访者模式等等（不一一列举）。

软件架构：分层架构，过滤器架构，插件架构，MVC架构等等。

规则和方法繁多，往往难以融会贯通地使用到实际项目中，接下来我用一个项目来介绍如何遵循软件设计原则，如何使用良好的设计模式和架构，这个项目是一个用于智能家具系统的网关软件设计项目。

我们先来了解一下智能家居系统，它是由网关，智能触控屏，控制模块，检测模块等组成，实现对家居设备智能控制。该系统可以控制灯，空调，窗帘，蓝牙音乐，地暖等家居设备。用户可以通过语音指令，智能触控屏，手机APP，电脑云端等方式监控家居设备状态。该系统应用场景如图：

![在这里插入图片描述](https://img-blog.csdnimg.cn/904f08eb3b79455fb405d9ca30bd8424.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

**网关功能描述**

智能家居系统的控制枢纽是网关，网关的主要任务是实现对触控模块，控制模块，检测模块的状态监测和控制。网关的功能如下：

1、使用嵌入式以太网控制器硬件实现TCP访问局域网内的本地服务器和外网的云端服务器。本地服务器可以配置网关参数，可以通过网口对网关固件进行升级，用户通过手机APP使用云端服务器下发数据给网关。

2、使用485总线接收面板的和环境传感器的上报数据，分析数据并下发485指令操作开关控制器实现对执行单元的控制。

3、用户可以通过扩展的输入输出IO口，实现简单的开关输入和控制。

**网关硬件描述**

网关的硬件使用华大的HC32F460单片机为核心，外围设备有485电路，RTC电路，LCD电路，以太网控制器电路，IO控制电路。网关的硬件框图如下:

![在这里插入图片描述](https://img-blog.csdnimg.cn/935d237fa9d4455ab452cb5a3ca19b27.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

**软件框架**

网关软件使用FreeRTOS操作系统，采用了模块化设计方法，每个功能独立成一个模块，每个单独的模块采用了分层设计。软件框架图如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/70d895b0f10147128804a934df66baba.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

软件使用了模块化设计的方法，模块化设计核心思想就是“分而治之”， 就是把一个复杂的问题分解为若干个简单的问题，然后逐个解决。在嵌入式软件设计中通常以硬件外设划分模块如：485模块，RTC模块，LCD模块，温湿度模块，ADC数据采集模块，GPIO控制模块等。模块化设计提高了软件系统的扩展性，模块可以根据需求布署和删除，模块化设计遵循了单一原则。软件工程源码中模块的划分如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/b02d6bcf783a4d509e9ccc500b6963eb.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

单独的功能模块采用了的分层设计，分层设计的核心思想也是“分而治之”，分层设计将软件功能水平分割成合理的多个子系统，软件中紧密关联的部分被集中放在一个层内。分层设计的框图如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/00bdf8cdb8ef4623926323f32edff820.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

分层架构有以下优点：

1、每一层都把一个具体功能抽象化。

2、可以降低代码的相互依赖程度，更改代码时影响的层很少。

3、层可以被复用。

软件工程源码中GPIO任务的BSP层代码如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/8c0503c8553d441c878d4e53abced50d.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

每个层都有一个xxxx_interface()函数，上层文件通过这个函数使用下层提供的服务，这种设计原则为接口隔离原则。3层模型的调用关系图如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/690027a1a2584995a7e6ba504ece159d.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

每个功能模块采用了3层的分层设计，第1层处理MCU寄存器相关操作，第2层处理驱动控制和逻辑控制，第2层用来处理与其他业务模块的数据交互.分层设计提高了软件系统的移植性，如果项目更换了MCU那么只用修改第1层，如果更换改了外设那么只用修改第2层，如果更改了业务逻辑那么只用修改第3层。

**任务之间通信**

网关软件中的普通任务相互隔离，所有普通任务只与消息推送任务进行数据交互，消息推送任务将消息推送给相应任务。各个任务之间的信息交互模式如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/c2e2dca5e52d476dadd82e08de4f093d.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

这种设计模式为中间者模式。在中间者模式，对象之间不能直接通信，而是间接地通过中间者进行通信。中间者收到信息后，再将信息转发给相关对象，这样减少了对象之间的相互依赖。中间者模式有以下优点：

1、对象之间是松耦合

2、将多对多的关系通过中间者转换成一对一的关系

3、修改一个对象，不需要考虑其它对象通信适应问题。

消息推送任务采用订阅与发布机制。普通任务状态改变向消息推送任务发布消息，推送任务获取发布任务的发布信息名称，并一层检查其他任务的订阅信息名称，是否包含发布信息名称，然后将信息推送给相关任务。这样减少了任务之间的耦合，提高了软件的扩展性。消息推送任务代码如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/5d0089ade46e43fa9f3de309d26c7894.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

**业务控制**

网关软件使用模块化设计，普通任务不参入软件的整体业务控制，每个普通任务只负责完成模块内部的控制逻辑，如RTC任务时负责设置时间和读取时间，GPIO只负责读取IO口和控制IO口。软件设计了一个特殊的逻辑控制任务，这个任务负责记录软件整体状态，并通过发布消息改变其他的任务的状态。这种也是使用的中间者模式。逻辑框图如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/50e1e754ad384541b2bd98b67a8a8f97.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

逻辑控制任务是通过发布特定的消息来实现对子任务的控制，子任务状态改变后也是通过发布消息通知逻辑控制任务。

**逻辑控制任务分析**

网关软件中逻辑控制任务控制整个程序运行状态，因此逻辑控制任务设计非常关键，逻辑控制任务设计软件设计框架如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/2b59d723d8b44b2fb17cc7f67e9829ef.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

子任务将信息发布给逻辑控制任务，由于不同的子任务发布的消息格式不同，数据长度不同，因此逻辑控制任务先用指令归一化接口，将不同模块的不同格式的数据转换成逻辑任务中统一的数据格式如：指令类型+指令数据。代码如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/1f2ac603000c41168958e861c11e5794.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

指令归一化处理后将指令放入队列缓存器中，然后指令过滤器读取队列缓存器中的指令进行处理，这种设计模式为命令模式，这种设计模式可以把一个命令的形成和执行在时间上去耦，命令的生成和执行可以在不用时间完成。

指令过滤器从指令队列中读取指令，并一层一层处理指令得到有效的执行指令，这种设计构架为过滤器构架，代码如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/331906bee2084552833f6e25ad65dacf.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAbGl5aW51bzIwMTc=,size_20,color_FFFFFF,t_70,g_se,x_16)

经过逻辑处理后产生一些控制指令，并将控制指令放入队列缓存，指令分发处理模块读取控制指令，根据控制指令类型发布消息给相应的任务。

**总结**

网关的软件框架基本描述完了，希望获取源码的朋友们可以在评论区里留言，我将提供学习源码（删减部分与商业相关代码）。

总结一下关键词：

**模块化设计，分层设计，单一原则，接口隔离原则，中间者模式，订阅发布，归一化，命令模式，过滤器架构**


**创作不易希望朋友们点赞，转发，关注。希望获取源码的朋友们在评论区里留言。**

**作者：李巍**

**Github：liyinuoman2017**

